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

#define WM8741_I2C_ADDR     0x1A
// Note: reg addr values are shifted one bit right since LSB
// is actually data MSB (bit 8), but this is always zero
#define WM8741_REG_DACLLSB  0x00    // 0 attenuation reg
#define WM8741_REG_DACLMSB  0x02    // 1 attenuation reg
#define WM8741_REG_DACRLSB  0x04    // 2 attenuation reg
#define WM8741_REG_DACRMSB  0x06    // 3 attenuation reg
#define WM8741_REG_VOLCTRL  0x08    // 4 vol control reg
#define WM8741_REG_FORMAT   0x0A    // 5 format control reg
#define WM8741_REG_FILTER   0x0C    // 6 filter control reg
#define WM8741_REG_MODE1    0x0E    // 7 mode control 1 reg
#define WM8741_REG_MODE2    0x10    // 8 mode control 2 reg
#define WM8741_REG_RESET    0x12    // 9 software reset reg

#define WM8741_BIT_VOLUPD   0x20	  // logical OR with volume to apply update

#define WM8741_VOLDEFS      0x05    // defaults for vol control reg
#define WM8741_VOLMUTE      0x0D    // defaults + mute for vol control reg

#define WM8741_FILTER1      0x00    // PCM Filter response 1
#define WM8741_FILTER2      0x01    // PCM Filter response 2
#define WM8741_FILTER3      0x02    // PCM Filter response 3
#define WM8741_FILTER4      0x03    // PCM Filter response 4
#define WM8741_FILTER5      0x04    // PCM Filter response 5

#define WM8741_OSRHIGH      0x40    // defaults + OSR High for mode control 1 reg
#define WM8741_OSRMED       0x20    // defaults + OSR Medium for mode control 1 reg
#define WM8741_OSRLOW       0x00    // defaults + OSR Low for mode control 1 reg

#define WM8741_RESET        0x00    // value for reset reg

extern void dac_init (void);
extern void dac_set_volume (byte vol);
extern void dac_mute (boolean mute_flag);
extern void dac_select_filter (byte filter);
extern void mux_select_input (byte new_in_sel);
