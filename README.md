Opus DAC
========

Arduino based controller for Twisted Pear Opus DAC and S/PDIF Mux.

This code is designed to run on the LCDuino, an Arduino based controller from
Linux Works and AMB Labs. Originally intended for the AMB Labs components I
have used it to control the Twisted Pear WM8741 based DAC and CS8416
based multiplexer.

* LinuxWorks Labs: http://www.netstuff.org/audio/
* AMB Laboratories: http://www.amb.org/audio/
* Twisted Pear Audio: http://www.twistedpearaudio.com/

Source Code
-----------

I started with the Volu-Master source designed for the LCDuino in an AMB system.
I modified it to use spare outputs on the Arduino to control the Mux, the
existing i2c bus is used to program the DAC.

Over time the code has changed greatly such that it's no longer suitable for
the AMB system. A lot of the flexibility and configurability has gone but in
return I have a much smaller code base that I can understand and fit in my head.

Some key parts of the original code remain:
* The IR receiver and learning code is basically the same but I restrict myself
to just the Sony protocol.
* The Motor-pot code is re-arranged but the algorithm is the same.
* Programming of the LCD via the i2 port expander hasn't changed too much.
* The realtime clock code hasn't changed much.
