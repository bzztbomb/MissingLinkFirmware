/*
 
 ArdOSC - OSC Library for Arduino.
 
 -------- Lisence -----------------------------------------------------------
 
 ArdOSC
 
 The MIT License
 
 Copyright (c) 2009 - 2010 recotana( http://recotana.com )　All right reserved
 
 */	



#ifndef OSCcommon_h
#define OSCcommon_h

//======== user define ==============

//#define _DEBUG_

#define _USE_FLOAT_
#define _USE_STRING_



//======== user define  end  ========

#define kMaxAugument	16

#define kMaxRecieveData	100
#define kMaxOSCAdrCharactor	255
#define kMaxStringCharactor	255



extern "C" {
#include <inttypes.h>
}


#ifdef _DEBUG_
#include "HardwareSerial.h"
#endif

#ifdef _DEBUG_
#define DBG_LOGLN(...)	Serial.println(__VA_ARGS__)
#define DBG_LOG(...)	Serial.print(__VA_ARGS__)

#else
#define DBG_LOGLN
#define DBG_LOG
#endif




#endif