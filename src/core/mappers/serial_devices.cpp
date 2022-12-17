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

serialDevice::serialDevice() {
	reset();
}
serialDevice::~serialDevice() {}

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

serialROM::serialROM(const BYTE *_rom) {
	bitPosition = 0;
	command = 0;
	rom = _rom;
}
serialROM::~serialROM() {}

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
	serialDevice::saveMapper(mode, slot, fp);
	save_slot_ele(mode, slot, bitPosition);
	save_slot_ele(mode, slot, command);

	return (EXIT_OK);
}

// ----------------------------------------------------------------------------------------

inverterROM::inverterROM() {
	command = 0;
	result = 0;
}
inverterROM::~inverterROM() {}

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
	serialDevice::saveMapper(mode, slot, fp);
	save_slot_ele(mode, slot, command);
	save_slot_ele(mode, slot, result);

	return (EXIT_OK);
}

// ----------------------------------------------------------------------------------------

GPIO_OneBus::GPIO_OneBus() {
	mask = 0;
	latch = 0;
	state = 0xFF;
}
GPIO_OneBus::~GPIO_OneBus() {}

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
		d.device->saveMapper(mode, slot, fp);
		save_slot_ele(mode, slot, d.select);
		save_slot_ele(mode, slot, d.clock);
		save_slot_ele(mode, slot, d.data);
	}
	save_slot_ele(mode, slot, mask);
	save_slot_ele(mode, slot, latch);
	save_slot_ele(mode, slot, state);

	return (EXIT_OK);
}
