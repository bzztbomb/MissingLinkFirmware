
/*
 * UDP endpoint
 *
 * A simple UDP endpoint example using the WiShield 1.0
 */

#include <WiShield.h>
#include <OSCMessage.h>

#define WIRELESS_MODE_INFRA	1
#define WIRELESS_MODE_ADHOC	2

// Wireless configuration parameters ----------------------------------------
unsigned char local_ip[] = {192,168,1,23};	// IP address of WiShield
unsigned char gateway_ip[] = {192,168,1,1};	// router or gateway IP address
unsigned char subnet_mask[] = {255,255,255,0};	// subnet mask for the local network
const prog_char ssid[] PROGMEM = {"craque"};		// max 32 bytes

unsigned char security_type = 3;	// 0 - open; 1 - WEP; 2 - WPA; 3 - WPA2

// WPA/WPA2 passphrase
const prog_char security_passphrase[] PROGMEM = {"DEADBEEF23"};	// max 64 characters

// WEP 128-bit keys
// sample HEX keys
prog_uchar wep_keys[] PROGMEM = {	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,	// Key 0
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00,	// Key 1
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00,	// Key 2
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00	// Key 3
								};

// setup the wireless mode
// infrastructure - connect to AP
// adhoc - connect to another WiFi device
unsigned char wireless_mode = WIRELESS_MODE_INFRA;

unsigned char ssid_len;
unsigned char security_passphrase_len;
//---------------------------------------------------------------------------

void setup()
{
   Serial.begin(9600); 
   Serial.println("init");
   WiFi.init();
}

void loop()
{
	WiFi.run();
}

void handleOSCMessage(OSCMessage& oscMess)
{
    uint16_t i;
    unsigned char *ip=oscMess.getIpAddress();
    
    long int intValue;
    float floatValue;
    char *stringValue;
    
    Serial.print(ip[0],DEC);
    Serial.print(".");
    Serial.print(ip[1],DEC);
    Serial.print(".");
    Serial.print(ip[2],DEC);
    Serial.print(".");
    Serial.print(ip[3],DEC);
    Serial.print(":");
    
    Serial.print(oscMess.getPortNumber());
    Serial.print(" ");
    Serial.print(oscMess.getOSCAddress());
    Serial.print(" ");
    Serial.print(oscMess.getTypeTags());
    Serial.print("--");
    
    for(i=0 ; i<oscMess.getArgsNum(); i++){
      
     switch( oscMess.getTypeTag(i) ){
      
        case 'i':       
          intValue = oscMess.getInteger32(i);
          
          Serial.print(intValue);
          Serial.print(" ");
         break; 
         
         
        case 'f':        
          floatValue = oscMess.getFloat(i);
        
          Serial.print(floatValue);
          Serial.print(" ");
         break; 
        
        
         case 's':         
          stringValue = oscMess.getString(i);
         
          Serial.print(stringValue);
          Serial.print(" ");
         break;       
     }         
    }
    Serial.println("");  
}

