/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

#include "serial_devices.hpp"

serialDevice::serialDevice() : state(0), clock(TRUE), data(TRUE), output(TRUE) {}
serialDevice::~serialDevice() = default;

void serialDevice::reset(void) {
	state = 0;
	clock = TRUE;
	data = TRUE;
	output = TRUE;
}
BYTE serialDevice::getData(void) const {
	return (output);
}
void serialDevice::setPins(BYTE select, BYTE newClock, BYTE newData) {
	if (!select) {
		clock = newClock;
		data = newData;
	}
}
BYTE serialDevice::saveMapper(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, state);
	save_slot_ele(mode, slot, clock);
	save_slot_ele(mode, slot, data);
	save_slot_ele(mode, slot, output);

	return (EXIT_OK);
}

// ----------------------------------------------------------------------------------------

// General I²C serial device
I2CDevice::I2CDevice(BYTE _deviceType, BYTE _deviceAddr) :
	serialDevice::serialDevice(), deviceType(_deviceType), deviceAddr(_deviceAddr), readMode(false) {}
I2CDevice::~I2CDevice() = default;

void I2CDevice::reset() {
	serialDevice::reset();
	readMode = FALSE;
}

void I2CDevice::setPins(UNUSED(BYTE select), BYTE newClock, BYTE newData) {
	if (clock && newClock && data && !newData) { // I²C start
		state = 1;
	} else if (clock && newClock && !data && newData) { // I²C stop
		state = 0;
	} else if (clock && !newClock) {
		receiveBit();
	}
	clock = newClock;
	data = newData;
}

void I2CDevice::receiveBit() {
	switch (state) {
		case 1:
			// Start condition
			state++;
			break;
		case 2:
			// 1st bit of device type
			state = !!(deviceType & 0x08) == data ? state + 1 : 0;
			break;
		case 3:
			// 2nd bit of device type
			state = !!(deviceType & 0x04) == data ? state + 1 : 0;
			break;
		case 4:
			// 3rd bit of device type
			state = !!(deviceType & 0x02) == data ? state + 1: 0;
			break;
		case 5:
			// 4th bit of device type
			state = !!(deviceType & 0x01) == data ? state + 1 : 0;
			break;
		case 6:
			// 1st bit of device address
			state = !!(deviceAddr & 0x04) == data ? state + 1 : 0;
			break;
		case 7:
			// 2nd bit of device address
			state = !!(deviceAddr & 0x02) == data ? state + 1 : 0;
			break;
		case 8:
			// 3rd bit of device address
			state = !!(deviceAddr & 0x01) == data ? state + 1 : 0;
			break;
		case 9:
			// Read/write
			readMode = data;
			state++;
			break;
		default:
			break;
	}
}

