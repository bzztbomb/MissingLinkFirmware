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
#ifndef _HANDLEMIDI_H_
#define _HANDLEMIDI_H_

#include "OSCMessage.h"

// Setup/process
void missinglink_init_midi();
void missinglink_midi_process();

// Message handlers
void handleTouchOSCMidi(OSCMessage& oscMess, void* userArg);
void handleTouchOSCMidiZ(OSCMessage& oscMess, void* userArg);
void handleMidiButton(OSCMessage& oscmess, void* userArg);

void handleMidiStart(OSCMessage& ocsMess, void* userArg);
void handleMidiContinue(OSCMessage& ocsMess, void* userArg);
void handleMidiStop(OSCMessage& ocsMess, void* userArg);
void handleMidiReset(OSCMessage& ocsMess, void* userArg);
void handleMidiTune(OSCMessage& ocsMess, void* userArg);
void handleMidiSong(OSCMessage& oscMess, void* userArg);
void handleMidiSysex(OSCMessage& oscMess, void* userArg);

void handleChannelNoteOn(OSCMessage& oscMess, void* userArg);
void handleChannelNoteOff(OSCMessage& oscMess, void* userArg);
void handleChannelCC(OSCMessage& oscMess, void* userArg);
void handleChannelAftertouch(OSCMessage& oscMess, void* userArg);
void handleChannelPitch(OSCMessage& oscMess, void* userArg);
void handleChannelPatch(OSCMessage& oscMess, void* userArg);
void handleChannelPressure(OSCMessage& oscMess, void* userArg);

// I2C/USB midi
void receiveEvent(int howMany);
void requestEvent();

struct ChannelMsgInfo
{
   char* messageOffset;
   int channel;
};

#endif
