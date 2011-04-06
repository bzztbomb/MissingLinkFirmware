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

#include <avr/io.h>

int main(void) {
	uint8_t i, j, k, cnt = 0;
	DDRB = 0xFF;
	PORTB = 0xF0;

   DDRC = 0xFF;
   PORTC = 0xFF;

	while(1){
		for(i = 0; i < 255; i++)
			for(j = 0; j < 128; j++)
				for(k = 0; k < 128; k++);
		cnt++;
		PORTB = ~cnt;
      if (PORTC == 0)
         PORTC = 0xFF;
      else
         PORTC = 0x00;
	}

	return 0;
}

