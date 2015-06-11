#include "Arduino.h"
#include "SPI.h"
#include "TeensyPins.h"
#include "LCD.h"
#include "Encoder.h"
#include "Menu.h"





// Setup the LCD and encoder.
SPISettings SPI_settings_dac(30000000, MSBFIRST, SPI_MODE2);
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

void write_atten(uint8_t data) {
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



void main_menu_press_event(Encoder *this_encoder) {
    lcd.clear();
    main_menu.switch_to_mode(enc_main_menu.position());
}

void about_this_box(Encoder *this_encoder) {
    lcd.write("80 MHz TeensyAOM", 0x00);
    lcd.write("N. Anderson, JQI", 0x40);
    delay(1000);
}

// For the instantiation of the Encoders/Menus, we will break from the Google C++ style guide and use indentation to indicate the nesting structure:
Encoder enc_main_menu(ENC_A, ENC_B, ENC_SW);
    Encoder enc_vco_tune(ENC_A, ENC_B, ENC_SW);
    Encoder enc_attenuation(ENC_A, ENC_B, ENC_SW, RF_atten_press_event, go_to_main_menu);
    Encoder enc_int_ext_output_ctrl(ENC_A, ENC_B, ENC_SW, int_ext_ctrl_press_event, go_to_main_menu);
    Encoder enc_VCO_on_off(ENC_A, ENC_B, ENC_SW, VCO_on_off_press_event, go_to_main_menu);

void menu(Menu* this_menu) {
    lcd.write(this_menu->mode_name() + "       ",0x00);
    lcd.write(this_menu->mode_name(enc_main_menu.position()) + "       ", 0x40);
    enc_main_menu.button_state();
}

float vco_freq(int position) {
    float MHz_per_bit = 50.0/(62885.0-42943.0);
    return (100.0 - MHz_per_bit*(position - 42943.0));
}

void vco_freq_press_event(Menu *this_menu) {
    this_encoder->change_step_size();
}

// This will be the hold event for every menu except main_menu.
void go_to_menu(Menu *this_menu) {
    delay(20);
    this_menu->switch_to_menu();
    lcd.clear();
    lcd.write(this_menu->mode_name() 0x00);
    delay(750);
}

void attenuation_press_event(Encoder *this_encoder) {
    lcd.write("Click!", 0x00);
}

void int_ext_ctrl_press_event(Encoder *this_encoder) {
    if (this_encoder->position() % 2 == 0) {
        digitalWrite(INT_EXT_OUTPUT_CTL, LOW);
        lcd.flash_string("INT TTL CTRL   ",0x40);
    }
    else {
        digitalWrite(INT_EXT_OUTPUT_CTL, HIGH);
        lcd.flash_string("EXT TTL CTRL   ",0x40);
    }
    main_menu.switch_to_menu();
}

void vco_on_off_press_event(Encoder *this_encoder) {
    if (this_encoder->position() % 2 == 0) {
        digitalWrite(VCO_EN, HIGH);
        lcd.flash_string("   --VCO ON--   ",0x40);
    }
    else {
        digitalWrite(VCO_EN, LOW);
        lcd.flash_string("   --VCO OFF--   ",0x40);
    }
    main_menu.switch_to_menu();
}





int step_sizes_VCO_freq[] = {1,4,399};



void mode_VCO_tune() {
    lcd.write("VCO TUNE: " + enc_VCO_freq.step_size_label() + "     ",0x00);
    enc_VCO_freq.button_state();
    write_dac_vco_tune(enc_VCO_freq.position());
    lcd.write("Freq " + String(vco_freq(enc_VCO_freq.position()),3) + " MHz  ",0x40);
}

float percent_attenuation(int value) {
    return (31.5*(float)value)/63;
}

void mode_attenuation() {
    lcd.write("RF ATTEN: LSB   ",0x00);
    lcd.write(String("Atten:   " + String(percent_attenuation(enc_attenuation.position()),1)+" dB "), 0x40);
    enc_attenuation.button_state();
    write_atten(enc_attenuation.position());
}

void mode_int_ext_output_ctrl() {
    lcd.write("SELECT:          ", 0x00);
    if (enc_int_ext_output_ctrl.position() % 2 == 0) {
        lcd.write("INT RF TTL CTRL    ",0x40);
        digitalWrite(INT_EXT_OUTPUT_CTL, LOW);
    }
    else {
        lcd.write("EXT RF TTL CTRL    ",0x40);
        digitalWrite(INT_EXT_OUTPUT_CTL, HIGH);
    }
    enc_int_ext_output_ctrl.button_state();
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

void (*modes[4])() = {mode_VCO_tune, mode_attenuation, mode_int_ext_output_ctrl, VCO_on_off_mode};

void encoder_interrupt_wrapper() {
    if (main_menu.menu_select()) {
        enc_main_menu.interrupt();
    }
    else {
        if (main_menu.current_mode() == 0) {
            enc_VCO_freq.interrupt();
        }
        else if (main_menu.current_mode() == 1) {
            enc_attenuation.interrupt();
        }
        else if (main_menu.current_mode() == 2) {
            enc_int_ext_output_ctrl.interrupt();
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
    main_menu.attach_mode(0,mode_VCO_tune);
    main_menu.attach_mode(1,mode_attenuation);
    main_menu.attach_mode(2,mode_int_ext_output_ctrl);
    main_menu.attach_mode(3,VCO_on_off_mode);

    // Mode 0:
    enc_VCO_freq.init(50859, 42943, 62885);
    enc_VCO_freq.define_step_sizes(3, step_sizes_VCO_freq, step_labels_VCO_freq);
    enc_VCO_freq.reverse_polarity();
    main_menu.attach_knob_to_mode(0, &enc_VCO_freq);

    // Mode 1:
    enc_attenuation.init(63, 0, 63);
    main_menu.attach_knob_to_mode(1, &enc_attenuation);

    // Mode 2:
    enc_int_ext_output_ctrl.init(2 << 14, 0, 65000);

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