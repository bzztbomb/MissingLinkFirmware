#/*
# * MissingLink Firmware
# * Copyright 2011 Jabrudian Industries LLC
# *
# * MissingLink Firmware is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * MissingLink Firmware is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with MissingLink Firmware.  If not, see <http://www.gnu.org/licenses/>.
# *
# */
MCU=atmega328p
CC=avr-gcc
CPP=avr-g++
OBJCOPY=avr-objcopy
# optimize for size:
INCLUDES=-I../Libraries/WiShield -I../Libraries/OSC -I../Libraries/avrmidi -I../Libraries/arduino -I../Libraries/twi
CPPFLAGS=-c -g -Os -Wall -fno-exceptions -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -funsigned-bitfields -mmcu=$(MCU) -DF_CPU=16000000UL -DARDUINO=21 $(INCLUDES)
CFLAGS=-c -g -Os -Wall -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -funsigned-bitfields -mmcu=$(MCU) -DF_CPU=16000000UL -DARDUINO=21 $(INCLUDES)
LDFLAGS=-Os -Wl,--gc-sections -Wl,-Map,main.map -lm -mmcu=$(MCU)
PORT=/dev/ttyUSB0

SRC = \
	MissingLink.cpp 	\
	HandleMidi.cpp 	\
	MLSerial.cpp 		\
	ML_Config.c 		\
	Output.cpp 			\
	Parse.cpp			\
	udpapp.cpp			\
	../Libraries/OSC/OSCMessage.cpp \
	../Libraries/OSC/OSCEncoder.cpp \
	../Libraries/OSC/OSCDecoder.cpp \
	../Libraries/WiShield/WiShield.cpp \
	../Libraries/WiShield/clock-arch.c \
	../Libraries/WiShield/g2100.c \
	../Libraries/WiShield/memb.c \
	../Libraries/WiShield/network.c \
	../Libraries/WiShield/stack.c \
	../Libraries/WiShield/timer.c \
	../Libraries/WiShield/uip-fw.c \
	../Libraries/WiShield/uip-split.c \
	../Libraries/WiShield/uip.c \
	../Libraries/WiShield/uip_arp.c \
	../Libraries/WiShield/uiplib.c \
	../Libraries/arduino/WInterrupts.c \
	../Libraries/arduino/wiring.c \
	../Libraries/twi/twi.c \
	../Libraries/avrmidi/bytequeue.c \
	../Libraries/avrmidi/interrupt_setting.c \
	../Libraries/avrmidi/midi.c \
	../Libraries/avrmidi/midi_device.c

OBJC = ${SRC:.c=.o}
OBJ  = ${OBJC:.cpp=.o}

.cpp.o:
	$(CPP) -c $(CPPFLAGS) -o $*.o $<

.c.o:
	echo CC $<
	$(CC) -c $(CFLAGS) -o $*.o $<

MissingLink.out : $(OBJ)
	$(CC) $(LDFLAGS) -o MissingLink.out $(OBJ)

MissingLink.bin : MissingLink.out
	$(OBJCOPY) -R .eeprom -O binary MissingLink.out MissingLink.bin

program: MissingLink.bin
	../bootloader/uploader/sysexupload MissingLink.bin

MissingLink.hex : MissingLink.out
	$(OBJCOPY) -R .eeprom -O ihex MissingLink.out MissingLink.hex

programmkII : MissingLink.hex
	avrdude  -pm328p -cavrispmkII -Pusb -e -Ulock:w:0x3F:m -Uefuse:w:0x00:m -Uhfuse:w:0xda:m -Ulfuse:w:0xff:m
	avrdude  -pm328p -cavrispmkII -Pusb -Uflash:w:MissingLink.hex:i -Ulock:w:0x0F:m

programmkIIFull : MissingLink.hex
	avrdude  -pm328p -cavrispmkII -Pusb -e -Ulock:w:0x3F:m -Uefuse:w:0x00:m -Uhfuse:w:0xda:m -Ulfuse:w:0xff:m
	avrdude  -pm328p -cavrispmkII -Pusb -Uflash:w:MissingLinkFull.hex:i -Ulock:w:0x0F:m

programmtinyFull : MissingLinkFull.hex
	avrdude  -pm328p -cusbtiny  -e -Ulock:w:0x3F:m -Uefuse:w:0x00:m -Uhfuse:w:0xda:m -Ulfuse:w:0xff:m
	avrdude  -pm328p -cusbtiny  -Uflash:w:MissingLinkFull.hex:i -Ulock:w:0x0F:m

clean:
	rm -f *.o *.map *.out *.hex */*.o *.bin
	rm -rf ../Libraries/*/*.o
