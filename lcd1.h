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

#ifndef LCD1_DRIVERS_H
#define LCD1_DRIVERS_H

/*
 * lcd via i2c
 */

// MCP i2c PE 'user input' pins
#define LCD_MCP_INPUT_PINS_MASK  B01100000     // TODO: pick one for input switch (readable) and one for power-relay (writable)

// IMPORTANT! Wire. must have a begin() before calling init()
class LCDI2C4Bit {
    public:
        LCDI2C4Bit( byte devI2CAddress, byte num_lines, byte lcdwidth, byte backlightPin);
        void init (void);
        void command (byte);
        void write (byte);
        void print (char value[]);
        void clear (void);
        void clear_line (byte);
        void backLight (byte);

        void SendToLCD (byte);
        void WriteLCDByte (byte);
        void SetMCPReg (byte, byte);
        byte GetMCPReg (byte);
        void cursorTo (byte, byte);
        void SetInputKeysMask (byte);
        byte ReadInputKeys (void);
        void load_bignum (void);
        void draw_bignum_at (byte val, byte char_pos);
        void send_string (const char *str, const byte addr);
        //void send_string_P (PROGMEM char *str, byte addr);
        // display and backlight routines
        void turn_display_on (void);
        void turn_display_off (void);
        void fade_backlight_off (void);
        void fade_backlight_to_dim (void);
        void restore_backlight (void);
        void fade_backlight_on (void);
        void handle_backlight_auto (void);
        unsigned long one_second_counter_ts;
        int seconds;
        byte lcd_inactivity_timeout;                    // how long do we keep the 'bright' setting before we fade
        byte backlight_admin;                           // administratively set (enable auto timeout; normal mode)
        byte backlight_currently_on;
        byte backlight_min;                             // = MIN_BL_LEVEL
        byte backlight_max;                             // = DEFAULT_BL_LEVEL
        byte backlight_mode;
    private:
        byte lcd_i2c_address;
        byte myNumLines;
        byte myWidth;
        byte myBacklightPin;
        byte dataPlusMask;
        byte myInputKeysMask;
};

extern LCDI2C4Bit lcd;   //  = LCDI2C4Bit(LCD_MCP_DEV_ADDR, LCD_LINES, LCD_COLS, PWM_BACKLIGHT_PIN);   //0xa7 is the hardware addr of the i2c chip

//extern void display_progmem_string_to_lcd_P (PROGMEM char *p_ptr[], const byte addr);
extern void lcd_print_long_hex (long p_value);
extern void blink_led13 (byte on_off_flag);
extern boolean scan_front_button (void);

// some C++ class refs
//extern decode_results results;
extern LCDI2C4Bit lcd;

#endif // LCD1_DRIVERS_H
