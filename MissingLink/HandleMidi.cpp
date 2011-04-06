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
#include <stdlib.h>
#include <string.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include "led.h"
#include "HandleMidi.h"
#include "MLSerial.h"
#include "WiShield.h"
#include "Parse.h"
#include "Output.h"
#include "wiring.h"
#include "udpapp.h"
#include "udpapp-ml.h"
extern "C" {
#include "midi.h"
#include "ML_Config.h"
#include "twi.h"
#include "bytequeue.h"
#include "uip.h"
}

//
// Device stuff
//
static MidiDevice md_serial;
static MidiDevice md_usbToOSC;
static MidiDevice md_serialToOSC;

// sysex
static uint8_t serial_sysex_index = 0;
static uint8_t serial_sysex_command = 0xFF;
static uint8_t usb_sysex_index = 0;
static uint8_t usb_sysex_command = 0xFF;

// USB out (via I2C)
#define USB_OUT_BUFFER_SIZE 64
static uint8_t usb_buff[USB_OUT_BUFFER_SIZE];
static byteQueue_t usb_queue;

uint8_t onReceiveService(uint8_t inByte);
void onRequestService();
bool sendByteUSB(unsigned char b);

void sendByte(unsigned char b)
{
   if ((fromOSC & TO_MIDI) == TO_MIDI)
   {
      Serial.write(b);
   }
   if ((fromOSC & TO_USBMIDI) == TO_USBMIDI)
   {
      sendByteUSB(b);
   }
}

void midi_send_serial(MidiDevice * device, uint8_t count, uint8_t byte0, uint8_t byte1, uint8_t byte2) {
   uint8_t out[3];
   out[0] = byte0;
   out[1] = byte1;
   out[2] = byte2;
   if (count > 3)
      count = 3;

   uint8_t i;
   for(i = 0; i < count; i++) {
      Serial.write(out[i]);
   }
}

void midi_check_sysex(MidiDevice * device, uint8_t count, uint8_t byte0, uint8_t byte1, uint8_t byte2)
{
   // This below is a little hacky, but I think it's good for a couple of reasons.
   if (device == &md_serial)
   {
      if ((fromMIDI & TO_MIDI) == TO_MIDI)
      {
         // Here we send a full packet in one blast, which will minimize chances of
         // merge issues.  (Still have long sysex to worry about)
         midi_send_serial(&md_serial, count, byte0, byte1, byte2);
      }
      if ((fromMIDI & TO_USBMIDI) == TO_USBMIDI)
      {
         // Here we have a full packet for the host to grab.  Less time of
         // the host asking for packets repeatedly and we not being able to provide them.
         uint8_t out[3];
         out[0] = byte0;
         out[1] = byte1;
         out[2] = byte2;
         if (count > 3)
            count = 3;

         uint8_t i;
         for(i = 0; i < count; i++) {
            sendByteUSB(out[i]);
         }
      }
   }

   // Header contents:  start, dev id, "M", "L", "N", "K", Command, Data, F7
   // TODO: PROGMEM this shit
   // 240, 125, 77, 76, 78, 75,
   static const uint8_t ML_HEADER[] = {0xF0, 0x7D, 0x4D, 0x4C, 0x4E, 0x4B};
   static const uint8_t ML_HEADER_LEN = 6;

   uint8_t in[3];
   in[0] = byte0;
   in[1] = byte1;
   in[2] = byte2;
   if (count > 3)
      count = 3;

   uint8_t& sysex_index = (device == &md_serial) ? serial_sysex_index : usb_sysex_index;
   uint8_t& sysex_command = (device == &md_serial) ? serial_sysex_command : usb_sysex_command;
   for (uint8_t i = 0; i < count; i++)
   {
      if (sysex_index < ML_HEADER_LEN)
      {
         if (in[i] == ML_HEADER[sysex_index])
         {
            sysex_index++;
        } else {
            DEBUG_PRINTF("index: %d, mismatch: %d %d\n", sysex_index, in[i], ML_HEADER[sysex_index]);
            sysex_index = 0;
         }
         DEBUG_PRINTF("csysindex: %d\n", sysex_index);
      } else {
         if (sysex_index == ML_HEADER_LEN)
         {
            if (in[i] != SYSEX_END)
            {
               sysex_command = in[i];
               DEBUG_PRINTF("ML SYSEX: %d\n", sysex_command);
               sysex_index++;
            } else {
               sysex_index = 0; // reset
            }
         } else {
               // dispatch
            switch (sysex_command)
            {

               case 0: // Reset configuration to defaut, (if we need to get aggressive about code size, kill this and use eeprom write, 0, 255, device reset to get back to defaults.
                  {
                     if (in[i] == SYSEX_END)
                     {
                        DEBUG_PRINTF("Resetting to default.\n");
                        setDefaultConfig();
                        sysex_index = 0xFF;
                     }
                  }
                  break;
               case 2 : // write to eeprom location
                  {
                     static uint16_t eeprom_offset = 0;
                     static uint8_t currdata = 0;
                     uint16_t offset = sysex_index - ML_HEADER_LEN - 1; // for command
                     // Accumlate offset
                     if (offset < 4)
                     {
                        if (offset == 0)
                           eeprom_offset = 0;
                        DEBUG_PRINTF("\noffset: %d, %d\n", offset, in[i]);
                        eeprom_offset |= in[i] << ((3-offset)*4);
                     } else {
                        // Actually write data, we split 4 bits between two bytes (super inefficient, but simple)
                        offset -= 4; // 1 byte for command, 4 bytes for length
                        if (offset++%2 == 0)
                        {
                           currdata = in[i] << 4;
                           DEBUG_PRINTF("storing high byte: %d, offset: %d\n", currdata, offset);
                        } else {
                           currdata |= in[i];
                           offset /= 2; // Since we use two bytes to store one byte of data
                           offset--; // -1 to account for the initial two bytes
                           eeprom_write_byte((uint8_t*) (eeprom_offset + offset), currdata);
                           DEBUG_PRINTF("writing %x @ offset %d\n", currdata, eeprom_offset + offset);
                        }
                     }
                  }
                  break;
               case 1 : // Reset (handled by PIC)
                  break;
               case 3 : // Enter AVR bootloader, handled on PIC
               default:
                  sysex_index = 0xFF; // reset (the increment that happens will overflow to 0)
                  break;
            }

            if (in[i] != SYSEX_END)
            {
               sysex_index++;
            } else {
               sysex_index = 0; // reset
            }
         }
      }
   }
}

