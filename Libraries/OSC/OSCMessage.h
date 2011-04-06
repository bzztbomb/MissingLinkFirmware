/*

 ArdOSC - OSC Library for Arduino.

 -------- Lisence -----------------------------------------------------------

 ArdOSC

 The MIT License

 Copyright (c) 2009 - 2010 recotana( http://recotana.com )　All right reserved

 */

#ifndef OSCMessage_h
#define OSCMessage_h

#include "OSCcommon.h"

class OSCMessage
{
public:
	OSCMessage();
	OSCMessage(const char *_oscAddress);
	~OSCMessage();

	void flush();

	uint16_t getAllPackSize();
	uint16_t getStrPackSize(const char* data);
	static uint16_t getPackSize(uint16_t size);

	void setAddress(uint8_t *_ip , uint16_t _port);

	void setIpAddress( uint8_t *_ip );
	void setIpAddress( uint8_t _ip1, uint8_t _ip2,	uint8_t _ip3, uint8_t _ip4);

	uint8_t *getIpAddress();
	void setPortNumber(uint16_t _port);
	uint16_t getPortNumber();

	int16_t setOSCMessage( const char *_address ,char *types , ... );
	int16_t getArgsNum();

	int16_t setOSCAddress(const char *_address);
	const char* getOSCAddress();
   uint16_t getOSCAddressLen() { return oscAdrSize; }

	int16_t setTypeTags(const char *_tags);
	const char* getTypeTags();
	char	getTypeTag(uint16_t _index);

	int32_t	getInteger32(uint16_t _index);

#ifdef _USE_FLOAT_
	float	getFloat(uint16_t _index);
#endif

#ifdef _USE_STRING_
	char   *getString(uint16_t _index);
#endif

	friend class OSCServer;
	friend class OSCClient;
	friend class OSCDecoder;
	friend class OSCEncoder;
private:
	uint16_t	allPackSize;	//
	const char* oscAddress;		// "/ard"
	uint16_t	oscAdrSize;		// "/ard" ->4
	uint16_t	oscAdrPacSize;	// "/ard\0\0\0\0" ->8

	const char* typeTag;		// "iif"
	uint16_t	typeTagSize;	// ",iif" -> 4
	uint16_t	typeTagPacSize;	// ",iif\0\0\0\0" -> 8

   const uint8_t* argStart;
	uint16_t	argsNum;		// "iif" -> 3
	uint16_t	argsPacSize;	// "iif" -> 4 + 4 + 4 = 12

	uint8_t		ipAddress[4];	//{192,168,0,10}
	uint16_t	portNumber;		//10000

   void getRaw(uint16_t _index, void* ptr);
};

#endif
