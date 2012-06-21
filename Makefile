#  $Id: Makefile,v 1.25.2.2 2011/05/06 07:42:12 ti Exp $
# LCDduino Controller for Twisted Pear Opus DAC and Mux
# Simeon Walker 2011-2012
#
#  Top-level Makefile: Please use GNU make
#
#  Volu-Master(tm)
#  Arduino controller-based digital volume control and
#  input/output selector
#
#  LCDuino-1/Volu-Master Team: Bryan Levin, Ti Kan
#
#  Project website: http://www.amb.org/audio/lcduino-1/
#  Discussion forum: http://www.amb.org/forum/
#
#  Author: Bryan Levin (Linux-Works Labs)
#  Copyright (c) 2009-2011 Bryan Levin
#  All rights reserved.
#
#
#  LICENSE
#  -------
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#


#
# -----------------------------------------------------------------------
# This section is where users can edit to suit the local setup
#

# The path to your Arduino software top level directory
ifndef ARDUINO_ROOT
ARDUINO_ROOT	= /usr/share/arduino
endif

# The path to your avrdude program executable
ifndef AVRDUDE
AVRDUDE		= avrdude
endif

# The default LCDuino-1 device path (if not set externally via environment)
ifndef port
port		= /dev/ttyUSB0
endif

PROG_NAME			= opusdac
PRE_REL				=
#PRE_REL			= __$(shell date +%F__%H_%M_%S_%P)
ARDUINO_CORE_ROOT	= $(ARDUINO_ROOT)/hardware/arduino/cores/arduino
VARIANTS			= $(ARDUINO_ROOT)/hardware/arduino/variants/standard
MCU					= atmega328p
EXTRA_C_FLAGS		= -w -Wunused-variable -Wextra -Wunused-macros -pedantic
#EXTRA_C_FLAGS		= -Wall -pedantic -Wunused-variable -Wextra
CLI_BUILD_FLAGS		= -DCLI_BUILD -mmcu=$(MCU)  $(EXTRA_C_FLAGS)

#
# Programming support using avrdude. Settings and variables.
#
UPLOAD_RATE			= 57600
F_CPU				= 16000000
AVRDUDE_PROGRAMMER	= arduino
AVRDUDE_PORT		= $(port)
AVRDUDE_FLAGS		= -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -b $(UPLOAD_RATE) -D


#
# Project paths and files
#
MAKEFILE = Makefile
MY_INCLUDE_PATHS = \
	-I$(ARDUINO_CORE_ROOT) \
	-I$(VARIANTS) \
	-I$(ARDUINO_ROOT)/libraries/EEPROM \
	-I$(ARDUINO_ROOT)/libraries/Wire \
	-I$(ARDUINO_ROOT)/libraries/Wire/utility


#
# Arduino "core" library
#
CORE_LIB = arduino_core.a
CORE_C_SRC = \
	$(ARDUINO_CORE_ROOT)/wiring.c \
	$(ARDUINO_CORE_ROOT)/wiring_analog.c \
	$(ARDUINO_CORE_ROOT)/wiring_digital.c \
	$(ARDUINO_CORE_ROOT)/wiring_pulse.c \
	$(ARDUINO_CORE_ROOT)/wiring_shift.c \
	$(ARDUINO_CORE_ROOT)/WInterrupts.c
CORE_CXX_SRC = \
	$(ARDUINO_CORE_ROOT)/main.cpp \
	$(ARDUINO_CORE_ROOT)/WMath.cpp \
	$(ARDUINO_CORE_ROOT)/Print.cpp \
	$(ARDUINO_CORE_ROOT)/HardwareSerial.cpp
CORE_OBJS = \
	main.o \
	wiring.o \
	wiring_analog.o \
	wiring_digital.o \
	wiring_pulse.o \
	wiring_shift.o \
	WInterrupts.o \
	WMath.o \
	Print.o \
	HardwareSerial.o

#
# not called "core" but still part of the Arduino lib system
#
SYS_OBJS = \
	EEPROM.cpp.o \
	Wire.cpp.o \
	twi.c.o

LIB_EEPROM		= $(ARDUINO_ROOT)/libraries/EEPROM
LIB_WIRE		= $(ARDUINO_ROOT)/libraries/Wire
LIB_WIRE_UTIL	= $(ARDUINO_ROOT)/libraries/Wire/utility

#
# Volu-Master objs
#
MY_OBJS = \
	$(PROG_NAME).cpp.o \
	motorpot.cpp.o \
	lcd1.cpp.o \
	irremote.cpp.o \
	rtc.cpp.o \
	dac_hw.cpp.o \
	util.cpp.o

