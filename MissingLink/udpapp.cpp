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
#include "config.h"
#include "udpapp.h"
#include "uip-conf.h"
extern "C" {
#include "uip.h"
}
#include <string.h>

#include "MLSerial.h"
#include "OSCMessage.h"
#include "OSCDecoder.h"
#include "OSCEncoder.h"

static struct udpapp_state s;
static OSCDecoder decoder;
static struct uip_udp_conn* connOutgoing;

// This exists within the sketch for the user to modify.
extern void handleOSCMessage(OSCMessage& oscMess);

void dummy_app_appcall(void)
{
}

void udpapp_init(void)
{
   // Allocate our outgoing udp connection, we'll reuse this.
   connOutgoing = uip_udp_new(NULL, 0);

   // Allocate our incoming udp connection, we'll reuse this as well!
   struct uip_udp_conn *c = uip_udp_new(NULL, HTONS(0));
   if(c != NULL) {
      uip_udp_bind(c, HTONS(12344));
   }
   PT_INIT(&s.pt);
}

static unsigned char parse_msg(void)
{
#if 0
   for (int i = 0; i < uip_datalen(); i++)
      Serial.write(((char*)(uip_appdata))[i]);
   Serial.write("\n");
#endif

   OSCMessage m;
   decoder.decode(&m, (uint8_t*)(uip_appdata));
   u16_t* ip = uip_udp_conn->ripaddr;
   m.setIpAddress(ip[0] & 0xFF, ip[0] >> 8, ip[1] & 0xFF, ip[1] >> 8);
   m.setPortNumber(htons(uip_udp_conn->rport));

   // Here, we mark the connection as available for new remote ip/port combo notice that
   // we leave the local port alone so that uIP knows to keep listening.
   uip_udp_conn->ripaddr[0] = uip_udp_conn->ripaddr[1] = 0;
   uip_udp_conn->rport = 0;

   handleOSCMessage(m);

   return 1;
}

void sendAppBuffer(uint8_t len, uint16_t addr1, uint16_t addr2, uint16_t addr3, uint16_t addr4, uint16_t port)
{
   uip_ipaddr_t addr;
   uip_ipaddr(&addr, addr1, addr2, addr3, addr4);

   uip_ipaddr_copy(&connOutgoing->ripaddr, addr);
   connOutgoing->rport = HTONS(port);
   connOutgoing->ttl = UIP_TTL;

   uip_send(uip_appdata, len);
   uip_udp_conn = connOutgoing;
}

void bounceMessage(uint16_t addr1, uint16_t addr2, uint16_t addr3, uint16_t addr4, uint16_t port)
{
   uip_ipaddr_t addr;
   uip_ipaddr(&addr, addr1, addr2, addr3, addr4);

   uip_ipaddr_copy(&connOutgoing->ripaddr, addr);
   connOutgoing->rport = HTONS(port);
   connOutgoing->ttl = UIP_TTL;

   uip_send(uip_appdata, uip_len);
   uip_udp_conn = connOutgoing;
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

