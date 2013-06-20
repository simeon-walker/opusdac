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

// system includes
#include "Arduino.h"
#include <ctype.h>
#include <EEPROM.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
//#include <avr/pgmspace.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <string.h>
#include <Wire.h>
#include <math.h>

// local includes
#include "config.h"
#include "opusdac_defs.h"

#include "lcd1.h"
#include "motorpot.h"
#include "irremote.h"
#include "irlearn.h"
#include "rtc.h"
#include "dac_hw.h"
#include "util.h"

// forward defs
void setup (void);
void loop (void);

void init_system (void);
void init_eeprom (void);
void power_off_logic (void);
void power_on_logic (boolean cold_start);
void enter_setup_mode (void);
void read_eeprom_oper_values (void);
void handle_keys_normal (void);
void handle_keys_standby (void);

boolean check_config_button (unsigned long timeout);
void next_backlight_mode (void);

byte update_volume (byte new_vol);
void select_input (byte);

void draw_volume (byte volume);
void draw_input (byte input);
void draw_status (void);
void draw_clock (byte admin_forced);
void blink_led13 (byte on_off_flag);

// globals
boolean power_on = false;
boolean mute = false;
byte volume;
byte current_input = (1);
byte filter = (2);

extern unsigned long ir_code_cache[IFC_MAX];            // from irlearn.cpp

IRrecv irrecv(IR_PIN);                                  // create an instance of the IR receiver class
DS1302 rtc = DS1302();                                  // create an RTC instance
signed char last_mins;

// create an instance of the lcd i2c 4bit class.
// this does a lot of stuff!  you get back a 'filled in' variable called 'lcd'
// and the screen is initialized and ready for you to write to.
LCDI2C4Bit lcd = LCDI2C4Bit(LCD_MCP_DEV_ADDR, 2, 16, PWM_BACKLIGHT_PIN);
byte lcd_in_use_flag;

/***********************************  start of main C code **********************************/

void setup (void) {                                     // **** REAL POWER-ON STARTS HERE ****
    Wire.begin();                                       // Always start the wire lib first (or very early)
    pinMode(AMP_RELAY_PIN, OUTPUT);                     // Setup Amp relay pin and
    digitalWrite(AMP_RELAY_PIN, LOW);                   // make sure relay is open
    pinMode(DAC_RELAY_PIN, OUTPUT);                     // Setup DAC relay pin and
    digitalWrite(DAC_RELAY_PIN, LOW);                   // make sure DAC is OFF at boot time
    pinMode(MUX_A0_PIN, OUTPUT);                        // Configure Mux control pins
    pinMode(MUX_A1_PIN, OUTPUT);
    pinMode(POT_INPUT_PIN, INPUT);                      // Analog-in 1
#ifdef MOTOR_POT_ENABLED
    pinMode(MOTOR_POT_ROTATE_CW,  OUTPUT);              // Analog-2 used as digital-15
    pinMode(MOTOR_POT_ROTATE_CCW, OUTPUT);              // Analog-3 used as digital-15
    motor_drive(LOW, LOW);                              // Stop motor
#endif
    digitalWrite(LED13, LOW);                           // init led13 to OFF
    if (EEPROM.read(EEPROM_MAGIC) != MY_MAGIC) {        // if this is the first time we are running,
        init_eeprom();                                  // init some eeprom area
        EEPROM.write(EEPROM_MAGIC, MY_MAGIC);
    }
    rtc.init();                                         // realtime clock chip init
    lcd.SetInputKeysMask(LCD_MCP_INPUT_PINS_MASK);
    lcd.init();
    lcd.backlight_min = BACKLIGHT_MIN;
    lcd.backlight_max = BACKLIGHT_MAX;
    lcd.load_bignum();                                  // Load big numeral font
    read_eeprom_oper_values();
    cache_ir_codes();                                   // Get learned IR codes
    irrecv.enableIRIn();                                // IR receiver init
    if (power_on) {                                     // system ON (fresh boot)
        lcd.turn_display_on();
        lcd.restore_backlight();                        // if power is ON, lcd must be visible ;)
        power_on_logic(true);                           // power-on from cold
    } else {
        lcd.clear();
        if (lcd.backlight_mode != BACKLIGHT_OFF) {
            lcd.turn_display_on();
            lcd.restore_backlight();
        }
    }
}

