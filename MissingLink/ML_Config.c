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
#include "ML_Config.h"
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "config.h"
#include "Output.h"

// Wireless params
unsigned char wireless_mode = WIRELESS_MODE_ADHOC;
unsigned char security_type = 0; // 0 - open; 1 - WEP; 2 - WPA; 3 - WPA2

// TCP/IP params
unsigned char local_ip[] = {192,168,1,100}; // IP address of WiShield
unsigned char gateway_ip[] = {192,168,1,1}; // router or gateway IP address
unsigned char subnet_mask[] = {255,255,255,0}; // subnet mask for the local network

// MIDI routing params
unsigned char fromMIDI;
unsigned char fromUSBMIDI;
unsigned char fromOSC;

// For MIDI->OSC routing
unsigned char oscTargetSet = 0;
unsigned char oscTargetIp[] = {0, 0, 0, 0};
uint16_t oscTargetPort = 12345;

void setDefaultConfig()
{
   MissingLinkConfig mlconfig;
   memset(&mlconfig, 0, sizeof(mlconfig));
   mlconfig.version = CURR_CONFIG_VERSION;
   mlconfig.wireless_mode = WIRELESS_MODE_ADHOC; // adhoc
   strcpy_P(mlconfig.ssid, PSTR("MissingLink"));
   mlconfig.security_type = 0;
   mlconfig.local_ip[0] = 192;
   mlconfig.local_ip[1] = 168;
   mlconfig.local_ip[2] = 1;
   mlconfig.local_ip[3] = 100;
   mlconfig.subnet_mask[0] = 255;
   mlconfig.subnet_mask[1] = 255;
   mlconfig.subnet_mask[2] = 255;
   mlconfig.subnet_mask[3] = 0;

   mlconfig.fromMIDI = TO_MIDI;
   mlconfig.fromUSBMIDI = 0;
   mlconfig.fromOSC = TO_MIDI;

   mlconfig.oscTargetPort = 12345;

   eeprom_write_block(&mlconfig, 0, sizeof(MissingLinkConfig));
}

void dumpConfig()
{
#if 0
#ifdef DEBUG_OUT
   MissingLinkConfig m;
   eeprom_read_block(&m, 0, sizeof(MissingLinkConfig));
   DEBUG_PRINTF("%d: version: %d\n", CONFIG_OFFSET(version), m.version);
   DEBUG_PRINTF("%d: wireless_mode: %d\n", CONFIG_OFFSET(wireless_mode), m.wireless_mode);
   DEBUG_PRINTF("%d: ssid: %s\n", CONFIG_OFFSET(ssid), m.ssid);
   DEBUG_PRINTF("%d: security_type: %d\n", CONFIG_OFFSET(security_type), m.security_type);
   DEBUG_PRINTF("%d: wep_keys\n", CONFIG_OFFSET(wep_keys));
   DEBUG_PRINTF("%d: passphrase: %s\n", CONFIG_OFFSET(security_passphrase), m.security_passphrase);
   DEBUG_PRINTF("%d: ip: %d.%d.%d.%d\n", CONFIG_OFFSET(local_ip), m.local_ip[0], m.local_ip[1], m.local_ip[2], m.local_ip[3]);
   DEBUG_PRINTF("%d: gateway_ip: %d.%d.%d.%d\n", CONFIG_OFFSET(gateway_ip), m.gateway_ip[0], m.gateway_ip[1], m.gateway_ip[2], m.gateway_ip[3]);
   DEBUG_PRINTF("%d: subnet_mask: %d.%d.%d.%d\n", CONFIG_OFFSET(subnet_mask), m.subnet_mask[0], m.subnet_mask[1], m.subnet_mask[2], m.subnet_mask[3]);
#endif
#endif
}

void initConfig()
{
   uint8_t version = eeprom_read_byte(CONFIG_OFFSET(version));
   if (version != CURR_CONFIG_VERSION)
   {
      setDefaultConfig();
   }

   dumpConfig();

   // Read in our config.
   wireless_mode = eeprom_read_byte(CONFIG_OFFSET(wireless_mode));
   security_type = eeprom_read_byte(CONFIG_OFFSET(security_type));
   eeprom_read_block(local_ip, CONFIG_OFFSET(local_ip), 4);
   eeprom_read_block(subnet_mask, CONFIG_OFFSET(subnet_mask), 4);
   eeprom_read_block(gateway_ip, CONFIG_OFFSET(gateway_ip), 4);

   fromMIDI = eeprom_read_byte(CONFIG_OFFSET(fromMIDI));
   fromUSBMIDI = eeprom_read_byte(CONFIG_OFFSET(fromUSBMIDI));
   fromOSC = eeprom_read_byte(CONFIG_OFFSET(fromOSC));
   eeprom_read_block(oscTargetIp, CONFIG_OFFSET(oscTargetIp), 4);
   oscTargetSet = (oscTargetIp[0] != 0 || oscTargetIp[1] != 0 || oscTargetIp[2] != 0 || oscTargetIp[3] != 0);
   oscTargetPort = eeprom_read_word(CONFIG_OFFSET16(oscTargetPort));
}

void return_ssid(uint8_t* ssid, uint8_t* ssid_len)
{
   eeprom_read_block(ssid, CONFIG_OFFSET(ssid), ZG_MAX_SSID_LENGTH);
   *ssid_len = strnlen(ssid, ZG_MAX_SSID_LENGTH);
}

void return_passphrase(uint8_t* passphrase, uint8_t* passphrase_len)
{
   eeprom_read_block(passphrase, CONFIG_OFFSET(security_passphrase), ZG_MAX_WPA_PASSPHRASE_LEN);
   *passphrase_len = strnlen(passphrase, ZG_MAX_WPA_PASSPHRASE_LEN);
}

void return_wepkeys(uint8_t* keys)
{
   eeprom_read_block(keys, CONFIG_OFFSET(wep_keys), ZG_MAX_ENCRYPTION_KEYS * ZG_MAX_ENCRYPTION_KEY_SIZE);
}
