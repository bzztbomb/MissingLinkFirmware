/*
 * Missing Link Firmware
 */
#include <string.h>

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include <WiShield.h>
#include <OSCMessage.h>
#include <wiring.h>

#include "led.h"
#include "HandleMidi.h"
#include "Parse.h"
#include "Output.h"
#include "udpapp-ml.h"
extern "C"
{
#include "ML_Config.h"
}

int wifiInit = 0;

#define BLINK_LENGTH 20
unsigned long POVBlink = 0;

void dumpMessage(OSCMessage& oscMess)
{
#ifdef DEBUG_OUT
   unsigned char *ip=oscMess.getIpAddress();

   DEBUG_PRINTF("%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
   DEBUG_PRINTF(":%d %s %s --", oscMess.getPortNumber(), oscMess.getOSCAddress(), oscMess.getTypeTags());

   for(uint8_t i=0 ; i<oscMess.getArgsNum(); i++)
   {
      switch( oscMess.getTypeTag(i) )
      {
      case 'i':
         DEBUG_PRINTF("%d ", oscMess.getInteger32(i));
         break;

      case 'f':
         DEBUG_PRINTF("%f ", oscMess.getFloat(i));
         break;
      }
   }
   DEBUG_PRINTF("\n");
#endif
}

//--------------------------------------------------------------------------
void setup()
{
   // Pin1, Output: Data available for PIC
   // Pin2&3, Output: LED
   DDRC = _BV(1) | _BV(2) | _BV(3);
   PORTC = ~(_BV(1));
   LED_RED;
   initOutput();
   initConfig();
   missinglink_init_midi();
   WiFi.init(1);
   DEBUG_PRINTF("setup complete.\n");
}

void loop()
{
   if (wifiInit)
   {
      WiFi.run();
      if (millis() - POVBlink > BLINK_LENGTH)
      {
         LED_GREEN;
      }
   } else {
      wifiInit = WiFi.continueInit();
      if (wifiInit)
      {
         LED_GREEN;
      }
   }

   missinglink_midi_process();
}

void handlePingMessage(OSCMessage& oscMess, void* userArg)
{
   if ((fromOSC & TO_OSC) == TO_OSC)
      return; // Ping response will be handled by OSC->OSC

   const uint8_t* ip = oscMess.getIpAddress();
   bounceMessage(ip[0], ip[1], ip[2], ip[3], 12345);
}

//
// Message dispatching stuff below.
//

// Function pointer and a struct to hold message/handler combo.
typedef void (*MessageProcessor)(OSCMessage& oscMess, void* userData);
struct MessageHandler
{
   char* PROGMEM address;
   MessageProcessor processor;
};

uint8_t dispatch(OSCMessage& oscMess, unsigned char offset, const MessageHandler* messages, void* userArg)
{
   const char* OSCaddress=(oscMess.getOSCAddress()+offset);  // 5 = /midi
   for (unsigned char i = 0; messages[i].address != NULL; i++)
   {
      if (strstr_P(OSCaddress, messages[i].address) == OSCaddress)
      {
         messages[i].processor(oscMess, userArg);
         return 1;
      }
   }
   return 0;
}

void midiDispatch(OSCMessage& oscMess, void* userArg)
{
   /*static */const MessageHandler messages[] = {
      { PSTR("/z"), &handleTouchOSCMidiZ},
      { PSTR("/button"), &handleMidiButton},
//      { PSTR("/clock"), &unimplemented },
      { PSTR("/"), &handleTouchOSCMidi},
      { NULL, NULL }
   };
   // 5 = /midi
   dispatch(oscMess, 5, messages, NULL);
}

typedef void (*ConfigMessageProcessor)(OSCMessage& oscMess, uint8_t messageLength, uint8_t* eepromOffset);
struct ConfigMessageHandler
{
   char* PROGMEM address;
   ConfigMessageProcessor processor;
   uint8_t messageLength;
   uint8_t* eepromOffset;
};

void handleSetDefaults(OSCMessage& oscMess, uint8_t messageLength, uint8_t* eepromOffset)
{
   setDefaultConfig();
}

void handleReboot(OSCMessage& oscMess, uint8_t messageLength, uint8_t* eepromOffset)
{
   void(* resetFunc) (void) = 0; //declare reset function @ address 0
   resetFunc();
}

void handleWriteEeprom(OSCMessage& oscMess, uint8_t messageOffset, uint8_t* eepromOffset)
{
   const char quoteChar = '\'';
   enum ConfigParseState {
      cpsNumeric,
      cpsString
   };
   uint8_t* eOffset = eepromOffset;
   const char* addr = oscMess.getOSCAddress();
   int addrLen = oscMess.getOSCAddressLen();
   char* currOffset = (char*) (addr + messageOffset); // /config/set/
   ConfigParseState parseState = cpsNumeric;
   while (currOffset < addr + addrLen)
   {
      if (parseState == cpsNumeric)
      {
         // Check to see if we should switch to string mode
         if (*currOffset == quoteChar)
         {
            parseState = cpsString;
            currOffset++;
         } else {
            // If not, try and parse numbers (including variables)
            char* oldOffset = currOffset;
            uint8_t val = (uint8_t) getValue(currOffset, &currOffset, oscMess);
            if (oldOffset != currOffset)
            {
               eeprom_write_byte((uint8_t*) eOffset, val);
               eOffset++;
            }
            currOffset++;
         }
      } else {
         // Check to see if we should switch to number mode
         if (*currOffset == quoteChar)
         {
            parseState = cpsNumeric;
            currOffset++;
         } else {
            // Nope, just take the value and write it
            uint8_t val = *currOffset;
            eeprom_write_byte(eOffset, val);
            eOffset++;
            currOffset++;
         }
      }
   }
}

void handleConfigSet(OSCMessage& oscMess, uint8_t messageLength, uint8_t* ignoredEepromOffset)
{
   char* addr = (char*) oscMess.getOSCAddress();
   char* currOffset = (char*) (addr + messageLength); // /config/set/
   char* oldOffset = currOffset;
   uint16_t eepromOffset = getValue(currOffset, &currOffset, oscMess);
   if (currOffset != oldOffset)
      handleWriteEeprom(oscMess, currOffset - addr, (uint8_t*) eepromOffset);
}

uint8_t configDispatcher(OSCMessage& oscMess, unsigned char offset, const ConfigMessageHandler* messages)
{
   const char* OSCaddress=(oscMess.getOSCAddress()+offset);  // 5 = /midi
   for (unsigned char i = 0; messages[i].address != NULL; i++)
   {
      if (strstr_P(OSCaddress, messages[i].address) == OSCaddress)
      {
         messages[i].processor(oscMess, messages[i].messageLength, messages[i].eepromOffset);
         return 1;
      }
   }
   return 0;
}

void configDispatch(OSCMessage& oscMess, void* userArg)
{
   // A little silly, but I want to prevent strlen calls and ram
   // usage.
   const uint8_t twoCharLen = 11;
   const uint8_t threeCharLen = twoCharLen + 1;
   const uint8_t fourCharLen = threeCharLen + 1;
   const uint8_t sixCharLen = fourCharLen + 2;
   const uint8_t sevenCharLen = sixCharLen + 1;
   const uint8_t eightCharLen = sevenCharLen + 1;
   const uint8_t tenCharLen = eightCharLen + 2;
   const uint8_t elevenCharLen = tenCharLen + 1;

   const ConfigMessageHandler messages[] = {
      { PSTR("defaults"), &handleSetDefaults, 0xFF, (uint8_t*) NULL },
      { PSTR("reboot"), &handleReboot, 0xFF, (uint8_t*) NULL },
      { PSTR("set"), &handleConfigSet, threeCharLen, (uint8_t*) NULL },
      { PSTR("mode"), &handleWriteEeprom, fourCharLen, CONFIG_OFFSET(wireless_mode) },
      { PSTR("ssid"), &handleWriteEeprom, fourCharLen, CONFIG_OFFSET(ssid) },
      { PSTR("security"), &handleWriteEeprom, eightCharLen, CONFIG_OFFSET(security_type) },
      { PSTR("wep1"), &handleWriteEeprom, fourCharLen, CONFIG_OFFSET(wep_keys)+ZG_MAX_ENCRYPTION_KEY_SIZE*0 },
      { PSTR("wep2"), &handleWriteEeprom, fourCharLen, CONFIG_OFFSET(wep_keys)+ZG_MAX_ENCRYPTION_KEY_SIZE*1 },
      { PSTR("wep3"), &handleWriteEeprom, fourCharLen, CONFIG_OFFSET(wep_keys)+ZG_MAX_ENCRYPTION_KEY_SIZE*2 },
      { PSTR("wep4"), &handleWriteEeprom, fourCharLen, CONFIG_OFFSET(wep_keys)+ZG_MAX_ENCRYPTION_KEY_SIZE*3 },
      { PSTR("wpa"), &handleWriteEeprom, threeCharLen, CONFIG_OFFSET(security_passphrase) },
      { PSTR("ip"), &handleWriteEeprom, twoCharLen, CONFIG_OFFSET(local_ip) },
      { PSTR("gateway"), &handleWriteEeprom, sevenCharLen, CONFIG_OFFSET(gateway_ip) },
      { PSTR("subnet"), &handleWriteEeprom, sixCharLen, CONFIG_OFFSET(subnet_mask) },
      { PSTR("fromMIDI"), &handleWriteEeprom, eightCharLen, CONFIG_OFFSET(fromMIDI) },
      { PSTR("fromUSBMIDI"), &handleWriteEeprom, elevenCharLen, CONFIG_OFFSET(fromUSBMIDI) },
      { PSTR("fromOSC"), &handleWriteEeprom, sevenCharLen, CONFIG_OFFSET(fromOSC) },
      { PSTR("targetIp"), &handleWriteEeprom, eightCharLen, CONFIG_OFFSET(oscTargetIp) },
      { PSTR("targetPort"), &handleWriteEeprom, tenCharLen, CONFIG_OFFSET(oscTargetPort) },
      { NULL, NULL, NULL, (uint8_t*) NULL }
   };
   uint8_t messageLength = 8; // /config/
   configDispatcher(oscMess, messageLength, messages);
}

bool canBlink()
{
   unsigned long now = millis();
   if (now - POVBlink > BLINK_LENGTH)
   {
      POVBlink = now;
      return true;
   } else {
      return false;
   }
}

// Main dispatcher
void handleOSCMessage(OSCMessage& oscMess)
{
   // For future MIDI->OSC messages
   if (!oscTargetSet)
   {
      memcpy(oscTargetIp, oscMess.getIpAddress(), 4);
   }

   // Blink the LED!
   if (canBlink())
   {
      LED_OFF;
   }

   DEBUG_PRINTF("\nhandleOSCMessage:  ");
   dumpMessage(oscMess);
   DEBUG_PRINTF("\n");

   // List of messages and handlers, make sure it ends with NULL address
   /*static */const MessageHandler messages[] = {
      { PSTR("/midi"), &midiDispatch},
      { PSTR("/ping"), &handlePingMessage },
      { PSTR("/config/"), &configDispatch },
      { NULL, NULL }
   };

   dispatch(oscMess, 0, messages, NULL);

   if ((fromOSC & TO_OSC) == TO_OSC)
   {
      const uint8_t* ip = oscMess.getIpAddress();
      bounceMessage(ip[0], ip[1], ip[2], ip[3], 12345);
   }
}

int main(void)
{
	init();

	setup();

	for (;;)
		loop();

	return 0;
}
