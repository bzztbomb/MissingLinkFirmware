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

#ifndef BOOTLOADERCFG_H
#define BOOTLOADERCFG_H

/*

//turn on port b bit zero as input
//turn on pullups
#define BOOTLOADER_INIT \
	DDRB = 0xFE; \
	PORTB = 0x01;

//a zero tells us that the button is down
#define BOOTLOADER_CONDITION (!(PINB & _BV(PINB0)))
*/

//turn on port b bit zero as input
//turn on pullups
#ifndef BOOTLOADER_INIT
#define BOOTLOADER_INIT
#endif

//a zero tells us that the button is down
#ifndef BOOTLOADER_CONDITION
#define BOOTLOADER_CONDITION (!(PIND & _BV(PIND4)))
#endif

#ifndef INDICATE_BOOTLOADER_MODE
#define INDICATE_BOOTLOADER_MODE \
	PORTB = 170;
#endif

//could be reduced [probably] to decrease ram size, but needs to be at least 43
//long in current implementation
#ifndef MIDIIN_BUF_SIZE
#define MIDIIN_BUF_SIZE 128
#endif

#endif
