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
#ifndef _ML_CONFIG_H_
#define _ML_CONFIG_H_

#include "witypes.h"
#include "g2100.h"

#define CURR_CONFIG_VERSION 3

typedef struct MissingLinkConfig
{
   // 0xFF = unprogrammed/invalid, 0x00 = valid data
   unsigned char version; // Offset: 0

   //
   // Wireless config
   //
   // 1 = infrastructure, 2 = adhoc
   unsigned char wireless_mode; // Offset 1

   char ssid[ZG_MAX_SSID_LENGTH]; // Offset: 2

   // Wireless security config: open; 1 - WEP; 2 - WPA; 3 - WPA2
   unsigned char security_type;  // Offset: 34

   // WEP keys (4, 13)
   unsigned char wep_keys[ZG_MAX_ENCRYPTION_KEYS][ZG_MAX_ENCRYPTION_KEY_SIZE]; // Offset: 35

   // WPA/WPA2 passpharse
   const prog_char security_passphrase[ZG_MAX_WPA_PASSPHRASE_LEN]; // Offset: 87

   //
   // TCP/IP
   //
   unsigned char local_ip[4]; // Offset: 151
   unsigned char gateway_ip[4]; // Offset: 155
   unsigned char subnet_mask[4]; // Offset: 159

   //
   // MIDI signal routing
   // bit 0 = toMIDI
   // bit 1 = toUSBMidi
   // bit 2 = toOSC
   //
   unsigned char fromMIDI; // Offst: 163
   unsigned char fromUSBMIDI; // Offset: 164
   unsigned char fromOSC; // Offst: 165

   // Where to send OSC messages
   unsigned char oscTargetIp[4]; // 166
   uint16_t oscTargetPort; // 167
} MissingLinkConfig;

#define TO_MIDI 0x1
#define TO_USBMIDI 0x2
#define TO_OSC 0x4

extern unsigned char fromMIDI;
extern unsigned char fromUSBMIDI;
extern unsigned char fromOSC;
extern unsigned char oscTargetSet; // 1 = don't override
extern unsigned char oscTargetIp[];
extern uint16_t oscTargetPort;

#define CONFIG_OFFSET(x) ((uint8_t*) offsetof(MissingLinkConfig, x))
#define CONFIG_OFFSET16(x) ((uint16_t*) offsetof(MissingLinkConfig, x))

void initConfig();
void setDefaultConfig();

#endif
