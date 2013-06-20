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

#ifndef _UTIL_H_
#define _UTIL_H_

extern void bcd2ascii (const byte val, byte *ms, byte *ls);
extern void bin2ascii (const byte val, byte *ms, byte *ls);

extern byte bcd2bin (const byte val);
extern byte bin2bcd (const byte val);

//extern void dec2bin (byte b, byte num_bits, char out_buf[]);
//extern byte bin2dec (char *str_buf, int len);

extern void hex2ascii (const byte val, byte *ms, byte *ls);

extern void EEwrite (int addr_val, byte data);
extern void EEwrite_long (int p_address, unsigned long p_value);

extern unsigned long EEread_long (int p_address);
extern long l_map (long x, long in_min, long in_max, long out_min, long out_max);

#endif

/*
 * memory checkers
 */

#ifdef USE_MEM_CHECKER

#ifdef __cplusplus
extern "C" {
#endif

// externs
unsigned int freeMemory (void);
unsigned int availableMemory (void);

extern uint8_t *heapptr, *stackptr;

#ifdef  __cplusplus
}
#endif

#endif // USE_MEM_CHECKER

