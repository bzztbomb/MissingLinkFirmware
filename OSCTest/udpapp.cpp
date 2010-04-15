/*

Modified for OSC by Brian Richardson (skinny@knowhere.net)

*/

/******************************************************************************

  Filename:		udpapp.h
  Description:	UDP app for the WiShield 1.0

 ******************************************************************************

  TCP/IP stack and driver for the WiShield 1.0 wireless devices

  Copyright(c) 2009 Async Labs Inc. All rights reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  Contact Information:
  <asynclabs@asynclabs.com>

   Author               Date        Comment
  ---------------------------------------------------------------
   AsyncLabs			07/11/2009	Initial version

 *****************************************************************************/
extern "C" {
#include "uip.h"
}
#include <string.h>
#include "udpapp.h"
#include "config.h"
#include "HardwareSerial.h"
#include "OSCMessage.h"
#include "OSCDecoder.h"

static struct udpapp_state s;
static OSCDecoder decoder;

// This exists within the sketch for the user to modify.
extern void handleOSCMessage(OSCMessage& oscMess);

void dummy_app_appcall(void)
{
}

void udpapp_init(void)
{
   struct uip_udp_conn *c;

   c = uip_udp_new(NULL, HTONS(0));
   if(c != NULL) {
      uip_udp_bind(c, HTONS(12344));
   }
   PT_INIT(&s.pt);
}

static unsigned char parse_msg(void)
{
#if 0
    for (int i = 0; i < uip_datalen(); i++)
      Serial.print(((char*)(uip_appdata))[i]);
    Serial.println("");
#endif

    OSCMessage m;
    decoder.decode(&m, (uint8_t*)(uip_appdata));    
    handleOSCMessage(m);
    
    return 1;
}

static PT_THREAD(handle_connection(void))
{
   PT_BEGIN(&s.pt);

   // No state machine here now, we just stay in "Parse OSC Message" state.
   while (true) {
      PT_WAIT_UNTIL(&s.pt, uip_newdata());
      if(uip_newdata() && parse_msg()) {
         uip_flags &= (~UIP_NEWDATA);
      }
   } 

   PT_END(&s.pt);
}

void udpapp_appcall(void)
{
   handle_connection();
}