void sendMidiOSC(MidiDevice * device, uint8_t count, uint8_t byte0, uint8_t byte1, uint8_t byte2)
{
   uip_udp_prep_buffer();
   char* buf = (char*) uip_appdata;
   // TODO: Progmem me
   const char* msg = "/midi/0x";
   const uint8_t msglen = 8;
   strcpy(buf, msg);
   buf += msglen;
   // Status byte
   itoa(byte0, buf, 16);
   buf += strlen(buf);
   // First byte
   if (count > 1)
   {
      *buf++ = ' ';
      itoa(byte1, buf, 10);
      buf += strlen(buf);
   }
   // Second byte
   if (count > 2)
   {
      *buf++ = ' ';
      *buf++ = 'x';
   }
   // pad, add comma, typetag, data
   uint8_t pad = buf - (char*)uip_appdata; // current size
   pad = ((pad+4)&0xFC) - pad;
   // Clear this gap.
   for (uint8_t i = 0; i < pad; i++)
   {
      *buf++ = 0;
   }
   // comma
   *buf++ = ',';
   if (count > 2)
   {
      // typetag
      *buf++ = 'f';
      *buf++ = 0;
      *buf++ = 0;
      // data
      float data = ((float) byte2) / 127.0f;
      uint8_t* tmpptr = (uint8_t*) &data;
      *buf++ = *(tmpptr+3);
      *buf++ = *(tmpptr+2);
      *buf++ = *(tmpptr+1);
      *buf++ = *(tmpptr+0);
   }
   sendAppBuffer(buf - (char*)uip_appdata, oscTargetIp[0], oscTargetIp[1], oscTargetIp[2], oscTargetIp[3], oscTargetPort);
   WiFi.sendUdp();
}

