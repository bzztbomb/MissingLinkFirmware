/*
 
 ArdOSC - OSC Library for Arduino.
 
 -------- Lisence -----------------------------------------------------------
 
 ArdOSC
 
 The MIT License
 
 Copyright (c) 2009 - 2010 recotana( http://recotana.com )　All right reserved
 
 */				   

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "OSCcommon.h"
#include "OSCMessage.h"






OSCMessage::OSCMessage()
{	
	DBG_LOGLN("construct message");

	oscAddress=NULL;
	typeTag=NULL;
	arguments=NULL;
	
	flush();
 
}

OSCMessage::OSCMessage(const char *_oscAddress)
{
	DBG_LOGLN("construct message with Adr");
	oscAddress=NULL;
	typeTag=NULL;
	arguments=NULL;
	
	flush();
	
	setOSCAddress(_oscAddress);

}

OSCMessage::~OSCMessage()
{
	DBG_LOGLN("destruct message");
	flush();
}


void OSCMessage::flush(){
	
	if(oscAddress!=NULL) free(oscAddress);
	
	oscAddress=NULL;
	oscAdrSize=0;
	oscAdrPacSize=0;
	
	if(typeTag!=NULL) free(typeTag);
	
	typeTag=NULL;
	typeTagSize=0;
	typeTagPacSize=0;
	
	if(arguments!=NULL){
		
		for (uint16_t i=0; i< argsNum; i++) {
			free((void*)arguments[i]);
			arguments[i]=NULL;
		}
		free(arguments);		
	}
	argsNum=0;
	argsPacSize=0;	
	
	arguments=NULL;
	
	setIpAddress(0,0,0,0);
	portNumber=0;
	
	DBG_LOGLN("flush message obj");
	
}

void OSCMessage::setAddress(uint8_t *_ip , uint16_t _port){
	
	setIpAddress(_ip);
	
	portNumber=_port;
	
}
void OSCMessage::setIpAddress(uint8_t *_ip){
		
	ipAddress[0] = _ip[0];
	ipAddress[1] = _ip[1];
	ipAddress[2] = _ip[2];
	ipAddress[3] = _ip[3];
	
	DBG_LOG("set IP Adr with IParray:");
	DBG_LOG((int)ipAddress[0]);
	DBG_LOG(".");
	DBG_LOG((int)ipAddress[1]);
	DBG_LOG(".");
	DBG_LOG((int)ipAddress[2]);
	DBG_LOG(".");
	DBG_LOGLN((int)ipAddress[3]);

}

void OSCMessage::setIpAddress( uint8_t _ip1 ,uint8_t _ip2 ,uint8_t _ip3 ,uint8_t _ip4 ){
	
	ipAddress[0] = _ip1;
	ipAddress[1] = _ip2;
	ipAddress[2] = _ip3;
	ipAddress[3] = _ip4;
	
	DBG_LOG("set IP Adr with IP nums:");
	DBG_LOG((int)ipAddress[0]);
	DBG_LOG(".");
	DBG_LOG((int)ipAddress[1]);
	DBG_LOG(".");
	DBG_LOG((int)ipAddress[2]);
	DBG_LOG(".");
	DBG_LOGLN((int)ipAddress[3]);
	
}

uint8_t *OSCMessage::getIpAddress(){
	return ipAddress;
}

void OSCMessage::setPortNumber(uint16_t _port){
	portNumber=_port;
}

uint16_t OSCMessage::getPortNumber(){
	return portNumber;
}