// this routine assumes the LCD display is init'd and available for us to write status messages to
void power_on_logic (boolean cold_start) {
    power_on = true;
    EEPROM.write(EEPROM_POWER, power_on);
    digitalWrite(DAC_RELAY_PIN, HIGH);                  // turn on DAC
    dac_init();                                         // init dac to mute and min vol

    if (cold_start) {
        lcd.clear();
        lcd.fade_backlight_on();
        lcd.send_string("Opus AMB LCDuino", LCD_LINE1);
        if (check_config_button(2000)) {           // check if the user wanted to enter 'setup mode'
            enter_setup_mode();                         // user stays in this mode until he's done, then control returns
        }
    }
    lcd.clear();
    lcd.backlight_mode = EEPROM.read(EEPROM_NORMAL_BACKLIGHT_MODE);
    select_input(current_input);                        // Select input, set volume and draw display
    dac_select_filter(filter);                          // Set DAC filter mode to saved state
    dac_mute(mute);                                     // Set mute to saved state
    draw_status();
    digitalWrite(AMP_RELAY_PIN, HIGH);                  // push Amp 'button'
    delay(AMP_RELAY_MS);
    digitalWrite(AMP_RELAY_PIN, LOW);                   // release Amp 'button'
}


void power_off_logic (void) {
    lcd.clear();
    lcd.send_string("     Amp Off", LCD_LINE1);
    digitalWrite(AMP_RELAY_PIN, HIGH);                  // push Amp 'button'
    delay(AMP_RELAY_MS);
    digitalWrite(AMP_RELAY_PIN, LOW);                   // release Amp 'button'
#ifdef MOTOR_POT_ENABLED
    motor_drive(LOW, LOW);                              // Stop motor
#endif
    dac_mute(true);                                     // total hardware mute
    delay(DAC_OFF_DELAY);
    lcd.send_string("     DAC Off", LCD_LINE2);
    digitalWrite(DAC_RELAY_PIN, LOW);                   // turn off DAC
    power_on = false;
    EEPROM.write(EEPROM_POWER, power_on);
    delay(1000);
    lcd.clear();
    lcd.backlight_mode = EEPROM.read(EEPROM_STANDBY_BACKLIGHT_MODE);
    draw_clock(1);                                      // Force first draw of clock
}

/* we get here if the user pressed the magic 'config button' in a short window at bootup time */
void enter_setup_mode (void) {
    lcd.clear();
    lcd.backLight(255);  // force it to be the max
    // wait 3-5 seconds, total, just to give the user a GOOD chance to remove his finger, so he won't cause a false-pos
    for (int i=0; i<4; i++) {
        lcd.send_string("Config Mode!", LCD_LINE1);
        if (i == 3) break;  // avoid clearing the display when we are done
        delay(300);
        lcd.clear();
        delay(70);
    }
    // Restore EEPROM defaults?
    delay(1000);
    lcd.clear();
    lcd.send_string("Restore Default?", LCD_LINE1);
    if (check_config_button(2000)) {
        lcd.send_string("Restored default", LCD_LINE2);
        init_eeprom();
        read_eeprom_oper_values();
    }
    // Learn IR commands?
    delay(1000);
    lcd.clear();
    lcd.send_string("Learn IR?", LCD_LINE1);
    if (check_config_button(2000)) {
        irrecv.resume();                                // we just consumed one key; 'start' to receive the next value
        // ir_learn()
    }
    delay(500);
    lcd.clear();
}

void loop (void) {
    if (!power_on) {                                    // service/display clock and some IR commands (but minimal)
        if (lcd.backlight_mode != BACKLIGHT_OFF) {
            draw_clock(0);
        }
        handle_keys_standby();
    } else {
        volume = handle_pot(volume);                    // Update volume IFF pot moved
#ifdef MOTOR_POT_ENABLED
        handle_motor_pot(volume);                       // always check the motor logic 2nd
#endif
        handle_keys_normal();                           // IR keyscan and case-statement routines
    }
    lcd.handle_backlight_auto();                        // timeout the lcd backlight if we are in the right mode
}


void handle_keys_standby (void) {                       // key handling when system is 'off'
    if (scan_front_button()) {
        delay(150);                                     // debounce
        power_on_logic(1);                              // 1 = 'show banner & wait'
        irrecv.resume();
        return;
    }
    key = get_IR_key();                                 // we got a valid IR start pulse! fetch the keycode, now.
    if (key == 0) {
        return;                                         // try again to sync up on an IR start-pulse
    } else if (key == ir_code_cache[IFC_POWER_TOGGLE] or key == ir_code_cache[IFC_POWER_ON]) {
        lcd.restore_backlight();
        blink_led13(1);
        power_on_logic(0);                              // 0 = no banner, quick startup
        delay(400);
    } else if (key == ir_code_cache[IFC_BACKLIGHT]) {
        blink_led13(1);
        next_backlight_mode();
        EEPROM.write(EEPROM_STANDBY_BACKLIGHT_MODE, lcd.backlight_mode);
        draw_clock(1);                                  // Force redraw of clock
    }
    blink_led13(0);
    irrecv.resume();                                    // we just consumed one key; 'start' to receive the next value
}


