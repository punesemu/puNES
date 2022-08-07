/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef SERIAL_DEVICES_INTERFACE_H_
#define SERIAL_DEVICES_INTERFACE_H_

#include <stdio.h>
#include "common.h"

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

typedef void* hgpio_onebus;
typedef void* hserial;

// ----------------------------------------------------------------------------------------

EXTERNC hserial serial_device_create(void);
EXTERNC void serial_device_free(hserial s);
EXTERNC void serial_device_reset(hserial s);
EXTERNC BYTE serial_device_get_data(hserial s);
EXTERNC void serial_device_set_pins(hserial s, BYTE select, BYTE newClock, BYTE newData);
EXTERNC BYTE serial_device_save_mapper(hserial s, BYTE mode, BYTE slot, FILE *fp);

// ----------------------------------------------------------------------------------------

EXTERNC hgpio_onebus gpio_onebus_create(void);
EXTERNC void gpio_onebus_free(hgpio_onebus g);
EXTERNC void gpio_onebus_reset(hgpio_onebus g);
EXTERNC void gpio_onebus_attach_serial_device(hgpio_onebus g, hserial s, BYTE select, BYTE clock, BYTE data);
EXTERNC BYTE gpio_onebus_read(hgpio_onebus g, BYTE address);
EXTERNC void gpio_onebus_write(hgpio_onebus g, BYTE address, BYTE value);
EXTERNC BYTE gpio_onebus_save_mapper(hgpio_onebus g, BYTE mode, BYTE slot, FILE *fp);

#undef EXTERNC

#endif /* SERIAL_DEVICES_INTERFACE_H_ */