BYTE I2CDevice::saveMapper(BYTE mode, BYTE slot, FILE *fp) {
	if (serialDevice::saveMapper(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	save_slot_ele(mode, slot, readMode);

	return (EXIT_OK);
}

// ----------------------------------------------------------------------------------------

serialROM::serialROM(const BYTE *_rom) : bitPosition(0), command(0), rom(_rom) {}
serialROM::~serialROM() = default;

void serialROM::setPins(BYTE select, BYTE newClock, BYTE newData) {
	if (select) {
		state = 0;
	} else if (!clock && newClock) {
		if (state < 8) {
			command = command << 1 | newData * 1;
			if (++state == 8 && command != 0x30) {
				state = 0;
			} else {
				bitPosition = 0;
			}
		} else {
			output = rom[bitPosition >> 3] >> (7 - (bitPosition & 7)) & 1 ? TRUE : FALSE;
			if (++bitPosition >= 256 * 8) {
				state = 0;
			}
		}
	}
	clock = newClock;
}
BYTE serialROM::saveMapper(BYTE mode, BYTE slot, FILE *fp) {
	if (serialDevice::saveMapper(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	save_slot_ele(mode, slot, bitPosition);
	save_slot_ele(mode, slot, command);

	return (EXIT_OK);
}

// ----------------------------------------------------------------------------------------

inverterROM::inverterROM() : command(0), result(0) {}
inverterROM::~inverterROM() = default;

void inverterROM::setPins(UNUSED(BYTE select), BYTE newClock, BYTE newData) {
	if (clock && newClock && data && !newData) { // START in I²C
		state = 1;  // Write command and data
	} else if (clock && newClock && !data && newData) { // STOP in I²C
		state = 19; // Read result
	} else if (!clock && newClock) {
		if (state == 0) {
			data = newData;
		} else if (state >= 1 && state < 9) { // command byte
			command = command << 1 | newData * 1;
			if (++state == 9 && command != 0x80) {
				state = 0;
			}
		} else if (state == 9) {// terminating bit
			state++;
		} else if (state >= 10 && state < 18) { // data byte
			result = result << 1 | newData * 1;
			if (++state == 18) {
				result = ((-result >> 4) & 0xF) | ((-result << 4) & 0xF0);
			}
		} else if (state == 18) { // terminating bit
			state = 0;
		} else {
			output = (result & 0x80) != 0;
			result <<= 1;
		}
	}
	clock = newClock;
	data = newData;
}
BYTE inverterROM::saveMapper(BYTE mode, BYTE slot, FILE *fp) {
	if (serialDevice::saveMapper(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	save_slot_ele(mode, slot, command);
	save_slot_ele(mode, slot, result);

	return (EXIT_OK);
}

// ----------------------------------------------------------------------------------------

GPIO_OneBus::GPIO_OneBus() : mask(0), latch(0), state(0xFF) {}
GPIO_OneBus::~GPIO_OneBus() = default;

void GPIO_OneBus::reset(void) {
	mask = 0;
	state = 0xFF;
	serialDevices.clear();
}
BYTE GPIO_OneBus::read(BYTE address) {
	switch (address & 7) {
		case 0:
			return (mask);
		case 2:
			return (latch);
		case 3:
			state = ~mask;
			for (auto &d : serialDevices) {
				state &= d.device->getData() << d.data | ~(1 << d.data);
			}
			return (state);
		default:
			return (0xFF);
	}
}
void GPIO_OneBus::write(BYTE address, BYTE value) {
	switch (address & 7) {
		case 0:
			mask = value;
			state = (state & ~mask) | (latch & mask);
			for (auto &d : serialDevices) {
				d.device->setPins(state >> d.select & 1, state >> d.clock & 1, state >> d.data & 1);
			}
			break;
		case 2:
		case 3:
			latch = value;
			state = (state & ~mask) | (latch & mask);
			for (auto &d : serialDevices) {
				d.device->setPins(state >> d.select & 1, state >> d.clock & 1, state >> d.data & 1);
			}
			break;
	}
}
void GPIO_OneBus::attachSerialDevice(serialDevice *s, BYTE select, BYTE clock, BYTE data) {
	serialDevices.push_back({ s, select, clock, data });
}
BYTE GPIO_OneBus::saveMapper(BYTE mode, BYTE slot, FILE *fp) {
	for (auto &d : serialDevices) {
		if (d.device->saveMapper(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
		save_slot_ele(mode, slot, d.select);
		save_slot_ele(mode, slot, d.clock);
		save_slot_ele(mode, slot, d.data);
	}
	save_slot_ele(mode, slot, mask);
	save_slot_ele(mode, slot, latch);
	save_slot_ele(mode, slot, state);

	return (EXIT_OK);
}

// ----------------------------------------------------------------------------------------

// General I²C EEPROM device
EEPROM_I2C::EEPROM_I2C(WORD _addressMask, BYTE _deviceAddr, BYTE *_rom): I2CDevice::I2CDevice(0b1010, _deviceAddr),
	address(0), addressMask(_addressMask), rom(_rom), bit(0), latch(0) {}
EEPROM_I2C::~EEPROM_I2C() = default;

void EEPROM_I2C::reset() {
	I2CDevice::reset();
	address = 0;
	bit = 0;
	latch = 0;
}
void EEPROM_I2C::receiveBit() {
	switch (state) {
		default:
			// Initial states
			I2CDevice::receiveBit();
			break;
		case 10:
			// ACK state after address+mode/read transfer. Load latch in read mode.
			bit =0;
			if (readMode) {
				latch = rom[(address & addressMask)];
				state = 11;
			} else {
				latch = 0;
				state = 12;
			}
			break;
		case 11:
			// Read mode: return next bit, go back to state 10 after all bits were returned.
			if (++bit == 8) {
				address = (address & ~0xFF) | ((address + 1) & 0xFF);
				state--;
			}
			break;
		case 12:
			// Write mode: receive address byte LSB. MSB must be set by derived class, if applicable.
			latch |= (data ? (0x80 >> bit) : 0);
			if (++bit == 8) {
				address = (address & ~0xFF) | latch;
				state++;
			}
			break;
		case 13:
			// Write mode: ACK state after address load/write.
			bit = 0;
			latch = 0;
			state++;
			break;
		case 14:
			// Write mode: Transfer next bit, go back to state 14 after all bits were written.
			latch |= (data ? (0x80 >> bit) : 0);
			if (++bit == 8) {
				rom[address & addressMask] = latch;
				address = (address & ~0xFF) | ((address + 1) & 0xFF);
				state--;
			}
			break;
	}
	// Set data line based on (new) state
	switch (state) {
		case 10:
		case 13:
			// ACK
			output = FALSE;
			break;
		case 11:
			// Return bit in read mode
			output = !!(latch & (0x80 >> bit));
			break;
		default:
			// Default state is high
			output = TRUE;
			break;
	}
}

BYTE EEPROM_I2C::saveMapper(BYTE mode, BYTE slot, FILE *fp) {
	if (I2CDevice::saveMapper(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	save_slot_ele(mode, slot, address);
	save_slot_ele(mode, slot, bit);
	save_slot_ele(mode, slot, latch);

	return (EXIT_OK);
}

EEPROM_24C01::EEPROM_24C01(BYTE _deviceAddr, BYTE *_rom):
	EEPROM_I2C(0x07F, _deviceAddr, _rom) {
}
EEPROM_24C01::~EEPROM_24C01() = default;

void EEPROM_24C01::receiveBit() {
	switch (state) {
		// 24C01 uses a non-standard bit sequence, in which the device address itself is the address within the ROM.
		case 1:
			// Start condition
			bit = 0;
			latch = 0;
			state++;
			break;
		case 2:
			// Receive address bit.
			latch |= (data ? (0x01 << bit) : 0);
			if (++bit == 7) {
				address = latch;
				state++;
			}
			break;
		case 3:
			// Read/write mode
			readMode = data;
			++state;
			break;
		case 4:
			// ACK state after address+mode/read transfer. Load latch in read mode.
			bit = 0;
			if (readMode) {
				latch = rom[address & addressMask];
				state = 5;
			} else {
				latch = 0;
				state = 6;
			}
			break;
		case 5:
			// Read mode: return next bit, go back to state 4 after all bits were returned.
			if (++bit == 8) {
				address = (address + 1) & addressMask;
				state = 4;
			}
			break;
		case 6:
			// Write mode: Transfer next bit, go back to state 4 after all bits were written.
			latch |= (data ? (0x01 << bit) : 0);
			if (++bit == 8) {
				rom[address & addressMask] = latch;
				address = (address + 1) & addressMask;
				state = 4;
			}
			break;
	}
	// Set data line based on (new) state
	switch (state) {
		case 4:
			// ACK
			output = FALSE;
			break;
		case 5:
			// Return bit in read mode
			output = !!(latch & (0x01 << bit));
			break;
		default:
			// Default state is high
			output = TRUE;
			break;
	}
}


EEPROM_24C02::EEPROM_24C02(BYTE _deviceAddr, BYTE *_rom):
	EEPROM_I2C(0x0FF, _deviceAddr, _rom) {
}
EEPROM_24C02::~EEPROM_24C02() = default;

EEPROM_24C04::EEPROM_24C04(BYTE _deviceAddr, BYTE *_rom):
	EEPROM_I2C(0x1FF, _deviceAddr, _rom) {
}
EEPROM_24C04::~EEPROM_24C04() = default;

void EEPROM_24C04::receiveBit() {
	switch (state) {
		case 8:
			// 3rd bit of device address is replaced by address bit 8 to support 512 bytes
			address = (address & ~0x100) | (data * 0x100);
			state++;
			output = TRUE;
			break;
		default:
			// All other states
			EEPROM_I2C::receiveBit();
			break;
	}
}

EEPROM_24C08::EEPROM_24C08(BYTE _deviceAddr, BYTE *_rom):
	EEPROM_I2C(0x3FF, _deviceAddr, _rom) {
}
EEPROM_24C08::~EEPROM_24C08() = default;

void EEPROM_24C08::receiveBit() {
	switch (state) {
		case 7:
			// 2nd bit of device address is replaced by address bit 8 to support 512 bytes
			address = (address & ~0x200) | (data * 0x200);
			state++;
			output = TRUE;
			break;
		case 8:
			// 3rd bit of device address is replaced by address bit 8 to support 512 bytes
			address = (address & ~0x100) | (data * 0x100);
			state++;
			output = TRUE;
			break;
		default:
			// All other states
			EEPROM_I2C::receiveBit();
			break;
	}
}

EEPROM_24C16::EEPROM_24C16(BYTE _deviceAddr, BYTE *_rom):
	EEPROM_I2C(0x7FF, _deviceAddr, _rom) {
}
EEPROM_24C16::~EEPROM_24C16() = default;

void EEPROM_24C16::receiveBit() {
	switch (state) {
		case 6:
			// 1st bit of device address is replaced by address bit 8 to support 512 bytes
			address = (address & ~0x400) | (data * 0x400);
			state++;
			output = TRUE;
			break;
		case 7:
			// 2nd bit of device address is replaced by address bit 8 to support 512 bytes
			address = (address & ~0x200) | (data * 0x200);
			state++;
			output = TRUE;
			break;
		case 8:
			// 3rd bit of device address is replaced by address bit 8 to support 512 bytes
			address = (address & ~0x100) | (data * 0x100);
			state++;
			output = TRUE;
			break;
		default:
			// All other states
			EEPROM_I2C::receiveBit();
			break;
	}
}
