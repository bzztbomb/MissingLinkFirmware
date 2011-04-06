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
#include "OSCDecoder.h"



int16_t OSCDecoder::decode( OSCMessage::OSCMessage *mes ,const uint8_t *recData ){


//===========  BIN -> OSC Address(String) Decode   ===========

	const uint8_t *packStartPtr=recData;

	mes->setOSCAddress((char*)packStartPtr);

	packStartPtr += mes->oscAdrPacSize;



//===========  BIN -> TypeTag (String) Decode   ===========

	mes->setTypeTags((char*)(packStartPtr+1));

	packStartPtr += mes->typeTagPacSize;


//===========  BIN -> Auguments Decode   ===========

	mes->argStart = packStartPtr;
	mes->argsPacSize=mes->argsNum * 4;

	return 0;
}
