CFLAGS = -Wall -g -I. -I../ -I../midi
LDFLAGS = -lportmidi
CC = g++
SRC = main.cpp \
		../midi/sysex_tools.c

OBJ = ${SRC:.cpp=.o}

current: sysexupload

.cpp.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} -o $*.o $<

sysexupload: ${OBJ}
	@echo CC $@
	@${CC} ${CFLAGS} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f *.o sysexupload

all: sysexupload
