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

#ifndef _VM_COMMON_H_
#define _VM_COMMON_H_

#include "Arduino.h"

// globally visible functions

extern void handle_motor_pot (byte curr_vol);
extern byte handle_pot (byte curr_vol);

extern int pot_read_smoothed (void);
extern void motor_drive(byte ccw, byte cw);

extern boolean toggle_mute (boolean mute_flag);
extern byte update_volume (byte new_vol);
byte volume_up (byte curr_vol);
byte volume_down (byte curr_vol);
extern byte next_dac_filter (byte curr_filter);

#endif // _VM_COMMON_H_
