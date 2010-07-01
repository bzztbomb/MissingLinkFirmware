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
#include "OSCEncoder.h"

static OSCEncoder encoder;

void dummy_app_appcall(void)
{
}

void udpapp_init(void)
{
}

void udpapp_appcall(void)
{
}

void sendOSCMessage(OSCMessage& m, uint8 addr1, uint8 addr2, uint8 addr3, uint8 addr4, uint8 port)
{
   uip_ipaddr_t addr;
   struct uip_udp_conn *c;

   uip_ipaddr(&addr, addr1, addr2, addr3, addr4);
   c = uip_udp_new(&addr, HTONS(port));
   uint16_t len = encoder.encode(&m, (uint8_t*) uip_appdata);
   uip_send(uip_appdata, len);   
   uip_close(); // Is this needed?  UDP is connectionless, or we might be closing the connection before the data gets sent out.. 
}