void missinglink_init_midi()
{
   midi_device_init(&md_serial);
   midi_device_set_send_func(&md_serial, midi_send_serial);
   midi_register_fallthrough_callback(&md_serial, midi_check_sysex);

   midi_device_init(&md_usbToOSC);
   midi_register_fallthrough_callback(&md_usbToOSC, sendMidiOSC);
   midi_device_init(&md_serialToOSC);
   midi_register_fallthrough_callback(&md_serialToOSC, sendMidiOSC);

   bytequeue_init(&usb_queue,  usb_buff, USB_OUT_BUFFER_SIZE);

   twi_setAddress(4);
   twi_attachSlaveTxEvent(onRequestService);
   twi_attachSlaveRxEvent(onReceiveService);
   twi_init();
}

void missinglink_midi_process()
{
   // Serial
   uint8_t amt = Serial.available();
   if (amt > 3)
      amt = 3;
   uint8_t buf[3];
   for (uint8_t i = 0; i < amt; i++)
      buf[i] = Serial.read();
   midi_device_input(&md_serial, amt, buf[0], buf[1], buf[2]);
   midi_device_process(&md_serial);

   // Blink LED, main loop will reset back
   if (amt > 0 && canBlink())
   {
      LED_RED;
   }

   // Serial to OSC
   if ((fromMIDI & TO_OSC) == TO_OSC)
   {
      midi_device_input(&md_serialToOSC, amt, buf[0], buf[1], buf[2]);
      midi_device_process(&md_serialToOSC);
   }

   // Process this too!
   if ((fromUSBMIDI & TO_OSC) == TO_OSC)
   {
      midi_device_process(&md_usbToOSC);
   }
}

//
// Message stuff
//
void handleSimpleMess(OSCMessage& oscMess, int msgOffset)
{
   const char* addr = oscMess.getOSCAddress();
   int addrLen = oscMess.getOSCAddressLen() - msgOffset;
   const char* currOffset = addr + msgOffset;

   sendChunk(currOffset, addrLen, oscMess);
}

// Handles a "touchosc midi" address which requires different parsing.
void handleTouchOSCMidi(OSCMessage& oscMess, void* userArg)
{
   MessageInfo mi;
   if (!getMessageInfo(oscMess, mi))
      return;
   if (mi.isZ)
      return;
   // For any message without variable, send only when x > 0
   if (!mi.hasX && !mi.hasY && oscMess.getArgsNum() > 0)
   {
      if (!(getTouchArg(oscMess, 0, 1.0f, 0.0f) > 0.0f))
         return;
   }
   handleSimpleMess(oscMess, 6); // /midi/
}

// Handles a "touchosc midi" address which requires different parsing.
void handleTouchOSCMidiZ(OSCMessage& oscMess, void* userArg)
{
   const char* addr = oscMess.getOSCAddress();
   int addrLen = oscMess.getOSCAddressLen() - 6;

   // Find the z on, x or x/y slide, and z off chunks
   uint8_t chunks[4];
   const int startPos = 8; // len(/midi/z/)
   chunks[0] = startPos;
   uint8_t currChunk = 1;
   for (uint8_t i = startPos; i < addrLen + 7 && currChunk < 4; i++)
   {
      if (addr[i] == '/')
      {
         chunks[currChunk++] = i;
      }
   }
   if (currChunk < 3)
      return; // bad message

   // Ok, if currChunk == 3 then it's an x or x/y message (since the /z will be missing from the address)
   if (currChunk == 3)
   {
      sendChunk(addr + (chunks[1]), chunks[2] - chunks[1], oscMess);
   } else {
      if (oscMess.getFloat(0) > 0.0f)
      {
         // push down
         sendChunk(addr + (chunks[0]), chunks[1] - chunks[0], oscMess);
      } else {
         // push up
         sendChunk(addr + (chunks[2]), chunks[3] - chunks[2], oscMess);
      }
   }
}

// Handles a "touchosc midi" address which requires different parsing.
void handleMidiButton(OSCMessage& oscMess, void* userArg)
{
   const char* addr = oscMess.getOSCAddress();
   int addrLen = oscMess.getOSCAddressLen();

   MessageInfo mi;
   if (!getMessageInfo(oscMess, mi))
      return;
   if (mi.isZ)
      return;

   // Find the z on, x or x/y slide, and z off chunks
   const int startPos = 13; // len(/midi/button/)
   uint8_t chunks[2] = {startPos, startPos};
   uint8_t currChunk = 1;
   for (uint8_t i = startPos; addr[i] != 0 && currChunk < 2; i++)
   {
      if (addr[i] == '/')
      {
         chunks[currChunk++] = i;
      }
   }
   if (currChunk < 2)
      return; // bad message

   // Ok, if currChunk == 3 then it's an x or x/y message (since the /z will be missing from the address)
   if (oscMess.getFloat(0) > 0.0f)
   {
      // push down
      sendChunk(addr + chunks[0], chunks[1] - chunks[0], oscMess);
   } else {
      // push up
      sendChunk(addr + chunks[1], addrLen - chunks[1], oscMess);
   }
}

