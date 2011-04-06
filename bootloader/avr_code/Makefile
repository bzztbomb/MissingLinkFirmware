MCU=atmega328p
BOOTLOADER_ADDRESS=0x7800

CC=avr-gcc
OBJCOPY=avr-objcopy
# optimize for size:
CFLAGS= -mmcu=$(MCU) -I. -I../ -I../midi/ -I../../Libraries/twi/ -Wall -Wstrict-prototypes -Os -fpack-struct -fshort-enums -funsigned-bitfields
LDFLAGS += -Wl,-Ttext=$(BOOTLOADER_ADDRESS)

SRC = main.c \
		bootloadercfg.h \
		../../Libraries/twi/twi.c \
		../midi/bytequeue/interrupt_setting.c \
		../midi/bytequeue/bytequeue.c \

OBJ  = ${SRC:.c=.o}

#-------------------
all: main.hex
#-------------------

.c.o:
	@echo CC $<
	@$(CC) -c $(CFLAGS) -Os -o $*.o $<

main.hex : main.out
	$(OBJCOPY) -R .eeprom -O ihex main.out main.hex

main.out : $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o main.out -Wl,-Map,main.map $(OBJ)

program: main.hex
	avrdude  -pm328p -cavrispmkII -Pusb -e -Ulock:w:0x3F:m -Uefuse:w:0x00:m -Uhfuse:w:0xda:m -Ulfuse:w:0xff:m
	avrdude  -pm328p -cavrispmkII -Pusb -Uflash:w:main.hex:i -Ulock:w:0x0F:m

#-------------------
clean:
	rm -f *.o *.map *.out *.hex
	rm -rf ../../Libraries/*/*.o
#-------------------
