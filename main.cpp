#include "Arduino.h"
#include "SPI.h"
#include "TeensyPins.h"
#include "LCD.h"
#include "Encoder.h"
#include "Menu.h"





// Setup the LCD and encoder.
SPISettings SPI_settings_dac_vco_tune(30000000, MSBFIRST, SPI_MODE2);
SPISettings SPI_settings_rf_atten(10000000, MSBFIRST, SPI_MODE2);
LCD lcd(LCD_RST, LCD_RS, LCD_CS);

void write_dac_vco_tune(int value) {
    SPI.beginTransaction(SPI_settings_dac_vco_tune);
    digitalWrite(DAC_SYNC, LOW);
    SPI.transfer(0b00010100);
    SPI.transfer(lowByte(value >> 8));
    SPI.transfer(lowByte(value));
    SPI.endTransaction();
    digitalWrite(DAC_SYNC, HIGH);
}

void write_rf_atten(uint8_t data) {
    digitalWrite(ATTEN_LE, LOW);
    digitalWrite(ATTEN_CLK, LOW);
    digitalWrite(ATTEN_MOSI, LOW);

    int current_bit;
    for (int i = 5; i >= 0; i--) {
        current_bit = (data >> i) & 0x01;
        digitalWrite(ATTEN_MOSI, current_bit);
        delayMicroseconds(1);

        digitalWrite(ATTEN_CLK, HIGH);
        delayMicroseconds(1);
        digitalWrite(ATTEN_CLK, LOW);
        delayMicroseconds(1);
    }

    digitalWrite(ATTEN_LE, HIGH);
    delayMicroseconds(1);
    digitalWrite(ATTEN_LE, LOW);
}

// Define the global menu structure:

String main_menu_labels[] = {"VCO frequency", "RF Attenuation", "Int/Ext RF TTL", "VCO on/off   "};
Menu main_menu(4, main_menu_labels);



void main_menu_press_event(Encoder *this_encoder) {
    lcd.clear();
    main_menu.switch_to_mode((main_menu.current_knob())->position()%4);
}

void main_menu_hold_event(Encoder *this_encoder) {
    lcd.write("80 MHz TeensyAOM", 0x00);
    lcd.write("N. Anderson, JQI", 0x40);
    delay(7000);
}

Encoder enc_main_menu(ENC_A, ENC_B, ENC_SW, main_menu_press_event, main_menu_hold_event);

void main_menu_select_mode(Menu* this_menu) {
    lcd.write(this_menu->mode_label(enc_main_menu.position() % 4) + "      ", 0x40);
    lcd.write("MAIN MENU:         ",00);
    enc_main_menu.button_state();
}


float nominal_vco_freq(int position) {
    float MHz_per_bit = 50.0/(62885.0-42943.0);
    return (100.0 - MHz_per_bit*(position - 42943.0));
}

void VCO_freq_press_event(Encoder *this_encoder) {
    this_encoder->change_step_size();
}

void go_to_main_menu(Encoder *this_encoder) {
    delay(20);
    main_menu.switch_to_select();
    lcd.clear();
    lcd.write("Main Menu...", 0x00);
    delay(1000);
}

void RF_atten_press_event(Encoder *this_encoder) {
    lcd.write("Click!", 0x00);
}

void int_ext_rf_press_event(Encoder *this_encoder) {
    if (this_encoder->position() % 2 == 0) {
        for(int i = 0; i < 5; i++) {
            lcd.write("INT RF TTL CTRL   ",0x40);
            delay(100);
            lcd.clear();
            delay(50);
        }
        digitalWrite(INT_EXT_OUTPUT_CTL, LOW);
        delay(250);
    }
    else {
        for(int i = 0; i < 5; i++) {
            lcd.write("EXT RF TTL CTRL   ",0x40);
            delay(100);
            lcd.clear();
            delay(50);
        }
        digitalWrite(INT_EXT_OUTPUT_CTL, HIGH);
        delay(200);
    }
    main_menu.switch_to_select();
}