void handle_keys_normal (void) {                        // key handling when system is on
    int i;
    byte let_go;
    if (scan_front_button()) {                          // one-shot check if the front button was pressed
        let_go = 0;
        for (i=0; i<15; i++) {                      // if the button is held for > 2 seconds then turn power off
            if (!scan_front_button()) {
                let_go = 1;
                break;                              // see if he held it down long enough to mean 'power-off'
            }
            delay(150);
        }
        if (!let_go) {                              // turn power off if the user really asked us to
            power_off_logic();
            delay(1000);                            // debounce
            irrecv.resume();                        // we just consumed one key; 'start' to receive the next value
            return;
        }
        lcd.restore_backlight();                    // not a power-off request, so restore backlight
        mute = !mute;
        dac_mute(mute);
        draw_status();
        delay(250);                                 // Debounce switch
        irrecv.resume();                            // we just consumed one key; 'start' to receive the next value
        return;
    }
    key = get_IR_key();                             // we got a valid IR start pulse! fetch the keycode, now.
    if (key == 0) {
        return;                                     // try again to sync up on an IR start-pulse
    } else if (key == ir_code_cache[IFC_VOLUME_UP]) {
        blink_led13(1);
        if (volume < VOL_MAX) {
            volume = update_volume(volume+1);
            delay(VOL_DELAY_SHORT);
        }
    } else if (key == ir_code_cache[IFC_VOLUME_DOWN]) {
        blink_led13(1);
        if (volume > VOL_MIN) {
            volume = update_volume(volume-1);
            delay(VOL_DELAY_SHORT);
        }
    } else if (key == ir_code_cache[IFC_MUTE]) {
        blink_led13(1);
        mute = !mute;
        dac_mute(mute);
        draw_status();
        delay(400);
    } else if (key == ir_code_cache[IFC_INPUT1]) {
        blink_led13(1);
        select_input(0);
        delay(400);
    } else if (key == ir_code_cache[IFC_INPUT2]) {
        blink_led13(1);
        select_input(1);
        delay(400);
    } else if (key == ir_code_cache[IFC_INPUT3]) {
        blink_led13(1);
        select_input(2);
        delay(400);
    } else if (key == ir_code_cache[IFC_INPUT4]) {
        blink_led13(1);
        select_input(3);
        delay(400);
    } else if (key == ir_code_cache[IFC_POWER_OFF]) {
        blink_led13(1);
        power_off_logic();
        delay(400);
    } else if (key == ir_code_cache[IFC_POWER_TOGGLE]) {
        blink_led13(1);
        power_off_logic();
        delay(400);
    } else if (key == ir_code_cache[IFC_NEXT_FILTER]) {
        blink_led13(1);
        lcd.restore_backlight();
        if (filter < 5) {
            filter++;
        } else {
            filter = 1;
        }
        dac_select_filter(filter);
        EEPROM.write(EEPROM_DAC_FILTER, filter);
        draw_status();
        delay(400);
    } else if (key == ir_code_cache[IFC_SET_CLOCK]) {
        blink_led13(1);
        // TODO clock setting
        delay(400);
    } else if (key == ir_code_cache[IFC_BACKLIGHT]) {
        blink_led13(1);
        next_backlight_mode();
        EEPROM.write(EEPROM_NORMAL_BACKLIGHT_MODE, lcd.backlight_mode);
        draw_input(current_input);
        draw_status();
        draw_volume(volume);
    }
    /* ************************************************************************************** *
     * common exit: everyone goes here to have their LED turned off and have IR rescan itself *
     * ************************************************************************************** */
    blink_led13(0);
    irrecv.resume(); // we just consumed one key; 'start' to receive the next value
}

/*************************************** End of main code *************************************/

void next_backlight_mode(void) {
    lcd.clear();
    lcd.send_string("Backlight Mode:", LCD_LINE1);

    if (lcd.backlight_mode == BACKLIGHT_OFF) {
        lcd.restore_backlight();
        lcd.backlight_mode = BACKLIGHT_AUTODIM;
        lcd.send_string(" AUTO-DIM", LCD_LINE2);

    } else if (lcd.backlight_mode == BACKLIGHT_AUTODIM) {
        lcd.restore_backlight();
        lcd.backlight_mode = BACKLIGHT_ON;
        lcd.send_string(" ON", LCD_LINE2);

    } else if (lcd.backlight_mode == BACKLIGHT_ON) {
        lcd.backlight_mode = BACKLIGHT_OFF;
        lcd.send_string(" OFF", LCD_LINE2);

    } else {
        lcd.backlight_mode = BACKLIGHT_AUTODIM;
    }
    delay(1000);
    lcd.clear();
}

