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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <avr/interrupt.h>
#include "Parse.h"
#include "Output.h"

bool getMessageInfo(OSCMessage& oscMess, MessageInfo& mi)
{
   const char* addr = oscMess.getOSCAddress();
   int addrLen = oscMess.getOSCAddressLen();
   if (addrLen <= 0)
      return false;

   mi.isZ = (addrLen > 2) && (addr[addrLen - 2] == '/') && (addr[addrLen - 1] == 'z');
   mi.hasX = ((addr[0] == 'x') || addr[0] == 'X');
   mi.hasY = ((addr[0] == 'y') || addr[0] == 'Y');
   for (int i = 1; i < addrLen; i++)
   {
      mi.hasX |= ((addr[i] == 'x' || addr[i] == 'X') && (addr[i-1] != '0'));
      mi.hasY |= (addr[i] == 'y' || addr[i] == 'Y');
   }

   // Make sure this message passes basic checks
   // Make sure we've got at least one argument if we're reference it in the message
   if ((mi.hasX || mi.isZ) && oscMess.getArgsNum() < 1)
      return false;
   // Make sure there's a y argument if we're referencing y
   if (mi.hasY && oscMess.getArgsNum() < 2)
      return false;

   return true;
}

uint8_t getTouchArg(OSCMessage& oscMess, uint8_t arg, float fScale, float fOffset)
{
   switch( oscMess.getTypeTag(arg) )
   {
   case 'i':
      return oscMess.getInteger32(arg);
      break;

   case 'f':
      {
         const float HAXVALUE = 0.9f; // Make sliders in TouchOSC happier
         return trunc(oscMess.getFloat(arg) * (fScale+HAXVALUE)) + fOffset;
      }
      break;

   case 's':
   default :
      return 0xFF;
      break;
   }
}

int getValue(const char* str, char** numstr, OSCMessage& oscMess)
{
   char* currOffset = (char*) str;
   float offset = 0.0f;
   float scale = 127.0f;
   int argIndex = -1;
   int whichBits = 0; // 0 = all, 1 = lsb7, 2 = msb7

   // Eat whitespace
   while ((*currOffset == ' ') || (*currOffset == '_'))
      currOffset++;
   // Handle constants
   if (*currOffset >= '0' && *currOffset <= '9')
   {
      return strtol(currOffset, numstr, 0);
   }
   // Handle variables
   if (*currOffset == 'x' || *currOffset == 'y' || *currOffset == 'z' ||
       *currOffset == 'X' || *currOffset == 'Y' || *currOffset == 'Z')
   {
      switch (*currOffset)
      {
      case 'X' :
      case 'x' : // first var
         argIndex = 0;
         break;
      case 'Y':
      case 'y': // second var
         argIndex = 1;
         break;
      case 'Z' :
      case 'z' : // first var for /z messages
         argIndex = 0;
         break;
      }
      currOffset++;
      if (*currOffset == 'L')
      {
         whichBits = 1;
         currOffset++;
      } else {
         if (*currOffset == 'M')
         {
            whichBits = 2;
            currOffset++;
         }
      }
      // Check for offset/scale
      if (*currOffset == '(')
      {
         currOffset++; // (
         const char* oldOffset = currOffset;
         float lowEnd = (float) strtol(currOffset, &currOffset, 0);
         // Check for parse issues
         if (oldOffset == currOffset)
            return 0;
         // Skip ".."
         currOffset += 2;
         // Parse the next param
         oldOffset = currOffset;
         float highEnd = (float) strtol(currOffset, &currOffset, 0);
         if (oldOffset == currOffset)
            return 0;
         currOffset++; // )
         offset = lowEnd;
         scale = highEnd - lowEnd;
      }
      *numstr = currOffset;
      int ret = getTouchArg(oscMess, argIndex, scale, offset);
      switch (whichBits)
      {
      case 1 :
         return ret & 0x7F;
         break;
      case 2 :
         return (ret >> 7) & 0x7F;
         break;
      default :
         return ret;
         break;
      }
   }
   return 0;
}

void sendChunk(const char* inbuffer, const uint8_t inlen, OSCMessage& oscMess)
{
   char* currOffset = (char*) inbuffer;

   while (currOffset < inbuffer + inlen)
   {
      cli();
      const char* oldOffset = currOffset;
      uint8_t val = (uint8_t) getValue(currOffset, &currOffset, oscMess);
      if (oldOffset != currOffset)
      {
         sendByte(val);
      } else {
         currOffset++;
      }
      sei();
   }
   DEBUG_PRINTF("\n");
}