#
# Volu-Master sources
#
MY_SRCS = \
	$(PROG_NAME).cpp \
	motorpot.cpp \
	lcd1.cpp \
	irremote.cpp \
	rtc.cpp \
	dac_hw.cpp \
	util.cpp

#
# Volu-Master headers
#
MY_INCLUDE_FILES = \
	config.h \
	motorpot.h \
	volcontrol_defs.h \
	lcd1.h \
	irremote.h \
	rtc.h \
	dac_hw.h \
	util.h

#
# Subdirectories that make needs to traverse into
#
SUBDIRS=

.SUFFIXES: .o .cpp .hex .elf .eep .lss .sym

COMMON_FLAGS	= $(CLI_BUILD_FLAGS) -c -Os -DF_CPU=$(F_CPU) -DARDUINO=22  -ffunction-sections -fdata-sections
CXX				= avr-g++ $(COMMON_FLAGS) -fno-exceptions
CC				= avr-gcc $(COMMON_FLAGS)

CXX_DEPEND		= avr-g++ $(CLI_BUILD_FLAGS)
CC_DEPEND		= avr-gcc $(CLI_BUILD_FLAGS)

TARG_ELF		= $(PROG_NAME).cpp.elf
TARG_HEX		= $(PROG_NAME).cpp.hex

#
# Our default 'make all' target
#
all:	$(TARG_HEX) $(CORE_LIB)
	@for flag in ${MAKEFLAGS} ''; do \
	case "$$flag" in *=*) ;; --*) ;; *[ik]*) set +e;; esac; done; \
	for i in $(SUBDIRS) ;\
	do \
		echo "making" all "in $$i..."; \
		$(MAKE) -C $$i $(MFLAGS) all; \
	done

#
# Generate the "core" library
#
$(CORE_LIB): $(CORE_OBJS)
	@for i in $(CORE_OBJS); do echo avr-ar rcs $(CORE_LIB) $$i; avr-ar rcs $(CORE_LIB) $$i; done

#
# C core components
#
pins_arduino.o:		$(ARDUINO_CORE_ROOT)/pins_arduino.c
	$(CC) -I$(ARDUINO_CORE_ROOT) $< -o $@

wiring.o:			$(ARDUINO_CORE_ROOT)/wiring.c
	$(CC) -I$(ARDUINO_CORE_ROOT) -I$(VARIANTS) $< -o $@

wiring_analog.o:	$(ARDUINO_CORE_ROOT)/wiring_analog.c
	$(CC) -I$(ARDUINO_CORE_ROOT) -I$(VARIANTS) $< -o $@

wiring_digital.o:	$(ARDUINO_CORE_ROOT)/wiring_digital.c
	$(CXX) -I$(ARDUINO_CORE_ROOT) -I$(VARIANTS) $< -o $@

wiring_pulse.o:		$(ARDUINO_CORE_ROOT)/wiring_pulse.c
	$(CXX) -I$(ARDUINO_CORE_ROOT) -I$(VARIANTS) $< -o $@

wiring_shift.o:		$(ARDUINO_CORE_ROOT)/wiring_shift.c
	$(CXX) -I$(ARDUINO_CORE_ROOT) -I$(VARIANTS) $< -o $@

WInterrupts.o:		$(ARDUINO_CORE_ROOT)/WInterrupts.c
	$(CC) -I$(ARDUINO_CORE_ROOT) -I$(VARIANTS) $< -o $@

#
# C++ core components
#
HardwareSerial.o:	$(ARDUINO_CORE_ROOT)/HardwareSerial.cpp
	$(CXX) -I$(ARDUINO_CORE_ROOT) -I$(VARIANTS) $< -o $@

WMath.o:			$(ARDUINO_CORE_ROOT)/WMath.cpp
	$(CXX) -I$(ARDUINO_CORE_ROOT) -I$(VARIANTS) $< -o $@

Print.o:			$(ARDUINO_CORE_ROOT)/Print.cpp
	$(CXX) -I$(ARDUINO_CORE_ROOT) -I$(VARIANTS) $< -o $@

main.o:				$(ARDUINO_CORE_ROOT)/main.cpp
	$(CC) -I$(ARDUINO_CORE_ROOT) -I$(VARIANTS) $< -o $@

#
# Arduino libs
#

EEPROM.cpp.o:		$(LIB_EEPROM)/EEPROM.cpp
	$(CXX) $(MY_INCLUDE_PATHS) $< -o $@

Wire.cpp.o:			$(LIB_WIRE)/Wire.cpp
	$(CXX) $(MY_INCLUDE_PATHS) $< -o $@

# NOTE! this is NOT c++ !  build this with regular C
twi.c.o:			$(LIB_WIRE_UTIL)/twi.c
	$(CC) $(MY_INCLUDE_PATHS) $< -o $@

