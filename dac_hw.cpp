/*
 * LCDduino Controller for Twisted Pear Opus DAC and Mux
 * Simeon Walker 2011-2012
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
#include "dac_hw.h"

// The write to WM8741 DAC
void wm8741_write(byte reg, byte val) {
    Wire.beginTransmission(WM8741_I2C_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

void dac_init (void) {
    delay(200);
    wm8741_write(WM8741_REG_RESET, WM8741_RESET);        // reset chip
    delay(200);
    wm8741_write(WM8741_REG_VOLCTRL, WM8741_VOLMUTE);    // soft mute
    //wm8741_write(WM8741_REG_FORMAT, WM8741_FMT24I2S);    // 24bit i2s input format (from mux)
    wm8741_write(WM8741_REG_MODE1, WM8741_OSRHIGH);      // OSR for 192 from Metronome
    wm8741_write(WM8741_REG_DACLLSB, 0x1F);              // max attn lsb
    wm8741_write(WM8741_REG_DACLMSB, 0x3F);              // max attn msb and apply
}


void dac_set_volume (byte vol) {
    // write lower 5 bits of attenuation
    wm8741_write(WM8741_REG_DACLLSB, ( (VOL_MAX-vol) & 0x07 ) << 3 );
    // write upper 5 bits of attenuation (and update)
    wm8741_write(WM8741_REG_DACLMSB, ( (VOL_MAX-vol) >> 2 ) | WM8741_BIT_VOLUPD );
}


void dac_mute (byte mute_flag) {
    if (mute_flag) {
        wm8741_write(WM8741_REG_VOLCTRL, WM8741_VOLMUTE);
    } else {
        wm8741_write(WM8741_REG_VOLCTRL, WM8741_VOLDEFS);
    }
}


void dac_select_filter (byte filter) {
    if (filter == 1) {
        wm8741_write(WM8741_REG_FILTER, WM8741_FILTER1);
    } else if (filter == 2) {
        wm8741_write(WM8741_REG_FILTER, WM8741_FILTER2);
    } else if (filter == 3) {
        wm8741_write(WM8741_REG_FILTER, WM8741_FILTER3);
    } else if (filter == 4) {
        wm8741_write(WM8741_REG_FILTER, WM8741_FILTER4);
    } else if (filter == 5) {
        wm8741_write(WM8741_REG_FILTER, WM8741_FILTER5);
    }
}

void mux_select_input (byte input) {
    digitalWrite(MUX_A0_PIN, (input & 0x01 ));         // bit0 of input number (0-3)
    digitalWrite(MUX_A1_PIN, (input & 0x02 ) >> 1 );   // bit1, shifted right for pin value
    return;
}

