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

/*
 * ds1302 stuff
 */

#ifndef DS1302_H
#define DS1302_H

#define SUPERCAP_INSTALLED 1

//  DS1302 connections
//#define CE1302_PIN                      4  // CE    (1302 pin-5)
//#define DAT1302_PIN                     3  // i/o   (1302 pin-6)
//#define CLK1302_PIN                     2  // clock (1302 pin-7)

/*
 * start of RTC (realtime clock) code
 * originally written by tom for his all-in-1 camera intervalometer
 */

//  DS1302 Opcodes

#define WriteCtrl               B10001110
#define ReadSecs                B10000001
#define WriteSecs               B10000000
#define ReadMins                B10000011
#define WriteMins               B10000010
#define ReadHrs                 B10000101
#define WriteHrs                B10000100
#define WriteTrickle            B10010000
#define ReadDay                 B10001011
#define WriteDay                B10001010
#define ReadDate                B10000111
#define WriteDate               B10000110
#define ReadMonth               B10001001
#define WriteMonth              B10001000
#define ReadYear                B10001101
#define WriteYear               B10001100

#ifdef SUPERCAP_INSTALLED

//  Sets DS1302 trickle charger for supercap backup
//  1 diode, 2k series resistance
#define TrickleSet              B10100101

#else
//  Sets DS1302 for no trickle charger output.  Use this for battery backup.
#define TrickleSet              B11111111

#endif  // supercap

class DS1302 {
    public:
        // functs
        DS1302 (void);
        void init (void);
        void GetTime (void);
        void SetTime (void);
        // vars
        unsigned char Hrs;         //  Hrs has 12/24 and AM/PM bits stripped
        unsigned char Mins;
        unsigned char Secs;        //  DS1302's BCD values
};

#endif // DS1302_H

extern DS1302 rtc;
