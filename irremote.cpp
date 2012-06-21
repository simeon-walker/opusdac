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
 *
 *
 * IRremote (original version)
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 */

#include "Arduino.h"
#include <Wire.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "irremote.h"

volatile irparams_t irparams;

IRrecv::IRrecv (int recvpin) {
    irparams.recvpin = recvpin;
    irparams.blinkflag = 0;                             // turn it off by default
}

void IRrecv::enableIRIn (void) {                        // initialization
    if (lcd_in_use_flag) return;                        // respect the lcd mutex
    // setup pulse clock timer interrupt
    TCCR2A = 0;                                         // normal mode
    //Prescale /8 (16M/8 = 0.5 microseconds per tick)
    // Therefore, the timer interval can range from 0.5 to 128 microseconds
    // depending on the reset value (255 to 0)
    cbi(TCCR2B,CS22);
    sbi(TCCR2B,CS21);
    cbi(TCCR2B,CS20);
    sbi(TIMSK2,TOIE2);                                  //Timer2 Overflow Interrupt Enable
    RESET_TIMER2;
    sei();                                              // enable interrupts
    irparams.rcvstate = STATE_IDLE;                     // initialize state machine variables
    irparams.rawlen = 0;
    pinMode(irparams.recvpin, INPUT);                   // set pin modes
}

// TIMER2 interrupt code to collect raw data.
// Widths of alternating SPACE, MARK are recorded in rawbuf.
// Recorded in ticks of 50 microseconds.
// rawlen counts the number of entries recorded so far.
// First entry is the SPACE between transmissions.
// As soon as a SPACE gets long, ready is set, state switches to IDLE, timing of SPACE continues.
// As soon as first MARK arrives, gap width is recorded, ready is cleared, and new logging starts

ISR (TIMER2_OVF_vect) {
    if (lcd_in_use_flag) return;                        // respect the lcd mutex
    RESET_TIMER2;
    uint8_t irdata = (uint8_t)digitalRead(irparams.recvpin);

    irparams.timer++;                                   // One more 50us tick
    if (irparams.rawlen >= RAWBUF) {
        irparams.rcvstate = STATE_STOP;                 // Buffer overflow
    }

    switch (irparams.rcvstate) {
    case STATE_IDLE:                                    // In the middle of a gap
        if (irdata == MARK) {
            if (irparams.timer < GAP_TICKS) {
                irparams.timer = 0;                     // Not big enough to be a gap.
            } else {                                    // gap just ended, record duration and start recording transmission
                irparams.rawlen = 0;
                irparams.rawbuf[irparams.rawlen++] = irparams.timer;
                irparams.timer = 0;
                irparams.rcvstate = STATE_MARK;
            }
        }
        break;

    case STATE_MARK:                                    // timing MARK
        if (irdata == SPACE) {                          // MARK ended, record time
            irparams.rawbuf[irparams.rawlen++] = irparams.timer;
            irparams.timer = 0;
            irparams.rcvstate = STATE_SPACE;
        }
        break;

    case STATE_SPACE:                                   // timing SPACE
        if (irdata == MARK) {                           // SPACE just ended, record it
            irparams.rawbuf[irparams.rawlen++] = irparams.timer;
            irparams.timer = 0;
            irparams.rcvstate = STATE_MARK;
        } else {                                        // SPACE
            if (irparams.timer > GAP_TICKS) {
                // big SPACE, indicates gap between codes
                // Mark current code as ready for processing
                // Switch to STOP
                // Don't reset timer; keep counting space width
                irparams.rcvstate = STATE_STOP;
            }
        }
        break;

    case STATE_STOP:                                    // waiting, measuring gap
        if (irdata == MARK) {                           // reset gap timer
            irparams.timer = 0;
        }
        break;
    }
}

void IRrecv::resume (void) {
    if (lcd_in_use_flag) return;                        // respect the lcd mutex
    RESET_TIMER2;
    // Disable the Timer2 Interrupt (which is used for receiving IR)
    //TIMSK2 &= ~_BV(TOIE2); //Timer2 Overflow Interrupt
    irparams.rcvstate = STATE_IDLE;
    irparams.rawlen = 0;
    irparams.timer = 0;
    //results.value = 0;
}

// Decodes the received IR message
// Returns 0 if no data ready, 1 if data ready.
// Results of decoding are stored in results
int IRrecv::decode (decode_results *results) {
    results->rawbuf = irparams.rawbuf;
    results->rawlen = irparams.rawlen;

    if (irparams.rcvstate != STATE_STOP) {
        return ERR;
    }
    if (decodeSony(results)) {
        return DECODED;
    }
    results->decode_type = UNKNOWN;
    results->bits = 0;
    results->value = 0;
    return DECODED;
}

long IRrecv::decodeSony (decode_results *results) {
    long data = 0;
    if (irparams.rawlen < 2 * SONY_BITS + 2) {
        return ERR;
    }
    int offset = 1; // Skip first space
    // Initial mark
    if (!MATCH_MARK(results->rawbuf[offset], SONY_HDR_MARK)) {
        return ERR;
    }
    offset++;
    while (offset + 1 < irparams.rawlen) {
        if (!MATCH_SPACE(results->rawbuf[offset], SONY_HDR_SPACE)) {
            break;
        }
        offset++;
        if (MATCH_MARK(results->rawbuf[offset], SONY_ONE_MARK)) {
            data = (data << 1) | 1;
        } else if (MATCH_MARK(results->rawbuf[offset], SONY_ZERO_MARK)) {
            data <<= 1;
        } else {
            return ERR;
        }
        offset++;
    }
    // Success
    //results->bits = (offset - 1) / 2;   // orig code
    results->bits = (offset - 1) >> 1;    // new code
    if (results->bits < 12) {
        results->bits = 0;
        return ERR;
    }
    results->value = data;
    results->decode_type = SONY;
    return DECODED;
}

/************************************
 * IR support routines (app level)
 ************************************/

extern IRrecv irrecv;                                   //(IR_PIN);
decode_results results;
unsigned long key;                                      // IR key received

unsigned long get_IR_key (void) {
    if (irrecv.decode(&results)) {
        delay(20);
        irrecv.resume();                                //'start' to receive the next value
        return results.value;                           //my_result;
    } else {
        return 0;                                       // no key pressed
    }
}

