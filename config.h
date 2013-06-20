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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#define MOTOR_POT_ENABLED               1               // Use motor in pot
#define MUX_INPUT_COUNT                 4               // Mux inputs

//#define DEBUG_SENSED_POT  1                           // to see the values as you turn the analog/linear pot
//#define USE_MEM_CHECKER   1                           // to see how much free RAM is left at runtime

//#define THERM_SENSOR_INPUT_PIN    0                   // first analog-in pin (port14)
#define POT_INPUT_PIN                   1               // second analog-in pin (port15)
#define CLK1302_PIN                     2               // DS1302 clock (1302 pin-7)
#define DAT1302_PIN                     3               // DS1302 i/o   (1302 pin-6)
#define CE1302_PIN                      4               // DS1302 CE    (1302 pin-5)
#define MUX_A0_PIN                      5               // Mux A0
#define MUX_A1_PIN                      6               // Mux A1
#define DAC_RELAY_PIN                   7               // DAC SSR
#define IR_PIN                          8               // IR Sensor input
#define PWM_BACKLIGHT_PIN               9               // pwm-controlled LED backlight
//#define X10_RTS_PIN                   10              // RTS line for C17A - DB9 pin 7
//#define X10_DTR_PIN                   11              // DTR line for C17A - DB9 pin 4
#define AMP_RELAY_PIN                   12              // Amp trigger relay
#define LED13                           13              // arduino standard
#define MOTOR_POT_ROTATE_CW             16              // 3rd analog port used as pwm/digital-16
#define MOTOR_POT_ROTATE_CCW            17              // 4th analog port used as pwm/digital-17

#define AMP_RELAY_MS                    150             // Time in ms to activate amp power relay
#define DAC_OFF_DELAY                   2000            // Delay from Amp off to DAC off

#define BACKLIGHT_OFF                   1
#define BACKLIGHT_AUTODIM               2
#define BACKLIGHT_ON                    3
#define BACKLIGHT_MIN                   (100)
#define BACKLIGHT_MAX                   (255)

// 16x2 LCD
#define LCD_COLS                       16
#define LCD_LINES                       2
#define LCD_USER_CHARS                  8
#define LCD_CHAR_ROWS                   8

// vol control enum value equates
#define VOL_MIN                     (0)
#define VOL_MAX                     (99)
#define VOL_DELAY_SHORT             (20)   // auto-repeat IR delay period

#define POT_MIN                     (0)    // min value we can see on the a/d of the arduino
#define POT_MAX                     (1023) // max value we can see on the a/d of the arduino
#define POT_CHANGE_THRESHOLD        (4)    // it has to be more than this to register as a pot 'twist'
#define POT_REREADS                 (10)   // re-read the pot many times to weed out noise (averaging)

#endif // _CONFIG_H_
