/*
 * MissingLink Firmware
 * Copyright 2011 Jabrudian Industries LLC
 *
 * MissingLink Firmware is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MissingLink Firmware is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MissingLink Firmware.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _OUTPUT_H_
#define _OUTPUT_H_

//#define DEBUG_OUT

#ifdef DEBUG_OUT
#include <stdio.h>
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

void initOutput();
void sendByte(unsigned char b);

#endif
