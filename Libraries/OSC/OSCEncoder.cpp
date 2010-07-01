/*
 
 ArdOSC - OSC Library for Arduino.
 
 -------- Lisence -----------------------------------------------------------
 
 ArdOSC
 
 The MIT License
 
 Copyright (c) 2009 - 2010 recotana( http://recotana.com )　All right reserved
 
 */	

#include <stdlib.h>
#include <string.h>

#include "OSCcommon.h"
#include "OSCEncoder.h"



int16_t OSCEncoder::encode( OSCMessage::OSCMessage *mes ,uint8_t *sendData ){
	
	
	uint8_t *packStartPtr=sendData;
	
	
//=========== OSC Address(String) -> BIN Encode   ===========
	
	memcpy( sendData ,mes->oscAddress ,mes->oscAdrSize);	
	packStartPtr += mes->oscAdrPacSize;
	mes->allPackSize = mes->oscAdrPacSize;
	
//=========== TypeTag(String) -> BIN Encode   ===========
	
	*packStartPtr=',';	
	memcpy(packStartPtr+1 ,mes->typeTag ,mes->argsNum);	
	packStartPtr += mes->typeTagPacSize;
	
	mes->allPackSize += mes->typeTagPacSize;
	
	
//=========== Auguments -> BIN Encode   ==================
	
	
	uint8_t *tmpPtr;
	
	uint32_t tmpInt;
	
#ifdef _USE_FLOAT_
	float	tmpFloat;
#endif
	
#ifdef _USE_STRING_
	uint16_t strSize;
#endif
	
	for (int i=0; i< mes->argsNum; i++) {
		
		switch ( mes->typeTag[i] ) {
				
			case 'i':
				
				tmpInt = mes->getInteger32(i);
				tmpPtr =(uint8_t*)(&tmpInt);
				
				*packStartPtr++=*(tmpPtr+3);
				*packStartPtr++=*(tmpPtr+2);
				*packStartPtr++=*(tmpPtr+1);
				*packStartPtr++=*tmpPtr;
				
				mes->allPackSize += 4;
				break;
				
#ifdef _USE_FLOAT_
			case 'f':
				
				tmpFloat = mes->getFloat(i);
				tmpPtr=(uint8_t*)(&tmpFloat);
				
				*packStartPtr++=*(tmpPtr+3);
				*packStartPtr++=*(tmpPtr+2);
				*packStartPtr++=*(tmpPtr+1);
				*packStartPtr++=*tmpPtr;
				
				mes->allPackSize += 4;
				break;
#endif
				
#ifdef _USE_STRING_
			case 's':
				strSize = strlen(mes->getString(i));
				
				if(strSize > kMaxStringCharactor){
					
					DBG_LOGLN("encode max str err");
					return 1;
				}
				
				memcpy( (char*)packStartPtr ,mes->getString(i) ,strSize );				
				packStartPtr += mes->getPackSize(strSize);
				
				mes->allPackSize += mes->getPackSize(strSize);
				break;
#endif
				
		}
	}	
	return 0;
}
