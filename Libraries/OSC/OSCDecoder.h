/*
 
 ArdOSC - OSC Library for Arduino.
 
 -------- Lisence -----------------------------------------------------------
 
 ArdOSC
 
 The MIT License
 
 Copyright (c) 2009 - 2010 recotana( http://recotana.com )　All right reserved
 
 */	

#ifndef OSCDecoder_h
#define OSCDecoder_h

#include "OSCMessage.h"

class OSCDecoder{
	
private:
	
	
public:
	
	int16_t decode( OSCMessage::OSCMessage *mes ,const uint8_t *recData );
		
};

#endif