void VCO_on_off_press_event(Encoder *this_encoder) {
    if (this_encoder->position() % 2 == 0) {
        for(int i = 0; i < 5; i++) {
            lcd.write("   --VCO ON--   ",0x40);
            delay(100);
            lcd.clear();
            delay(50);
        }
        digitalWrite(VCO_EN, HIGH);
        delay(250);
    }
    else {
        for(int i = 0; i < 5; i++) {
            lcd.write("   --VCO OFF--   ",0x40);
            delay(100);
            lcd.clear();
        }
        delay(50);
        digitalWrite(VCO_EN, LOW);
        delay(250);
    }
    main_menu.switch_to_select();
}



Encoder enc_VCO_freq(ENC_A, ENC_B, ENC_SW, VCO_freq_press_event, go_to_main_menu);
String step_labels_VCO_freq[] = {"LSB   ", "10 KHz", "1 MHz"};
int step_sizes_VCO_freq[] = {1,4,399};

Encoder enc_RF_atten(ENC_A, ENC_B, ENC_SW, RF_atten_press_event, go_to_main_menu);

Encoder enc_int_ext_rf_select(ENC_A, ENC_B, ENC_SW, int_ext_rf_press_event, go_to_main_menu);

Encoder enc_VCO_on_off(ENC_A, ENC_B, ENC_SW, VCO_on_off_press_event, go_to_main_menu);

void VCO_tune_mode() {
    lcd.write("VCO TUNE: " + enc_VCO_freq.step_size_label() + "     ",0x00);
    enc_VCO_freq.button_state();
    write_dac_vco_tune(enc_VCO_freq.position());
    lcd.write("freq " + String(nominal_vco_freq(enc_VCO_freq.position()),3) + " MHz  ",0x40);
}

float percent_atten(int val) {
    return (31.5*(float)val)/63;
}

void RF_atten_mode() {
    lcd.write("RF ATTEN: LSB   ",0x00);
    lcd.write(String("atten:   " + String(percent_atten(enc_RF_atten.position()),1)+" dB "), 0x40);
    enc_RF_atten.button_state();
    write_rf_atten(enc_RF_atten.position());
}

void int_ext_rf_select_mode() {
    lcd.write("SELECT:          ", 0x00);
    if (enc_int_ext_rf_select.position() % 2 == 0) {
        lcd.write("INT RF TTL CTRL    ",0x40);
        digitalWrite(INT_EXT_OUTPUT_CTL, LOW);
    }
    else {
        lcd.write("EXT RF TTL CTRL    ",0x40);
        digitalWrite(INT_EXT_OUTPUT_CTL, HIGH);
    }
    enc_int_ext_rf_select.button_state();
}

void VCO_on_off_mode () {
    lcd.write("SELECT:          ", 0x00);
    if (enc_VCO_on_off.position() % 2 == 0) {
        lcd.write("    -VCO ON-      ",0x40);
    }
    else {
        lcd.write("    -VCO OFF-      ",0x40);
    }
    enc_VCO_on_off.button_state();
}

void (*modes[4])() = {VCO_tune_mode, RF_atten_mode, int_ext_rf_select_mode, VCO_on_off_mode};

void encoder_interrupt_wrapper() {
    if (main_menu.menu_select()) {
        enc_main_menu.interrupt();
    }
    else {
        if (main_menu.current_mode() == 0) {
            enc_VCO_freq.interrupt();
        }
        else if (main_menu.current_mode() == 1) {
            enc_RF_atten.interrupt();
        }
        else if (main_menu.current_mode() == 2) {
            enc_int_ext_rf_select.interrupt();
        }
        else if (main_menu.current_mode() == 3) {
            enc_VCO_on_off.interrupt();
        }
    }
}


