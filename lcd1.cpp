/*
 * LCDduino Controller for Twisted Pear Opus DAC and Mux
 * Simeon Walker 2011-2013
 *
 * Derived from Volu-Master(tm) by Bryan Levin (Linux-Works Labs)
 * Copyright (c) 2009-2011 Bryan Levin
 * Project website: http://www.amb.org/audio/lcduino-1/
 *
 *  LICENSE
 *  -------
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Arduino.h"
#include <Wire.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "config.h"
#include "lcd1.h"
#include "lcd1_defs.h"
#include "opusdac_defs.h"   // defines and constants
#include "util.h"

const byte font_chars[] PROGMEM = {
    B11111, B11111, B11111, B00000, B11111, B00000, B00000, B00000,
    B11111, B11111, B11111, B00000, B11111, B00000, B00000, B00000,
    B00000, B00000, B00000, B00000, B00000, B00000, B00000, B00000,
    B11111, B00000, B00000, B11111, B11111, B00000, B00011, B00000,
    B11111, B00000, B00000, B11111, B11111, B00000, B00011, B00000,
    B00000, B00000, B00000, B00000, B00000, B00000, B00000, B00000,
    B11111, B00000, B11111, B00000, B00000, B11111, B00000, B00000,
    B11111, B00000, B11111, B00000, B00000, B11111, B00000, B00000
//  1       2       3       4       5       6       7       8
};
//                              0      1        2      3      4        5      6        7        8       9
const byte bignumchars1[] PROGMEM = { 1,2,1, 32,1,32, 3,3,1, 2,3,1, 1,6,1,   1,3,3, 1,3,3,  2,2,1,   1,3,1, 1,3,1 };
const byte bignumchars2[] PROGMEM = { 5,4,5, 32,5,32, 5,4,4, 4,4,5, 32,32,5, 4,4,5, 5,4,5,  32,32,5, 5,4,5, 32,32,5 };

extern byte lcd_in_use_flag;

// constructor
LCDI2C4Bit::LCDI2C4Bit (byte devI2CAddress, byte num_lines, byte lcdwidth, byte backlightPin) {
    myNumLines = num_lines;
    myWidth = lcdwidth;
    lcd_i2c_address = devI2CAddress;
    myBacklightPin = backlightPin;

    // backlight related things
    backlight_mode = BACKLIGHT_AUTODIM;                 // default
    backlight_currently_on = 1;                         // boolean
    one_second_counter_ts = 0;
    seconds = 0;
    lcd_inactivity_timeout = 4;                         // in seconds: how long do we keep the 'bright' setting before we fade
    backlight_admin = 0;                                // administratively set (enable auto timeout; normal mode)
    pinMode(myBacklightPin, OUTPUT);                    // pwm backlight
}

void LCDI2C4Bit::init (void) {
    lcd_in_use_flag = 1;                                // set a mutex so that the IR isr won't do anything while we are doing lcd i/o
    dataPlusMask = 0;                                   // clear our mask
    SetMCPReg(MCP_REG_IOCON, 0x0C);                     // setup the MCP port expander chip (not yet talking to the lcd at this point)
    delay(50);
    SetMCPReg(MCP_REG_IODIR, myInputKeysMask);
    delay(50);
    SetMCPReg(MCP_REG_GPPU, myInputKeysMask);
    delay(50);
    SendToLCD(0x03);
    delay(5);
    SendToLCD(0x03);
    delayMicroseconds(100);
    SendToLCD(0x03);
    delay(5);
    SendToLCD(0x02);
    WriteLCDByte(0x28);
    WriteLCDByte(0x0C);                                 // display ON, no cursor, no blink
    delayMicroseconds(60);
    command(0x01);                                      // clear display
    lcd_in_use_flag = 0;                                // clear that mutex (so that the IR isr CAN now do things)
}

void LCDI2C4Bit::SetInputKeysMask (byte input_keys_mask) {
    myInputKeysMask = input_keys_mask;
}

void LCDI2C4Bit::SetMCPReg (byte reg, byte val) {       // set one byte using the wire (i2c) interface
    Wire.beginTransmission(lcd_i2c_address);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

byte LCDI2C4Bit::GetMCPReg (byte reg) {                 // call the 'wire' routine (i2c) and get 1 byte back
    byte val = 0;
    Wire.beginTransmission(lcd_i2c_address);
    Wire.write(reg);
    Wire.endTransmission();
    val = Wire.requestFrom((byte)lcd_i2c_address, (byte)1);  // read 1 byte
    if (Wire.available()) {
        val = Wire.read();
    }
    return val;
}

byte LCDI2C4Bit::ReadInputKeys (void) {                 // 2 extra lines on the MCP chip that we can map to soft pushbuttons
    byte data;
    data = GetMCPReg(MCP_REG_GPIO);
    return data;
}

// lower layer routine that does the enable-bit 'wrapping' of a 4bit parallel transaction
void LCDI2C4Bit::SendToLCD (byte data) {
    data |= dataPlusMask;
    SetMCPReg(MCP_REG_OLAT, data);
    delayMicroseconds(9);
    data ^= 0x80; // 'enable' bit ON on LCD
    SetMCPReg(MCP_REG_OLAT, data);
    delayMicroseconds(9);
    data ^= 0x80; // 'enable' bit OFF on LCD
    SetMCPReg(MCP_REG_OLAT, data);
    delayMicroseconds(9);
}

// higher layer interface; this takes an 8-bit data and makes 2 calls to send to the lcd, 4 bits at a time
void LCDI2C4Bit::WriteLCDByte (byte bdata) {
    lcd_in_use_flag = 1;                                   // set a mutex so that the IR isr won't do anything while we are doing lcd i/o
    SendToLCD (bdata >> 4);
    delayMicroseconds(9);
    SendToLCD (bdata & 0x0F);
    delayMicroseconds(9);
    lcd_in_use_flag = 0;                                  // clear that mutex (so that the IR isr CAN now do things)
}

// 'write' sends character data, setting RS high (the other routine, 'commands' sends non-char data)
void LCDI2C4Bit::write (byte value) {
    dataPlusMask |= 0x10; // RS is set
    WriteLCDByte(value);
    dataPlusMask ^= 0x10; // RS is cleared
}

// 'command' leaves RS low when it sends bytes
void LCDI2C4Bit::command (byte command) {
    // RS - leave low
    WriteLCDByte(command);
    delayMicroseconds(800);
}

void LCDI2C4Bit::print (char value[]) {
    for (char *p = value; *p != 0; p++) {
        write(*p);
    }
}

int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };

// row,col are both zero-based
void LCDI2C4Bit::cursorTo (byte row, byte col) {
    command(0x80 | (col + row_offsets[row]));
}

// 'str' MUST be null-terminated
void LCDI2C4Bit::send_string (const char *str, const byte addr) {
    //  Send string at addr, if addr <> 0, or cursor position  if addr == 0
    if (addr != 0) {
        command(addr); // cursor pos
    }
    print((char *)str);
}

void LCDI2C4Bit::load_bignum (void) {
    byte a;
    byte x;
    byte y;
    // load special graphic chars into 'cgram'
    command(0x40);                                      // start off writing to CG RAM char 0
    for (x=0; x < LCD_USER_CHARS; x++) {                               // load the font set
        a = ( (x+1) << 3 ) | 0x40;
        for (y=0; y < LCD_CHAR_ROWS; y++) {
            command(a++);                               // write the character data to the character generator ram
            write(pgm_read_byte(&font_chars[y * LCD_USER_CHARS + x]));
        }
    }
    command(0x80);                                      // reset to dram mode
}
// char_pos: 0..15 (native lcd columns)
// val: ' ' or '0'..'9'
void LCDI2C4Bit::draw_bignum_at (byte val, byte char_pos) {
    byte j;
    if (val == ' ') {                           // normally we only want 0..9 but if a ' ' comes in, simulate a space
        command(LCD_LINE1 + char_pos);
        print((char *)"   ");
        command(LCD_LINE2 + char_pos);       // bottom half
        print((char *)"   ");
    } else {
        val -= '0';                                     // convert ascii char to binary value
        val *= 3;                                       // groups of 3 bytes in the struct/array
        command(LCD_LINE1 + char_pos);       // top half of char
        for (j=0; j<3; j++) {
            write(pgm_read_byte(&(bignumchars1[val + j])));
        }
        command(LCD_LINE2 + char_pos);
        for (j=0; j<3; j++) {                           // bottom half
            write(pgm_read_byte(&(bignumchars2[val + j])));
        }
    }
}


void LCDI2C4Bit::clear (void) {
    command(CMD_CLR);
}

void LCDI2C4Bit::clear_line (byte line_num) {
    command(line_num);
    for (byte col=0; col < LCD_COLS; col++) {
        write(' ');
    }
}

void LCDI2C4Bit::turn_display_off (void) {
    analogWrite(myBacklightPin, 0);                        // zero brightness
    backlight_currently_on = 0;                            // flag it as off now
    clear();
}

void LCDI2C4Bit::turn_display_on (void) {
    analogWrite(myBacklightPin, backlight_max);
}

void LCDI2C4Bit::backLight (byte value) {
    analogWrite(myBacklightPin, value);                    // 0..255 gets us 'dark' to 'very bright'
}

void LCDI2C4Bit::fade_backlight_on (void) {
    for (int bl=backlight_min; bl < backlight_max; bl++) {
        backLight(bl);                                     // restore to normal brightness again
        delay(2);
    }
    backlight_currently_on = 1;                            // flag it as now on
    seconds = 0;                                           // reset things so we start the count-down all over
    one_second_counter_ts = millis();
}

void LCDI2C4Bit::fade_backlight_to_dim (void) {
    for (int bl = backlight_max; bl > backlight_min; bl--) {
        backLight(bl);                                     // temporarily turn backlight entirely off (or go to its lowest 'low' setting)
        delay(2);
    }
    backlight_currently_on = 0;                            // flag it as now off
}

void LCDI2C4Bit::fade_backlight_off (void) {
    for (int bl = backlight_max; bl > 0; bl--) {
        backLight(bl);                                     // temporarily turn backlight entirely off (or go to its lowest 'low' setting)
        delay(2);
    }
    backlight_currently_on = 0;                            // flag it as now off
    clear();
}

void LCDI2C4Bit::restore_backlight (void) {
    if (backlight_currently_on == 0) {
        backLight(backlight_max);                          // restore to normal brightness again
        backlight_currently_on = 1;
    }
    seconds = 0;                                           // reset things so we start the count-down all over
    one_second_counter_ts = millis();
}

void LCDI2C4Bit::handle_backlight_auto (void) {            // backlight time-out logic
    if (backlight_currently_on == 1) {
        if ( abs(millis() - one_second_counter_ts) >= 1000) {
            seconds++;
            one_second_counter_ts = millis();              // reset ourself
        }
        if (seconds >= lcd_inactivity_timeout) {
            if (backlight_mode == BACKLIGHT_OFF) {
                fade_backlight_off();             // this also sets 'backlight_currently_on to 0'
            } else if (backlight_mode == BACKLIGHT_AUTODIM) {
                fade_backlight_to_dim();                      // this also sets 'backlight_currently_on to 0'
            }
        }
    }
}

void lcd_print_long_hex (long p_value) {
    byte byte1 = ((p_value >> 0) & 0xFF);
    byte byte2 = ((p_value >> 8) & 0xFF);
    byte byte3 = ((p_value >> 16) & 0xFF);
    byte byte4 = ((p_value >> 24) & 0xFF);
    byte ls,ms;

    hex2ascii(byte4, &ms, &ls);
    lcd.write(ms);
    lcd.write(ls);

    hex2ascii(byte3, &ms, &ls);
    lcd.write(ms);
    lcd.write(ls);

    hex2ascii(byte2, &ms, &ls);
    lcd.write(ms);
    lcd.write(ls);

    hex2ascii(byte1, &ms, &ls);
    lcd.write(ms);
    lcd.write(ls);
}

void blink_led13 (byte on_off_flag) {
    if (on_off_flag == 1) {
        PORTB |= ((byte)B00100000);  // turn pin 13 LED on
    } else {
        PORTB &= ((byte)B11011111);  // turn pin 13 LED off
    }
}

boolean scan_front_button (void) {
    byte in_keys = lcd.ReadInputKeys();
    if ( (in_keys & LCD_MCP_INPUT_PINS_MASK /*B01100000*/) != LCD_MCP_INPUT_PINS_MASK) {
        return true;
    } else {
        return false;
    }
}

// end lcd1.cpp
