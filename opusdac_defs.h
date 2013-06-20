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

#ifndef _VOLCONTROL_DEFS_H_
#define _VOLCONTROL_DEFS_H_

/*
 * IFC = internal function codes
 *
 * these are logical mappings of physical IR keypad keys to internal callable functions.
 * its the way we soft-map keys on a remote to things that happen when you press those keys.
 */

#define IFC_INVALID_CODE           (-1)
#define IFC_VOLUME_UP                0
#define IFC_VOLUME_DOWN              1
#define IFC_MUTE                     2
#define IFC_POWER_TOGGLE             3
#define IFC_POWER_ON                 4
#define IFC_POWER_OFF                5
#define IFC_NEXT_FILTER              6
#define IFC_INPUT1                   7
#define IFC_INPUT2                   8
#define IFC_INPUT3                   9
#define IFC_INPUT4                  10
#define IFC_SET_CLOCK               11
#define IFC_BACKLIGHT               12
// don't touch this, it has to be the last entry in this list
#define IFC_MAX                     13   // this must always be the last in the list

#define MY_MAGIC                  0xf4   // if magic doesn't match the re-init eeprom
/*
 * eeprom locations (slot numbers in eeprom address space)
 */

#define EEPROM_VERSION               0   // v1 = '1'
#define EEPROM_MAGIC                 1   // later, replace this with a 2-byte CRC
#define EEPROM_POWER                 2   // last-used power on/off setting
#define EEPROM_VOLUME                3   // used only via serial 'console'
#define EEPROM_INPUT_SEL             4   // last-used input selector
#define EEPROM_DAC_FILTER            5   // last-used DAC filter
#define EEPROM_POWERON_DELAY         6   // 0..90

#define EEPROM_NORMAL_BACKLIGHT_MODE  7   // our current backlight mode
#define EEPROM_STANDBY_BACKLIGHT_MODE 8   // our standby backlight mode

// block of 4 port 'last used' volume values
#define EEPROM_INPUT_BASE           10
//#define PORT1                     11
//#define PORT2                     12
//#define PORT3                     13

// IR learned keycodes (as 4-byte longwords, each) are stored here.
// table is IFC_MAX+1 deep, with each record being 4 bytes.
// currently, IFC_MAX is 13 and so 13*4=52
#define EEPROM_IR_LEARNED_BASE      50  // last byte = 102

// end EEPROM defs

/* enum values */

// system power
#define POWER_OFF                   (0)
#define POWER_ON                    (1)

// states of motor-pot logic
#define MOTOR_INIT                  (1)
#define MOTOR_SETTLED               (2)    // motor pot is at resting state
#define MOTOR_IN_MOTION             (3)    // motor pot is moving right now
#define MOTOR_COASTING              (4)    // motor pot just passed its destination and motor current is released

#endif // _VOLCONTROL_DEFS_H_

// end volcontrol_defs.h
