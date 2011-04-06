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
#ifndef _LED_H
#define _LED_H

#define LED_OFF { PORTC &= ~(_BV(2) | _BV(3)); }
#define LED_GREEN { LED_OFF; PORTC |= _BV(2); }
#define LED_RED { LED_OFF; PORTC |= _BV(3); }
#define LED_YELLOW { LED_OFF; PORTC |= _BV(2) | _BV(3); }

bool canBlink();

#endif
