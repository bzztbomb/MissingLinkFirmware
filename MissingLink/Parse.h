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
#ifndef _PARSE_H_
#define _PARSE_H_

#include "OSCMessage.h"

struct MessageInfo
{
   bool isZ; // does this message end in /z?
   bool hasX; // does this message reference an X variable?
   bool hasY; // does this message reference a Y variable?
};

uint8_t getTouchArg(OSCMessage& oscMess, uint8_t arg, float fScale = 127.0, float fOffset = 0.0f);
int getValue(const char* str, char** numstr, OSCMessage& msg);
void sendChunk(const char* inbuffer, const uint8_t inlen, OSCMessage& oscMess);
bool getMessageInfo(OSCMessage& oscMess, MessageInfo& mi);

#endif
