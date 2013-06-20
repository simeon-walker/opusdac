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


// conversion routines (bcd, ascii, etc)
void bcd2ascii (const byte val, byte *ms, byte *ls) {
    *ms = (val >> 4)   + '0';
    *ls = (val & 0x0f) + '0';
}

void bin2ascii (const byte val, byte *ms, byte *ls) {
    *ms = val / 10 + '0';
    *ls = val % 10 + '0';
}

// BCD to binary
byte bcd2bin (const byte val) {
    return (((val >> 4) * 10) + (val & 0x0f));
}

// binary to BCD
byte bin2bcd (const byte val) {
    return ((val / 10 * 16) + (val % 10));
}

// converts the 0..255 byte value 'b' into a printable ascii string '01101...'
// write string results in our SUPPLIED buffer and term with null byte.
// the result field should be 'num_bits' long in ascii chars.

//void dec2bin (byte b, byte num_bits, char string_buf[]) {
//    int i;
//    // for a normal 8-bit byte, num_bits s/b 8 (and so the high bit is bit7)
//    for (i=0; i<num_bits; i++) {
//        if (b & 0x01) {
//            string_buf[num_bits-i-1] = '1';
//        } else {
//            string_buf[num_bits-i-1] = '0';
//        }
//        // shift our whole byte to the right.  0's are auto-inserted to the left.
//        // we want to do all our compares on the bit-0 position so right-shifting
//        // is how we get access to each of the 8 bits.
//        b >>= 1;
//    }
//    string_buf[num_bits] = '\0';
//}

// converts the string '0101...' to a byte value 0..255
// takes string from our global buffer, string_buf[]
// returns value directly.

//byte bin2dec (char *str_buf, int len) {
//    byte i;
//    byte single_bit;
//    byte sum;
//    sum = 0;
//    for (i=0; i < len; i++) {
//        // get the next char from the string array
//        single_bit = str_buf[i];
//        // if its a '1' then add in the right power of 2
//        if (single_bit == '1') {
//            sum += (1 << (len - 1 - i));
//        }
//    }
//    return sum;
//}

void hex2ascii (const byte val, byte *ms, byte *ls) {
    static char hex_buf[8];

    sprintf(hex_buf, "%02x ", val);
    *ms = hex_buf[0];
    *ls = hex_buf[1];
}

// this fixes a bug in the arduino supplied map function.  you would almost never reach the very 'top' of the range
//  with the old functions.  this would make it very hard to turn the volume pot all the way up, for example.
long l_map (long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min + 1) / (in_max - in_min + 1) + out_min;
}

void EEwrite_long (int p_address, unsigned long p_value) {
    byte byte1 = ((p_value >>  0) & 0xFF);
    byte byte2 = ((p_value >>  8) & 0xFF);
    byte byte3 = ((p_value >> 16) & 0xFF);
    byte byte4 = ((p_value >> 24) & 0xFF);

    EEPROM.write(p_address,     byte1);
    EEPROM.write(p_address + 1, byte2);
    EEPROM.write(p_address + 2, byte3);
    EEPROM.write(p_address + 3, byte4);
}

unsigned long EEread_long (int p_address) {
    byte byte1 = EEPROM.read(p_address);
    byte byte2 = EEPROM.read(p_address + 1);
    byte byte3 = EEPROM.read(p_address + 2);
    byte byte4 = EEPROM.read(p_address + 3);

    unsigned long firstTwoBytes = ((byte1 << 0) & 0xFF) + ((byte2 << 8) & 0xFF00);
    unsigned long secondTwoBytes = (((byte3 << 0) & 0xFF) + ((byte4 << 8) & 0xFF00));
    secondTwoBytes *= 65536; // multiply by 2 to power 16 - bit shift 24 to the left

    return (firstTwoBytes + secondTwoBytes);
}

/*
 * memory free (checkers)
 */

#ifdef USE_MEM_CHECKER

extern unsigned int __bss_end;
extern unsigned int __heap_start;
extern void *__brkval;

uint8_t *heapptr, *stackptr;

unsigned int freeMemory (void) {
    unsigned int free_memory;
    if ((unsigned int)__brkval == 0) {
        free_memory = ((unsigned int)&free_memory) - ((unsigned int)&__bss_end);
    } else {
        free_memory = ((unsigned int)&free_memory) - ((unsigned int)__brkval);
    }
    return free_memory;
}

#if 0
int availableMemory (void) {
    //int size = 1024; // Use 2048 with ATmega328
    int size=1; // = 2048; // Use 2048 with ATmega328
    char *buf;
    while ((buf = (char *)malloc(size)) != NULL) {
        free(buf);  // now that we know we can get it, we don't want it anymore ;)
        if (++size >= 2048) break;  // upper limit, even though its absurd
    }
    return size;
}
#endif

#if 0
int availableMemory (void) {
    stackptr = (uint8_t *)malloc(4);                    // use stackptr temporarily
    heapptr = stackptr;                                 // save value of heap pointer
    free(stackptr);                                     // free up the memory again (sets stackptr to 0)
    stackptr =  (uint8_t *)(SP);                        // save value of stack pointer
    return 1;
}
#endif

#endif // USE_MEM_CHECKER

