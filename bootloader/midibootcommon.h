/*
 * A sysex bootloader for avr chips.
 * Copyright 2010 Alex Norman
 *
 * This file is part of SysexBoot.
 *
 * SysexBoot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SysexBoot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SysexBoot.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//this is a file that both the uploader and the atmega code uses
#ifndef MIDIBOOT_COMMON_H
#define MIDIBOOT_COMMON_H

#ifndef SYSEX_EDUMANUFID
#define SYSEX_EDUMANUFID 0x7D
#endif

#define MIDI_SYSEX_START 0xF0
#define MIDI_SYSEX_STOP 0xF7

//the edu manufacture id + 'x37vboot'
const unsigned char midiboot_sysex_id[] = {
	SYSEX_EDUMANUFID,
   120, 51, 55, 118, 98, 111, 111, 116
};

#define MIDIBOOT_SYSEX_ID_LEN 9

typedef enum {
	MIDIBOOT_INVALID = 0, 
	MIDIBOOT_LEAVE_BOOT = 1, 
	MIDIBOOT_GETPAGESIZE = 2, 
	MIDIBOOT_FILLPAGE = 3,
	MIDIBOOT_WRITEPAGE = 4
} midiboot_sysex_t;

#endif
