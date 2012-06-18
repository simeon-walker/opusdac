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

#define CLKFUDGE                        5		// fudge factor for clock interrupt overhead
#define CLK                           256		// max value for clock (timer 2)
#define PRESCALE                        8		// timer2 clock prescale
#define SYSCLOCK                 16000000		// main Arduino clock
#define CLKSPERUSEC                     (SYSCLOCK / PRESCALE / 1000000)   // timer clocks per microsecond

#define ERR                             0
#define DECODED                         1

#define RAWBUF                         76		// Length of raw duration buffer

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit)                   (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit)                   (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// clock timer reset value
#define INIT_TIMER_COUNT2               (CLK - USECPERTICK*CLKSPERUSEC + CLKFUDGE)
#define RESET_TIMER2                    (TCNT2 = INIT_TIMER_COUNT2)

// pulse parameters in usec
#define SONY_HDR_MARK                2400
#define SONY_HDR_SPACE                600
#define SONY_ONE_MARK                1200
#define SONY_ZERO_MARK                600
#define SONY_RPT_LENGTH             45000

#ifdef USE_FP_MATH
#define TOLERANCE                      25  // percent tolerance in measurements
#define LTOL                            (1.0 - TOLERANCE/100.)
#define UTOL                            (1.0 + TOLERANCE/100.)
#else
#define LTOL                          (3)  // (1.0 - TOLERANCE/100.)
#define UTOL                          (5)  // (1.0 + TOLERANCE/100.)
#endif  // FP_PATH

// Marks tend to be 100us too long, and spaces 100us too short
// when received. (sensor lag?)
#define MARK_EXCESS                   100

#define _GAP                         5000 // Minimum map between transmissions
#define GAP_TICKS                       (_GAP/USECPERTICK)

#ifdef USE_FP_MATH
#define TICKS_LOW(us)                   (int) (((us) * (LTOL) / USECPERTICK))
#define TICKS_HIGH(us)                  (int) (((us) * (UTOL) / USECPERTICK + 1))
#else
#define TICKS_LOW(us)                   (int) (((us) * (LTOL) / USECPERTICK/4))
#define TICKS_HIGH(us)                  (int) (((us) * (UTOL) / USECPERTICK/4 + 1))
#endif

#ifndef DEBUG
#define MATCH(measured_ticks, desired_us)       ((measured_ticks) >= TICKS_LOW(desired_us) && (measured_ticks) <= TICKS_HIGH(desired_us))
#define MATCH_MARK(measured_ticks, desired_us)  MATCH((measured_ticks), (desired_us) + MARK_EXCESS)
#define MATCH_SPACE(measured_ticks, desired_us) MATCH((measured_ticks), (desired_us) - MARK_EXCESS)
// Debugging versions are in IRremote.cpp
#endif

// receiver states
#define STATE_IDLE                      2
#define STATE_MARK                      3
#define STATE_SPACE                     4
#define STATE_STOP                      5

// information for the interrupt handler
typedef struct {
    uint8_t recvpin;                                    // pin for IR data from detector
    uint8_t rcvstate;                                   // state machine
    uint8_t blinkflag;                                  // TRUE to enable blinking of pin 13 on IR processing
    unsigned int timer;                                 // state timer, counts 50uS ticks.
    unsigned int rawbuf[RAWBUF];                        // raw data
    uint8_t rawlen;                                     // counter of entries in rawbuf
} irparams_t;

// Defined in IRremote.cpp
extern volatile irparams_t irparams;

// IR detector output is active low
#define MARK                            0
#define SPACE                           1
#define TOPBIT                 0x80000000
#define SONY_BITS                      12

#define SONY                            2               // Values for decode_type
#define UNKNOWN                       (-1)

#define USECPERTICK                    50               // microseconds per clock interrupt tick

/*
 * IRremote.h
 */
//extern unsigned long get_IR_key (void);

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
