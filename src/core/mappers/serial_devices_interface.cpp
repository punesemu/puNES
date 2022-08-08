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

#include "serial_devices_interface.h"
#include "serial_devices.hpp"

#define EXTERNC extern "C"

// ----------------------------------------------------------------------------------------

EXTERNC hserial serial_device_create(void) {
	return ((serialDevice *)new serialDevice());
}
EXTERNC void serial_device_free(hserial s) {
	delete (serialDevice *)s;
}
EXTERNC void serial_device_reset(hserial s) {
	((serialDevice *)s)->reset();
}
EXTERNC BYTE serial_device_get_data(hserial s) {
	return (((serialDevice *)s)->getData());
}
EXTERNC void serial_device_set_pins(hserial s, BYTE select, BYTE newClock, BYTE newData) {
	((serialDevice *)s)->setPins(select, newClock, newData);
}
EXTERNC BYTE serial_device_save_mapper(hserial s, BYTE mode, BYTE slot, FILE *fp) {
	return (((serialDevice *)s)->saveMapper(mode, slot, fp));
}

// ----------------------------------------------------------------------------------------

EXTERNC hgpio_onebus gpio_onebus_create(void) {
	return ((GPIO_OneBus *)new GPIO_OneBus());
}
EXTERNC void gpio_onebus_free(hgpio_onebus g) {
	delete (GPIO_OneBus *)g;
}
EXTERNC void gpio_onebus_reset(hgpio_onebus g) {
	((GPIO_OneBus *)g)->reset();
}
EXTERNC void gpio_onebus_attach_serial_device(hgpio_onebus g, hserial s, BYTE select, BYTE clock, BYTE data) {
	((GPIO_OneBus *)g)->attachSerialDevice((serialDevice *)s, select, clock, data);
}
EXTERNC BYTE gpio_onebus_read(hgpio_onebus g, BYTE address) {
	return (((GPIO_OneBus *)g)->read(address));
}
EXTERNC void gpio_onebus_write(hgpio_onebus g, BYTE address, BYTE value) {
	((GPIO_OneBus *)g)->write(address, value);
}
EXTERNC BYTE gpio_onebus_save_mapper(hgpio_onebus g, BYTE mode, BYTE slot, FILE *fp) {
	return (((GPIO_OneBus *)g)->saveMapper(mode, slot, fp));
}

#undef EXTERNC
