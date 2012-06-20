/*
 * LCDduino Controller for Twisted Pear Opus DAC and Mux
 * Simeon Walker 2011-2012
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
#include "EEPROM.h"

// local includes
#include "config.h"
#include "lcd1.h"
#include "dac_hw.h"
#include "util.h"

#include "volcontrol.h"
#include "volcontrol_defs.h"   // defines and constants

byte pot_state = MOTOR_INIT;
int pot_last_seen = 0;           // raw values range from 0..1023
byte vol_last_seen;

int pot_read_smoothed () {
    int avg_port_value = 0;
    for (byte i=0; i < POT_REREADS; i++) {
        avg_port_value += analogRead(POT_INPUT_PIN);
        //delayMicroseconds(200);
    }
    if (POT_REREADS > 1) {
        avg_port_value /= POT_REREADS;
    }
    return (avg_port_value);
}

byte handle_pot (byte curr_vol) {
    byte new_vol = curr_vol;
    int pot_value;

    if ( (pot_state == MOTOR_SETTLED) || (pot_state == MOTOR_INIT) ) {
        // Only read pot when stopped
        pot_value = pot_read_smoothed();
        if ( abs(pot_value - pot_last_seen) > POT_CHANGE_THRESHOLD ) {
            // vol returned only changes when pot has actually moved
            new_vol = l_map(pot_value, POT_MIN, POT_MAX, VOL_MIN, VOL_MAX);
            if (new_vol != curr_vol) {
                lcd.restore_backlight();
                update_volume(new_vol);
                pot_last_seen = pot_value;
                vol_last_seen = new_vol;
            }
        }
   }
   return new_vol;
}

#ifdef MOTOR_POT_ENABLED
void handle_motor_pot (byte target_vol) {                           // a state-driven dispatcher
    static int motor_stabilized;
    int sensed_value;
    int target_value = l_map(target_vol, VOL_MIN, VOL_MAX, POT_MIN, POT_MAX);

    switch (pot_state) {
    case MOTOR_INIT:                                    // initial state, just go to 'settled' from here
        pot_state = MOTOR_SETTLED;
        vol_last_seen = target_vol;
        break;

    case MOTOR_SETTLED:
        if (target_vol != vol_last_seen) {              // If volume has changed since last time then
            pot_last_seen = pot_read_smoothed();        // save current value of pot and
            pot_state = MOTOR_IN_MOTION;                // say we're in motion next time around.
            //sensed_value = pot_read_smoothed();
        }
        vol_last_seen = target_vol;
        break;

    case MOTOR_IN_MOTION:
        lcd.restore_backlight();
        motor_stabilized = 0;
        sensed_value = pot_read_smoothed();
        if ( abs(target_value - sensed_value) <= 4 ) {
            motor_drive(LOW, LOW);                          // Stop
            pot_state = MOTOR_COASTING;
            delay(5);                                       // 5ms
            return;
        } else {                                            // not at target volume yet
            if (sensed_value < target_value ) {
                motor_drive(HIGH, LOW);                     // Go CW
            } else if (sensed_value > target_value ) {
                motor_drive(LOW, HIGH);                     // Go CCW
            }
        }
        break;

    case MOTOR_COASTING:
        // we are waiting for the motor to stop (which means the last value == this value)
        lcd.restore_backlight();
        delay(20);
        sensed_value = pot_read_smoothed();
        if (sensed_value == pot_last_seen) {
            if (++motor_stabilized >= 5) {
                pot_state = MOTOR_SETTLED;   // yay!  we reached our target
            }
        } else {
            motor_stabilized = 0;  // we found a value that didn't match, so reset our 'sameness' counter
        }
        pot_last_seen = sensed_value;        // this is the oper value of the pot, from the ADC
        break;

    default:
        ;
    }
    return;
}
#endif

void motor_drive(byte cw, byte ccw) {
    if (cw==HIGH and ccw==HIGH) return;
    digitalWrite(MOTOR_POT_ROTATE_CCW, ccw);
    digitalWrite(MOTOR_POT_ROTATE_CW, cw);
}

// end volcontrol.cpp
