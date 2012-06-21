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

#include <Wire.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "config.h"
#include "rtc.h"  // our own includes

void send1302cmd (unsigned char cmd1, unsigned char cmd2);
unsigned char get1302data (unsigned char cmd);
void ds1302_init (void);
unsigned char shiftin (void);

DS1302::DS1302 (void) {                                 // constructor (does nothing; init() does all the work)
    return;
}

void DS1302::init (void) {                              // init the DS1302 realtime clock chip
    pinMode(CE1302_PIN,  OUTPUT);
    pinMode(DAT1302_PIN, OUTPUT);
    pinMode(CLK1302_PIN, OUTPUT);
    //  This routine is, most likely, called with the DS1302 running on the supercap or battery backup,
    // so it is important to preserve its time values.
    send1302cmd(WriteCtrl, 0);                          // Clear write protect
    Secs = get1302data(ReadSecs);                       // Clear CH bit, preserving secs reg
    Secs &= B01111111;
    send1302cmd(WriteSecs, Secs);
    send1302cmd(WriteTrickle, TrickleSet);              // begin charging the supercap
}

void DS1302::GetTime (void) {                           // retrieve time from DS1302 chip
    Secs = get1302data(ReadSecs);
    Mins = get1302data(ReadMins);
    Hrs  = get1302data(ReadHrs);
}

void DS1302::SetTime (void) {                           // write time to DS1302 chip
    send1302cmd(WriteHrs,  Hrs);
    send1302cmd(WriteMins, Mins);
    send1302cmd(WriteSecs, Secs);
}

void send1302cmd (unsigned char cmd1, unsigned char cmd2)  {
    digitalWrite(CE1302_PIN, 1);                        // Set CE1302 high
    delayMicroseconds(2);

    shiftOut(DAT1302_PIN, CLK1302_PIN, LSBFIRST, cmd1);
    delayMicroseconds(2);                               //  This delay might not be needed

    shiftOut(DAT1302_PIN, CLK1302_PIN, LSBFIRST, cmd2);
    digitalWrite(CE1302_PIN, 0);                        // Set CE1302 low
}

unsigned char get1302data (unsigned char cmd) {
    unsigned char dat;
    digitalWrite(CE1302_PIN, 1);                        // CE1302 high
    shiftOut(DAT1302_PIN, CLK1302_PIN, LSBFIRST, cmd);
    dat = shiftin();
    digitalWrite(CE1302_PIN, 0);                        //  CE1302 low
    return (dat);
}

//  Shifts in a byte from the DS1302.
//  This routine is called immediately after a shiftout.  The first bit is
//  present on DAT1302 at call, so only 7 additional clock pulses are
//  required.
//  Restores DAT1302 to OUTPUT prior to return.

unsigned char shiftin (void) {
    unsigned char dat = 0;
    int i = 0;

    pinMode(DAT1302_PIN, INPUT);       // Set DAT1302 as input

    for (i=0; i < 7; i++) {
        if (digitalRead(DAT1302_PIN) == 1) {
            dat |= B10000000;              // we found a 1-bit, so add it to our collection ;)
        }
        dat >>= 1;  // shift over so that we can logical-OR the next 1-bit (if any)
        digitalWrite(CLK1302_PIN, 1);    // Strobe in next data bit using CLK1302
        delay(1);
        digitalWrite(CLK1302_PIN, 0);
        delay(1);
    }
    pinMode(DAT1302_PIN, OUTPUT);      // Restore DAT1302 as output
    return dat;
}