// USB IN -> MIDI OUT
void setDataReadyFlag()
{
   if (bytequeue_length(&usb_queue) > 0)
   {
      PORTC |= _BV(1); // on!
   } else {
      PORTC &= ~(_BV(1)); // off!
   }
}

bool sendByteUSB(unsigned char b)
{
   bool ret = bytequeue_enqueue(&usb_queue, b);
   setDataReadyFlag();
   return ret;
}

uint8_t onReceiveService(uint8_t inByte)
{
   if ((fromUSBMIDI & TO_USBMIDI) == TO_USBMIDI)
   {
      if (!sendByteUSB(inByte) ||
          (bytequeue_length((byteQueue_t*) &usb_queue) >= USB_OUT_BUFFER_SIZE - 1))
      {
         return 0;
      }
   }
   midi_check_sysex(NULL, 1, inByte, 0, 0);
   if ((fromUSBMIDI & TO_MIDI) == TO_MIDI)
   {
      Serial.write(inByte);
   }
   if ((fromUSBMIDI & TO_OSC) == TO_OSC)
   {
      midi_device_input(&md_usbToOSC, 1, inByte, 0, 0);
   }
   return 1;
}

void onRequestService()
{
   unsigned char out[5];
   // SYSEX message can span multiple packets, so we need to store that state.
   static uint8_t usb_in_sysex_mode = 0;

   // Figure out lengths, we never send more than 3 bytes at a time
   uint8_t qlen = bytequeue_length(&usb_queue);
   if (qlen > 3)
      qlen = 3;
   uint8_t midi_packet_len = 0;

   // Figure out how much data to send
   if (qlen > 0)
   {
      uint8_t status_byte = bytequeue_get(&usb_queue, 0);
      if (status_byte == SYSEX_BEGIN)
      {
         usb_in_sysex_mode = 1;
      } else {
         midi_packet_len = midi_packet_length(status_byte);
      }
   }

   if (usb_in_sysex_mode == 0)
   {
      // Not a sysex message, just wait until we have all the data we need and
      // send it off.
      if (qlen >= midi_packet_len)
      {
         if (midi_packet_len > 0)
         {
            out[0] = 4;
            out[1] = bytequeue_get(&usb_queue, 0) >> 4;
            for (uint8_t i = 0; i < midi_packet_len; i++)
            {
               out[i+2] = bytequeue_get(&usb_queue, i);
            }
            bytequeue_remove(&usb_queue, midi_packet_len);
         } else {
            // Hrm, we're not in sysex mode and there's no status byte. Just consume
            // the byte
            bytequeue_remove(&usb_queue, 1);
         }
      }
   } else {
      // Search ahead for sysex_end
      uint8_t end = 0xFF;
      for (uint8_t i = 0; i < qlen; i++)
      {
         if (bytequeue_get(&usb_queue, i) == SYSEX_END)
         {
            end = i+1;
            usb_in_sysex_mode = 0;
            break;
         }
      }
      // If this is the end of a message, or we have a full packet, send it.
      if ((end != 0xFF) || (qlen >= 3))
      {
         out[0] = 4;
         // Figure out sysex flag byte
         out[1] = 0x4 + (end != 0xFF ? end : 0);
         // Loop until sysex_end or buffer end
         uint8_t consume = end != 0xFF ? end : 3;
         for (uint8_t i = 0; i < consume; i++)
         {
            out[i+2] = bytequeue_get(&usb_queue, i);
         }
         bytequeue_remove(&usb_queue, consume);
      }
   }
   // Finally, let's send this data!
   twi_transmit(out, out[0]+1);
   if (bytequeue_length(&usb_queue) == 0)
   {
      PORTC &= ~(_BV(1));
   }
}
