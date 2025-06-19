/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

// ----------------------------------------------------------------------------------------

EXTERNC heeprom_i2c eeprom_24c01_create(BYTE _deviceAddr, BYTE *_rom) {
	return ((EEPROM_24C01 *)new EEPROM_24C01(_deviceAddr, _rom));
}
EXTERNC heeprom_i2c eeprom_24c02_create(BYTE _deviceAddr, BYTE *_rom) {
	return ((EEPROM_24C02 *)new EEPROM_24C02(_deviceAddr, _rom));
}
EXTERNC heeprom_i2c eeprom_24c04_create(BYTE _deviceAddr, BYTE *_rom) {
	return ((EEPROM_24C02 *)new EEPROM_24C04(_deviceAddr, _rom));
}
EXTERNC heeprom_i2c eeprom_24c08_create(BYTE _deviceAddr, BYTE *_rom) {
	return ((EEPROM_24C08 *)new EEPROM_24C08(_deviceAddr, _rom));
}
EXTERNC heeprom_i2c eeprom_24c16_create(BYTE _deviceAddr, BYTE *_rom) {
	return ((EEPROM_24C16 *)new EEPROM_24C16(_deviceAddr, _rom));
}

EXTERNC void eeprom_i2c_free(heeprom_i2c e) {
	delete (EEPROM_I2C *)e;
}
EXTERNC void eeprom_i2c_reset(heeprom_i2c e) {
	((EEPROM_I2C *)e)->reset();
}
EXTERNC BYTE eeprom_i2c_get_data(heeprom_i2c e) {
	return (((EEPROM_I2C *)e)->getData());
}
EXTERNC void eeprom_i2c_set_pins(heeprom_i2c e, BYTE select, BYTE newClock, BYTE newData) {
	((EEPROM_I2C *)e)->setPins(select, newClock, newData);
}
EXTERNC BYTE eeprom_i2c_save_mapper(heeprom_i2c e, BYTE mode, BYTE slot, FILE *fp) {
	return (((EEPROM_I2C *)e)->saveMapper(mode, slot, fp));
}

#undef EXTERNC
