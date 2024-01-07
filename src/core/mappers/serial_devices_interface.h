/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

typedef void *hserial;
typedef void *hgpio_onebus;
typedef void *heeprom_i2c;

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

// ----------------------------------------------------------------------------------------

EXTERNC heeprom_i2c eeprom_24c01_create(BYTE _deviceAddr, BYTE *_rom);
EXTERNC heeprom_i2c eeprom_24c02_create(BYTE _deviceAddr, BYTE *_rom);
EXTERNC heeprom_i2c eeprom_24c04_create(BYTE _deviceAddr, BYTE *_rom);
EXTERNC heeprom_i2c eeprom_24c08_create(BYTE _deviceAddr, BYTE *_rom);
EXTERNC heeprom_i2c eeprom_24c16_create(BYTE _deviceAddr, BYTE *_rom);

EXTERNC void eeprom_i2c_free(heeprom_i2c e);
EXTERNC void eeprom_i2c_reset(heeprom_i2c e);
EXTERNC BYTE eeprom_i2c_get_data(heeprom_i2c e);
EXTERNC void eeprom_i2c_set_pins(heeprom_i2c e, BYTE select, BYTE newClock, BYTE newData);
EXTERNC BYTE eeprom_i2c_save_mapper(heeprom_i2c e, BYTE mode, BYTE slot, FILE *fp);

#undef EXTERNC

#endif /* SERIAL_DEVICES_INTERFACE_H_ */
