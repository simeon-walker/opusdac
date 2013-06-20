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

#include "irlearn.h"

#include "irremote.h"
#include "lcd1.h"
#include "util.h"
#include "opusdac_defs.h"

unsigned long ir_code_cache[IFC_MAX];

void learn_key (int idx , const char *str) {
    boolean blink_toggle = true;
    byte blink_count = 0;
    byte skip_this_key = 0;
    
    lcd.clear_line(LCD_LINE2);
    lcd.send_string(str, LCD_LINE2);               // prompt the user for which key to press
    
    /* non-blocking poll for a keypress */
    while (!skip_this_key) {
        key = get_IR_key();
        if (key != 0) {
            skip_this_key = 0;
            break;                              // exit this blink-loop when we found a key
        }
        //irrecv.resume();                        // we just consumed one key; 'start' to receive the next value
        
        if (blink_toggle) {
            lcd.clear_line(LCD_LINE2);
            delay(150);
        } else {
            ++blink_count;
            lcd.send_string(str, LCD_LINE2);
            delay(300);
        }
        blink_toggle = !blink_toggle;
        
        if (scan_front_button() == 1) {         // Skip code if front button pressed
            skip_this_key = 1;
            lcd.clear_line(LCD_LINE2);
            lcd.send_string("Key Skipped.", LCD_LINE2);
            delay(1000);
            break;
        }
    } // while...waiting for key
    
    if (!skip_this_key) {    // Got key so show code and save
        lcd.send_string(str, LCD_LINE2);
        lcd.send_string("Code: ", LCD_LINE2);
        lcd_print_long_hex(key);
        EEwrite_long(EEPROM_IR_LEARNED_BASE + (idx * sizeof(long)), key);
    }
    delay(1000);                // debounce a little more
    //irrecv.resume();            // we just consumed one key; 'start' to receive the next value
}

void cache_ir_codes (void) {
    // Read IR codes for each function into RAM
    for (int idx=0; idx <= IFC_MAX; idx++) {
        ir_code_cache[idx] = EEread_long(EEPROM_IR_LEARNED_BASE + (idx * sizeof(long)));
    }
}

// IR learn prompts

void ir_learn () {

    lcd.clear();
    lcd.send_string("Learning IR:", LCD_LINE1);
    delay(2000);

    learn_key(IFC_VOLUME_UP, "Volume Up");
    learn_key(IFC_VOLUME_DOWN, "Volume Down");
    learn_key(IFC_MUTE, "Mute");
    learn_key(IFC_POWER_TOGGLE, "Power Toggle");
    learn_key(IFC_POWER_ON, "Power On");
    learn_key(IFC_POWER_OFF, "Power Off");
    learn_key(IFC_NEXT_FILTER, "Next Filter");
    learn_key(IFC_INPUT1, "Input 1");
    learn_key(IFC_INPUT2, "Input 2");
    learn_key(IFC_INPUT3, "Input 3");
    learn_key(IFC_INPUT4, "Input 4");
    learn_key(IFC_SET_CLOCK, "Set Clock");
    learn_key(IFC_BACKLIGHT, "Backlight");

    cache_ir_codes();
    lcd.clear();
    lcd.restore_backlight();
    lcd.send_string("Completed.", LCD_LINE1);
}

