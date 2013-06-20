/*
 * LCDduino Controller for Twisted Pear Opus DAC and Mux
 * Simeon Walker 2011-2012
 *
 * IRremote
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 */

#ifndef IRremote_h
#define IRremote_h

extern uint8_t lcd_in_use_flag;    // mutex flag: we are not allowed to 'do anything' if the lcd is currently in use
extern unsigned long key;                               // IR key received
extern unsigned long get_IR_key (void);

class decode_results {                                  // Results returned from the decoder
    public:
        char decode_type;                               // NEC, SONY, RC5, RC6, UNKNOWN
        unsigned long value;                            // Decoded value
        int bits;                                       // Number of bits in decoded value
        volatile unsigned int *rawbuf;                  // Raw intervals in .5 us ticks
        int rawlen;                                     // Number of records in rawbuf.
};

class IRrecv {                                          // main class for receiving IR
    public:
        IRrecv (int recvpin);
        int decode (decode_results *results);
        void enableIRIn (void);
        void resume (void);
    private:                                            // These are called by decode
        uint8_t getRClevel (decode_results *results, int *offset, int *used, int t1);
        long decodeSony (decode_results *results);
};

#endif
