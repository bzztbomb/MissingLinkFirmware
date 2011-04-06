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
#ifndef _UDPAPP_ML_H_
#define _UDPAPP_ML_H_

void sendAppBuffer(uint8_t len, uint16_t addr1, uint16_t addr2, uint16_t addr3, uint16_t addr4, uint16_t port);
void bounceMessage(uint16_t addr1, uint16_t addr2, uint16_t addr3, uint16_t addr4, uint16_t port);

#endif
