/*
	TeensyPins.h  - TeensyAOM control library for Teensy 3.1 powered AOM driver.

	Created by Neil Anderson, 2015
	JQI - Joint Quantum Institute

	TeensyAOM is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.


	TeensyAOM  is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

// LCD pins
#define LCD_RST 6
#define LCD_RS 7
#define LCD_CS 8
// Rotary encoder pins
#define ENC_A 2
#define ENC_B 1
#define ENC_SW 0

#define ENC_A2 5
#define ENC_B2 4
#define ENC_SW2 3


// Toggle switch
#define TOGGLE_SW 30  // SW_3
#define SW4 29



#define INT_EXT_RF_CTL 9
#define TEENSY_RF_TTL 20// INT_RF_TTL
#define INT_EXT_OUTPUT_CTL 28
#define TEENSY_OUTPUT_TTL 27 //INT_OUTPUT_TTL
#define SERVO_CTL 31 //SEVO_SEL0
#define SETPT_CTL 32 //SERVO_SEL1
#define VCO_EN 21
#define EXT_DITHER_CTL 31 //REMAPPED TO SEVO_SEL0 AGAIN!
#define MOSI 11
#define SCK 13
#define DAC_CLR 16
#define DAC_LDAC 15
#define DAC_RST  31 // NO NEED FOR IT! REMAPPED TO SERVO_SEL0 AGAIN!!!
#define DAC_SYNC 14
#define SDA 18
#define SCL 19
#define ATTEN_CLK  25 //ATTEN_SCK
#define ATTEN_MOSI 24
#define ATTEN_LE 26