#
# The "binary" (fully linked runnable binary for Atmel)
#
$(TARG_ELF): $(MY_OBJS) $(SYS_OBJS) $(CORE_LIB)
	avr-gcc -Os -Wl,--gc-sections -mmcu=$(MCU) -o $(TARG_ELF) $(MY_OBJS) $(SYS_OBJS) $(CORE_LIB) -L. -lm

#
# The ASCII form in 'intel hex' suitable for direct upload
#
$(TARG_HEX):	$(TARG_ELF)
	avr-objcopy -O ihex -R .eeprom $(TARG_ELF) $(TARG_HEX)

#
# Normally $(PROG_NAME).ino, the main program app
#
$(PROG_NAME).cpp:	$(PROG_NAME).ino
	@echo "  ---> Copying $(PROG_NAME).ino to $(PROG_NAME).cpp ..."
	cp -f $(PROG_NAME).ino $(PROG_NAME).cpp

#
# The rest of the Volu-Master source build rules
#
$(PROG_NAME).cpp.o:	$(PROG_NAME).cpp $(MY_INCLUDE_FILES)
	$(CXX) $(MY_INCLUDE_PATHS) -c $(PROG_NAME).cpp -o $(PROG_NAME).cpp.o

motorpot.cpp.o:	motorpot.cpp $(MY_INCLUDE_FILES)
	$(CXX) $(MY_INCLUDE_PATHS) $< -o $@

lcd1.cpp.o:	lcd1.cpp $(MY_INCLUDE_FILES)
	$(CXX) $(MY_INCLUDE_PATHS) $< -o $@

irremote.cpp.o: irremote.cpp $(MY_INCLUDE_FILES)
	$(CXX) $(MY_INCLUDE_PATHS) $< -o $@

#irlearn.cpp.o: irlearn.cpp $(MY_INCLUDE_FILES)
#	$(CXX) $(MY_INCLUDE_PATHS) $< -o $@

rtc.cpp.o: rtc.cpp $(MY_INCLUDE_FILES)
	$(CXX) $(MY_INCLUDE_PATHS) $< -o $@

dac_hw.cpp.o: dac_hw.cpp $(MY_INCLUDE_FILES)
	$(CXX) $(MY_INCLUDE_PATHS) $< -o $@

util.cpp.o: util.cpp $(MY_INCLUDE_FILES)
	$(CXX) $(MY_INCLUDE_PATHS) $< -o $@

upload: $(TARG_HEX)
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:w:$(TARG_HEX):i

#
# Create extended listing file from ELF output file
#
list:
	avr-objdump -h -S $(PROG_NAME).cpp.elf > $(PROG_NAME).lss
	avr-nm -n  $(PROG_NAME).cpp.elf > $(PROG_NAME).sym

#
# Install files
#
install:
	for i in $(SUBDIRS) ;\
	do \
		echo "making" install "in $$i..."; \
		$(MAKE) -C $$i $(MFLAGS) install; \
	done

#
# Clean up the clutter
#
clean:
	rm -f *.zip *.o *.elf *.hex $(PROG_NAME).cpp *.sym *.lss *~* $(CORE_LIB)
	@for flag in ${MAKEFLAGS} ''; do \
	case "$$flag" in *=*) ;; --*) ;; *[ik]*) set +e;; esac; done; \
	for i in $(SUBDIRS) ;\
	do \
		echo "making" clean "in $$i..."; \
		$(MAKE) -C $$i $(MFLAGS) clean; \
	done

#
# Generate dependencies
#
depend:
	if grep '^# DO NOT DELETE' $(MAKEFILE) > /dev/null; \
	then \
		sed -e '/^# DO NOT DELETE/,$$d' $(MAKEFILE) > \
			$(MAKEFILE).$$$$ && \
		mv $(MAKEFILE).$$$$ $(MAKEFILE); \
	fi
	echo '# DO NOT DELETE THIS LINE -- make depend depends on it.' \
		>> $(MAKEFILE); \
	$(CXX_DEPEND) -M $(CLI_BUILD_FLAGS)  $(MY_INCLUDE_PATHS) $(MY_INCLUDE_FILES) $(MY_SRCS) >> $(MAKEFILE)
	@for flag in ${MAKEFLAGS} ''; do \
	case "$$flag" in *=*) ;; --*) ;; *[ik]*) set +e;; esac; done; \
	for i in $(SUBDIRS) ;\
	do \
		echo "making" depend "in $$i..."; \
		$(MAKE) -C $$i $(MFLAGS) depend; \
	done

.PHONY: all build elf hex eep lss sym program coff extcoff clean depend sizebefore sizeafter