boolean check_config_button (unsigned long timeout) {      // Wait until timout for button to be pressed
    unsigned long start_time = millis();
    while ( abs(millis() - start_time) <= timeout) {
        if (scan_front_button() == 1) {
            return 1;
        }
        delay(20);
    }
    return 0;
}

void draw_clock (byte admin_forced) {
    rtc.GetTime();
    // only re-display clock if the mins have changed OR if admin-forced was 'true'
    if ( (rtc.Mins != last_mins) || (admin_forced == 1) ) {
        byte ms, ls;
        bcd2ascii(rtc.Hrs, &ms, &ls);
        lcd.draw_bignum_at(ms, 0);                  // num: 0,1,2 space: 3
        lcd.draw_bignum_at(ls, 4);                  // num: 4,5,6 space: 7
        lcd.cursorTo(0,7);
        lcd.write(7);
        lcd.cursorTo(1,7);
        lcd.write(7);
        bcd2ascii(rtc.Mins, &ms, &ls);
        lcd.draw_bignum_at(ms, 9);                  // num: 9,10,11 space: 12
        lcd.draw_bignum_at(ls, 13);                 // num: 13,14,15

        last_mins = rtc.Mins;
    }
}

void draw_volume (byte volume) {
    byte ms, ls;
    bin2ascii(volume, &ms, &ls);
    lcd.draw_bignum_at(ms, 9);                  // cols 9,10,11
    lcd.draw_bignum_at(ls, 13);                 // cols 13,14,15
}

byte update_volume (byte new_vol) {
    if (new_vol < VOL_MIN) new_vol = VOL_MIN;
    if (new_vol > VOL_MAX) new_vol = VOL_MAX;
    draw_volume(new_vol);
    dac_set_volume(new_vol);
    return new_vol;
}

void select_input (byte new_input) {
    if (new_input > MUX_INPUT_COUNT-1) {                    // if we had a crazy value, force it to be the first port
        new_input = 0;
    }
    EEPROM.write(EEPROM_INPUT_BASE+current_input, volume);  // save volume of current input

    mux_select_input(new_input);
    EEPROM.write(EEPROM_INPUT_SEL, new_input);              // save new input selection

    volume = EEPROM.read(EEPROM_INPUT_BASE+new_input);      // read saved volume of new input
    update_volume(volume);                                  // set volume for new input (doesn't affect mute)

    current_input = new_input;                              // record new input
    draw_input(new_input);
}

void draw_input (byte input) {
    lcd.draw_bignum_at(input+'1', 0);           // cols 0,1,2
}

void read_eeprom_oper_values (void) {
    power_on                    = EEPROM.read(EEPROM_POWER);        // Last power state
    filter                      = EEPROM.read(EEPROM_DAC_FILTER);
    current_input               = EEPROM.read(EEPROM_INPUT_SEL);
    volume                      = EEPROM.read(EEPROM_INPUT_BASE+current_input);
    if (power_on) {
        lcd.backlight_mode      = EEPROM.read(EEPROM_NORMAL_BACKLIGHT_MODE);
    } else {
        lcd.backlight_mode      = EEPROM.read(EEPROM_STANDBY_BACKLIGHT_MODE);
    }
}

void draw_status () {
    char string_buf[17];    // usually 16+1 (1 for the nullbyte)

    if (mute) {
        lcd.send_string("MUTE", LCD_LINE1+4);
    } else {
        lcd.send_string("    ", LCD_LINE1+4);
    }
    sprintf(string_buf, "f:%0d ", filter);
    lcd.send_string(string_buf, LCD_LINE2+4);
}

void init_eeprom (void) {
    EEPROM.write(EEPROM_VERSION, 0x01);
    EEPROM.write(EEPROM_POWER, power_on);
    EEPROM.write(EEPROM_INPUT_SEL, current_input);
    EEPROM.write(EEPROM_DAC_FILTER, filter);
    EEPROM.write(EEPROM_NORMAL_BACKLIGHT_MODE, BACKLIGHT_ON);
    EEPROM.write(EEPROM_STANDBY_BACKLIGHT_MODE, BACKLIGHT_AUTODIM);
}

// end sketch
