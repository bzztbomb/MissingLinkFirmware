/*
 * A sysex bootloader for avr chips.
 * Copyright 2010 Alex Norman
 *
 * This file is part of SysexBoot.
 *
 * SysexBoot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SysexBoot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SysexBoot.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#define F_CPU 16000000UL
#define IN_BUFSIZE 260
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <util/delay.h>
#include <inttypes.h>
#include <stdbool.h>
#include "twi.h"
#include "bytequeue/bytequeue.h"
#include "bootloadercfg.h"
#include "midibootcommon.h"

#define SYSEX_BEGIN 0xF0
#define SYSEX_END 0xF7

#define MIDI_CIN_SYSEX_START                    0x4
#define MIDI_CIN_SYSEX_ENDS_1                   0x5
#define MIDI_CIN_SYSEX_ENDS_2                   0x6
#define MIDI_CIN_SYSEX_ENDS_3                   0x7

// From I2C master
uint8_t midiInBuf[MIDIIN_BUF_SIZE];
volatile byteQueue_t midiByteQueue;

// To I2C master
#define MIDIOUT_BUF_SIZE 64
uint8_t midiOutBuf[MIDIOUT_BUF_SIZE];
volatile byteQueue_t midiOutQueue;

uint8_t onReceiveService(uint8_t inByte)
{
   if (bytequeue_enqueue((byteQueue_t*) &midiByteQueue, inByte))
   {
      if (bytequeue_length((byteQueue_t*) &midiByteQueue) < MIDIIN_BUF_SIZE - 1)
         return 1;
   }
   return 0;
}

void onRequestService(void)
{
   uint8_t data[5];
   uint8_t i;

   // Packet length
   data[0] = bytequeue_length((byteQueue_t*) &midiOutQueue);
   if (data[0] > 3)
      data[0] = 3;
   // USB midi status byte
   data[1] = MIDI_CIN_SYSEX_START;
   for (i = 0; i < data[0]; i++)
   {
      data[i+2] = bytequeue_get((byteQueue_t*) &midiOutQueue, i);
      // Search for SYSEX_END and set USB status byte if needed.
      if (data[i+2] == SYSEX_END)
      {
         data[1] += (i+1);
      }
   }
   // If we're not sending a full packet, it better have a sysex_end in it
   if ((data[1] == MIDI_CIN_SYSEX_START) && (data[0] < 3))
   {
      data[0] = 0;
   } else {
      bytequeue_remove((byteQueue_t*) &midiOutQueue, data[0]);
      data[0] = 4;
   }

   twi_transmit(data, data[0]+1);
   if (bytequeue_length((byteQueue_t*) &midiOutQueue) == 0)
      PORTC &= ~(_BV(1));
}

void (*jump_to_app)(void) = 0x0;

void exit_bootloader(void){
   cli();
   boot_rww_enable();
   MCUCR = _BV(IVCE);
   MCUCR = 0;
   jump_to_app();
}

void midi_send_byte(uint8_t b)
{
   bytequeue_enqueue((byteQueue_t*) &midiOutQueue, b);
   PORTC |= _BV(1);    // notify PIC we've got data
}

void midi_send_encoded_byte(uint8_t b)
{
   midi_send_byte(b >> 4);
   midi_send_byte(b & 0xF);
}

void send_sysex_start(void) {
   uint8_t i = 0;
   midi_send_byte(SYSEX_BEGIN);
   for(i = 0; i < MIDIBOOT_SYSEX_ID_LEN; i++)
      midi_send_byte(midiboot_sysex_id[i]);
}

void send_sysex_end(void) {
   midi_send_byte(SYSEX_END);
}

//The ack is just our sysex id  in a sysex message, that is all
void send_ack(void){
   send_sysex_start();
   send_sysex_end();
}

int main(void) {
   uint8_t i, size, curByte;
   bool cancelTimeout = false;
   bool inSysexMode = false;
   uint16_t inByteIndex = 0;
   uint16_t pageAddress = 0;
   uint16_t counterLow = 0;

   //page size is 128, need 2 more bytes for page address
   //128 + 2 == 130..
   //we actually don't need so much space but, whatever
   uint8_t tmpUnpackedData[IN_BUFSIZE / 2];
   //once we pack it, it is 149 bytes
   uint8_t tmpPackedData[IN_BUFSIZE];
   midiboot_sysex_t sysexMode = MIDIBOOT_INVALID;

   //set up the bootloader conditions
   MCUCR = _BV(IVCE);
   MCUCR = _BV(IVSEL);

   // Everything INPUT except LED pins
   DDRC = _BV(1) | _BV(2) | _BV(3);
   // Enable pullups, set LED red
   PORTC = 0xFB;

   bytequeue_init((byteQueue_t *)&midiByteQueue, midiInBuf, MIDIIN_BUF_SIZE);
   bytequeue_init((byteQueue_t*) &midiOutQueue, midiOutBuf, MIDIOUT_BUF_SIZE);

   // Init I2C
   twi_setAddress(4);
   twi_attachSlaveTxEvent(onRequestService);
   twi_attachSlaveRxEvent(onReceiveService);
   twi_init();

   sei();

   //do main loop
   while(true){
      // Cheesy timeout method, but frugal on bytes
      if ((++counterLow == 0) && (!cancelTimeout))
      {
            exit_bootloader();
      }

      //read data from the queue and deal with it
      size = bytequeue_length((byteQueue_t *)&midiByteQueue);
      //deal with input data
      for(i = 0; i < size; i++){
         curByte = bytequeue_get((byteQueue_t *)&midiByteQueue, i);

         if(curByte == SYSEX_BEGIN){
            inSysexMode = true;
            inByteIndex = 0;
            sysexMode = MIDIBOOT_INVALID;
         } else if(curByte == SYSEX_END){
            if(inSysexMode){
               //see what mode we're in
               switch(sysexMode){
                  case MIDIBOOT_LEAVE_BOOT:
                     exit_bootloader();
                     break;
                  case MIDIBOOT_GETPAGESIZE:
                     //if we've been sent the correct size packet then send back our data
                     if(inByteIndex == (1 + MIDIBOOT_SYSEX_ID_LEN)){
                        cancelTimeout = true;
                        //pack the contents of SPM_PAGESIZE into tmpPackedData
                        //SPM_PAGESIZE is 2 bytes wide
                        send_sysex_start();
                        midi_send_byte(MIDIBOOT_GETPAGESIZE);
                        midi_send_encoded_byte(SPM_PAGESIZE >> 8);
                        midi_send_encoded_byte(SPM_PAGESIZE & 0xFF);
                        send_sysex_end();
                     }
                     break;
                     //actually write the page that has been filled up through MIDIBOOT_FILLPAGE
                  case MIDIBOOT_WRITEPAGE:

                     cli();

                     //erase the page
                     eeprom_busy_wait ();
                     boot_page_erase (pageAddress);
                     boot_spm_busy_wait ();
                     //write the page
                     boot_page_write (pageAddress);     // Store buffer in flash page.
                     boot_spm_busy_wait();       // Wait until the memory is written.

                     sei();

                     send_ack();
                     break;
                     //write data into the temporary page buffer [boot_page_fill]
                  case MIDIBOOT_FILLPAGE:
                     if(inByteIndex > (MIDIBOOT_SYSEX_ID_LEN + 1)) {
                        const uint16_t packedSize = inByteIndex - MIDIBOOT_SYSEX_ID_LEN - 1;
                        const uint16_t unpackedSize = packedSize / 2;
                        if(unpackedSize <= 64){
                           //the first two bytes are the address
                           uint16_t bytesToWrite = unpackedSize - 2;
                           uint16_t writeStartAddr;

                           //unpack our addr+data
                           uint16_t n;
                           for (n = 0; n < packedSize; n+=2)
                           {
                              tmpUnpackedData[n >> 1] = (tmpPackedData[n] << 4) | tmpPackedData[n+1];
                           }

                           //grab the start address
                           writeStartAddr = (tmpUnpackedData[0] << 8) | tmpUnpackedData[1];
                           //page address, just zero out the lower bits, used for WRITEPAGE
                           pageAddress = writeStartAddr & ~(SPM_PAGESIZE - 1);

                           cli();

                           //fill the temp page buffer
                           uint8_t * pageData = tmpUnpackedData + 2;
                           uint16_t j;
                           for(j = 0; j < bytesToWrite; j+=2){
                              uint16_t w = (((uint16_t)pageData[j + 1]) << 8) + pageData[j];
                              boot_page_fill (writeStartAddr + j, w);
                           }

                           sei();
                           //ack that we've done this so that we can get some more data
                           send_ack();
                        }
                     }
                     break;
                  default:
                     break;
               }
            }
            inSysexMode = false;
         } else if(inSysexMode){
            //make sure the sysex prefix matches ours
            if((inByteIndex < MIDIBOOT_SYSEX_ID_LEN) && (curByte != midiboot_sysex_id[inByteIndex])){
               inSysexMode = false;
            } else {
               //we're in sysex mode and we've matched the midiboot_sysex_id
               //the next byte tells us what to do
               if(inByteIndex == MIDIBOOT_SYSEX_ID_LEN){
                  if(curByte >= MIDIBOOT_LEAVE_BOOT && curByte <= MIDIBOOT_WRITEPAGE)
                  {
                     sysexMode = curByte;
                  }
                  else
                     inSysexMode = false;
                  //if we're filling a page, write it to the tmpPackedData, we'll unpack later
               } else if (sysexMode == MIDIBOOT_FILLPAGE){
                  //the first MIDIBOOT_SYSEX_ID_LEN + 1 bytes have already been dealt with
                  uint16_t index = inByteIndex - MIDIBOOT_SYSEX_ID_LEN - 1;
                  if(index < IN_BUFSIZE) {
                     tmpPackedData[index] = curByte;
                  } else {
                     //XXX ERROR!!!
                     sysexMode = MIDIBOOT_INVALID;
                     inSysexMode = false;
                  }
               }
               //increment the index
               inByteIndex++;
            }
         }
      }

      //advance the pointer
      bytequeue_remove((byteQueue_t *)&midiByteQueue, size);
   }

   return 0;
}