void setup() {
    // LOW gives control to TEENSY_OUTPUT_TTL. HIGH gives control to EXT_OUTPUT_TTL. Together these determine OUTPUT_TTL which controls the final switch before RF_OUT.
    pinMode(INT_EXT_OUTPUT_CTL, OUTPUT);
    digitalWrite(INT_EXT_OUTPUT_CTL, LOW);

    // As with EXT_OUTPUT_TTL, LOW routes output to RF_OUT and HIGH grounds RF_OUT.
    pinMode(TEENSY_OUTPUT_TTL, OUTPUT);
    digitalWrite(TEENSY_OUTPUT_TTL, LOW);

    // HIGH selects the VCO as the RF source, and LOW selects an external RF source through the EXT_RF SMA port.
    pinMode(TEENSY_RF_TTL, INPUT);
    digitalWrite(TEENSY_RF_TTL, HIGH);

    // HIGH turns the VCO on.
    pinMode(VCO_EN, OUTPUT);
    digitalWrite(VCO_EN,HIGH);

    // Reset the DAC:
    pinMode(DAC_CLR, OUTPUT);
    digitalWrite(DAC_CLR, LOW);
    delayMicroseconds(1);
    digitalWrite(DAC_CLR, HIGH);

    // Holding LDAC low ensures that data written to the external DAC will be processed immediately as output, updating VCO_SETPT.
    pinMode(DAC_LDAC, OUTPUT);
    digitalWrite(DAC_LDAC, LOW);

    // Before data is sent to the DAC, DAC_SYNC must be held LOW. It must remain low for the duration of data transfer.
    pinMode(DAC_SYNC, OUTPUT);
    digitalWrite(DAC_SYNC, HIGH);

    // These pins for the digital step attenuator are not actual CLK and MOSI outputs; they must be emulated when sending data.
    pinMode(ATTEN_CLK, OUTPUT);
    digitalWrite(ATTEN_CLK, LOW);
    pinMode(ATTEN_MOSI, OUTPUT);
    digitalWrite(ATTEN_MOSI, LOW);

    // When sending data to the digital step attenuator, ATTEN_LE must be pulsed high at the end of the transaction.
    pinMode(ATTEN_LE, OUTPUT);
    digitalWrite(ATTEN_LE, LOW);

    pinMode(TOGGLE_SW, INPUT);

    pinMode(ENC_A, INPUT);
    pinMode(ENC_B, INPUT);

    // Initialize the LCD.
    SPI.begin();
    lcd.init();

    // Initialize the encoder and attach an interrupt routine.
    Serial.begin(115200);
    attachInterrupt(ENC_A, encoder_interrupt_wrapper, CHANGE);

    // Menu Select:
    enc_main_menu.init(2 << 14, 0, 65000);
    main_menu.attach_menu_knob(&enc_main_menu);
    main_menu.attach_mode_select(main_menu_select_mode);
    main_menu.attach_mode(0,VCO_tune_mode);
    main_menu.attach_mode(1,RF_atten_mode);
    main_menu.attach_mode(2,int_ext_rf_select_mode);
    main_menu.attach_mode(3,VCO_on_off_mode);

    // Mode 0:
    enc_VCO_freq.init(50859, 42943, 62885);
    enc_VCO_freq.define_step_sizes(3, step_sizes_VCO_freq, step_labels_VCO_freq);
    enc_VCO_freq.reverse_polarity();
    main_menu.attach_knob_to_mode(0, &enc_VCO_freq);

    // Mode 1:
    enc_RF_atten.init(63, 0, 63);
    main_menu.attach_knob_to_mode(1, &enc_RF_atten);

    // Mode 2:
    enc_int_ext_rf_select.init(2 << 14, 0, 65000);

    // Mode 3:
    enc_VCO_on_off.init(2 << 14, 0, 65000);

    // Start with full attenuation:
    write_rf_atten(63);
}



void loop() {
    // The physical toggle switch is Int RF TTL:
    if (digitalRead(TOGGLE_SW) == LOW) {
        digitalWrite(TEENSY_OUTPUT_TTL, HIGH);
    }
    else {
        digitalWrite(TEENSY_OUTPUT_TTL, LOW);
    }

    main_menu.run_current_mode();
}