/*
 
 ArdOSC - OSC Library for Arduino.
 
 -------- Lisence -----------------------------------------------------------
 
 ArdOSC
 
 The MIT License
 
 Copyright (c) 2009 - 2010 recotana( http://recotana.com )　All right reserved
 
 */	


#ifndef OSCEncoder_h
#define OSCEncoder_h



#include "OSCMessage.h"

class OSCEncoder{
	
private:
	
	
public:
	
	int16_t encode( OSCMessage::OSCMessage *mes ,uint8_t *sendData );
	
};

#endif