int16_t OSCMessage::setOSCMessage( const char *_address ,char *types , ... ){
	
	va_list argsList;
	
	DBG_LOG("set OSCMessage:");
	
	
	if( setOSCAddress(_address) > 0) return 1;
	if( setTypeTags(types) > 0) return 1;
	
	
	
	
	//-------------------------------------------------	
	
	if (arguments!=NULL) free(arguments);
	arguments=(void**)calloc(argsNum, sizeof(void*));
	
	
	
	argsPacSize=0;
	
	void *tmpPtr;
	char *tmpStr;
	uint16_t strSize;
	
	va_start( argsList, types );	
	
	for(uint8_t i=0 ; i < argsNum ; i++){
		
		typeTag[i]=types[i];
		
		
		switch(types[i]){
				
			case 'i':
				tmpPtr=calloc(1, 4);
				
				memcpy(tmpPtr,(int32_t *)va_arg(argsList, int32_t *),4);
				arguments[i]=(int32_t *)tmpPtr;
				argsPacSize += 4;
				
				DBG_LOG(" int(4byte):");
				DBG_LOG(*(int32_t *)tmpPtr);
				
				break;
				
				
#ifdef _USE_FLOAT_				
			case 'f':
				tmpPtr=calloc(1, 4);
				
				memcpy(tmpPtr,(float *)va_arg(argsList, float *),4);
				arguments[i]=(float *)tmpPtr;
				
				argsPacSize += 4;
				
				DBG_LOG(" float(4byte):");
				DBG_LOG(*(float *)tmpPtr);
				
				break;
#endif	
				
				
#ifdef _USE_STRING_
			case 's':
				tmpStr=(char *)va_arg(argsList, char *);
				strSize=strlen(tmpStr);
				DBG_LOGLN(strSize);
				if(strSize > kMaxStringCharactor){
					
					DBG_LOGLN("set args message max str err");
					return 1;
				}
				
				tmpPtr=calloc(strSize+1, 1);
				strcpy((char*)tmpPtr, tmpStr);
				
				arguments[i]=tmpPtr;
				argsPacSize += getPackSize( strSize );
				
				DBG_LOG(" String,size:");
				DBG_LOG(strSize);
				DBG_LOG(" packsize:");
				DBG_LOG(getPackSize( strSize ));
				DBG_LOG(" :");
				DBG_LOG((char *)tmpPtr);
				
				break;
#endif
				
		}
		
	}
	DBG_LOGLN("");
	DBG_LOGLN("");
	return 0;
}




int16_t OSCMessage::setOSCAddress(const char *_address){

	oscAdrSize=(uint16_t)strlen(_address);
	
	if( oscAdrSize > kMaxOSCAdrCharactor ){
		DBG_LOGLN("set OSC max OSC Adr err");
		flush();
		return 1;
	}
	
	if(oscAddress!=NULL) free(oscAddress);
	
	oscAddress=(char*)calloc(1,oscAdrSize+1);
	strcpy(oscAddress,_address);
		
	oscAdrPacSize=getPackSize(oscAdrSize);
	
	DBG_LOG("set OSC Adr:");
	DBG_LOG(oscAddress);
	DBG_LOG(" size:");
	DBG_LOG(oscAdrSize);
	DBG_LOG(" packsize:");
	DBG_LOGLN(oscAdrPacSize);
	
	return 0;
}
char   *OSCMessage::getOSCAddress(){
	return oscAddress;
}



int16_t OSCMessage::setTypeTags(const char *_tags ){

	uint8_t maxErr=0;
	
	argsNum=(uint16_t)strlen(_tags);
	
	if(argsNum > kMaxAugument){
		DBG_LOGLN("set tags max arg err");
		flush();
		return 1;
	}
	
	typeTagSize = argsNum+1;
	
	if(typeTag!=NULL) free(typeTag);
	typeTag=(char*)calloc(1,argsNum+1);

	strcpy(typeTag,_tags);
	
	typeTagPacSize=getPackSize(typeTagSize);
	
	argsNum=typeTagSize;
	
	
	DBG_LOG("set TypeTag:");
	DBG_LOG(typeTag);
	DBG_LOG(" size:");
	DBG_LOG(typeTagSize);
	DBG_LOG(" packsize:");
	DBG_LOGLN(typeTagPacSize);
	
	return 0;
}

char  * OSCMessage::getTypeTags(){
	return typeTag;
}

char  OSCMessage::getTypeTag(uint16_t _index){
	if(_index>argsNum) _index=argsNum-1;
	return typeTag[_index];
}


int16_t OSCMessage::getArgsNum(){
	return argsNum;
}


int32_t OSCMessage::getInteger32(uint16_t _index){
	
	int32_t *value;
	if(_index > argsNum) _index=argsNum-1;
	value = (int32_t *)arguments[_index];
	
	return *value;
	
}

#ifdef _USE_FLOAT_
float OSCMessage::getFloat(uint16_t _index){
	
	float *value;
	if(_index > argsNum) _index=argsNum-1;
	value = (float *)arguments[_index];
	
	return *value;
	
	
}
#endif

#ifdef _USE_STRING_
char * OSCMessage::getString(uint16_t _index){
	
	if(_index > argsNum) _index=argsNum-1;
	
	return (char *)arguments[_index];
	
}
#endif

uint16_t OSCMessage::getAllPackSize(){
	
	return oscAdrPacSize+typeTagPacSize+argsPacSize;
}


uint16_t OSCMessage::getStrPackSize(const char* data){
	
	return getPackSize(strlen(data));
}

uint16_t OSCMessage::getPackSize(uint16_t size){
	
	return (size+4)&0xfffc;
}




