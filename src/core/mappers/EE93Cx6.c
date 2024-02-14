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

#include <stddef.h>
#include "EE93Cx6.h"

enum ee93cx6_opcodes {
	Opcode_misc = 0,
	Opcode_write = 1,
	Opcode_read = 2,
	Opcode_erase = 3,
	Opcode_writeDisable = 10,
	Opcode_writeAll = 11,
	Opcode_eraseAll = 12,
	Opcode_writeEnable = 13
};
enum ee93cx6_states {
	State_standBy = 0,
	State_startBit = 1,
	State_opcode = 3,
	State_address8 = 12,
	State_data8 = 20,
	State_address16 = 11,
	State_data16 = 27,
	State_finished = 99
};

struct _ee93cx6 {
	size_t capacity;
	BYTE *storage;
	BYTE opcode;
	WORD data;
	WORD address;
	BYTE state;
	BYTE lastCLK;
	BYTE writeEnabled;
	BYTE output;
	BYTE wordSize;
	BYTE State_address;
	BYTE State_data;
} ee93cx6;

void ee93cx6_init(BYTE *buffer, size_t _capacity, BYTE _wordSize) {
	ee93cx6.capacity = _capacity;
	ee93cx6.storage = buffer;
	ee93cx6.address = 0;
	ee93cx6.state = State_standBy;
	ee93cx6.lastCLK = FALSE;
	ee93cx6.writeEnabled = FALSE;
	ee93cx6.wordSize = _wordSize;

	ee93cx6.State_address = ee93cx6.wordSize == 16 ? State_address16 : State_address8;
	ee93cx6.State_data = ee93cx6.wordSize == 16 ? State_data16 : State_data8;
}

void ee93cx6_write(BYTE CS, BYTE CLK, BYTE DAT) {
	if (!CS && (ee93cx6.state <= ee93cx6.State_address)) {
		ee93cx6.state = State_standBy;
	} else if ((ee93cx6.state == State_standBy) && CS && CLK && !ee93cx6.lastCLK) {
		ee93cx6.state = State_startBit;
		ee93cx6.opcode = 0;
		ee93cx6.address = 0;
		ee93cx6.output = TRUE;
	} else if (CLK && !ee93cx6.lastCLK) {
		if ((ee93cx6.state >= State_startBit) && (ee93cx6.state < State_opcode)) {
			ee93cx6.opcode = (ee93cx6.opcode << 1) | (DAT * 1);
		} else if ((ee93cx6.state >= State_opcode) && (ee93cx6.state < ee93cx6.State_address)) {
			ee93cx6.address = (ee93cx6.address << 1) | (DAT * 1);
		} else if ((ee93cx6.state >= ee93cx6.State_address) && (ee93cx6.state < ee93cx6.State_data)) {
			if ((ee93cx6.opcode == Opcode_write) || (ee93cx6.opcode == Opcode_writeAll)) {
				ee93cx6.data = (ee93cx6.data << 1) | (DAT * 1);
			} else if (ee93cx6.opcode == Opcode_read) {
				if (ee93cx6.wordSize == 16) {
					ee93cx6.output = (ee93cx6.data & 0x8000) != 0;
				} else {
					ee93cx6.output = (ee93cx6.data & 0x80) != 0;
				}
				ee93cx6.data = ee93cx6.data << 1;
			}
		}

		ee93cx6.state++;
		if (ee93cx6.state == ee93cx6.State_address) {
			switch (ee93cx6.opcode) {
				case Opcode_misc:
					ee93cx6.opcode = (ee93cx6.address >> (ee93cx6.wordSize == 16 ? 6 : 7)) + 10;
					switch (ee93cx6.opcode) {
						case Opcode_writeDisable:
							ee93cx6.writeEnabled = FALSE;
							ee93cx6.state = State_finished;
							break;
						case Opcode_writeEnable:
							ee93cx6.writeEnabled = TRUE;
							ee93cx6.state = State_finished;
							break;
						case Opcode_eraseAll:
							if (ee93cx6.writeEnabled) {
								size_t i;

								for (i = 0; i < ee93cx6.capacity; i++) {
									ee93cx6.storage[i] = 0xFF;
								}
							}
							ee93cx6.state = State_finished;
							break;
						case Opcode_writeAll:
							ee93cx6.address = 0;
							break;
					}
					break;
				case Opcode_erase:
					if (ee93cx6.writeEnabled) {
						if (ee93cx6.wordSize == 16) {
							ee93cx6.storage[(ee93cx6.address << 1) | 0] = 0xFF;
							ee93cx6.storage[(ee93cx6.address << 1) | 1] = 0xFF;
						} else {
							ee93cx6.storage[ee93cx6.address] = 0xFF;
						}
					}
					ee93cx6.state = State_finished;
					break;
				case Opcode_read:
					if (ee93cx6.wordSize == 16) {
						ee93cx6.data = ee93cx6.storage[(ee93cx6.address << 1) | 0] | ee93cx6.storage[(ee93cx6.address << 1) | 1] << 8;
						ee93cx6.address++;
					} else {
						ee93cx6.data = ee93cx6.storage[ee93cx6.address++];
					}
					break;
			}
		} else if (ee93cx6.state == ee93cx6.State_data) {
			if (ee93cx6.opcode == Opcode_write) {
				if (ee93cx6.wordSize == 16) {
					ee93cx6.storage[(ee93cx6.address << 1) | 0] = ee93cx6.data & 0xFF;
					ee93cx6.storage[(ee93cx6.address << 1) | 1] = ee93cx6.data >> 8;
					ee93cx6.address++;
				} else {
					ee93cx6.storage[ee93cx6.address++] = ee93cx6.data;
				}
				ee93cx6.state = State_finished;
			} else if (ee93cx6.opcode == Opcode_writeAll) {
				if (ee93cx6.wordSize == 16) {
					ee93cx6.storage[(ee93cx6.address << 1) | 0] = ee93cx6.data & 0xFF;
					ee93cx6.storage[(ee93cx6.address << 1) | 1] = ee93cx6.data >> 8;
					ee93cx6.address++;
				} else {
					ee93cx6.storage[ee93cx6.address++] = ee93cx6.data;
				}
				ee93cx6.state = (CS && (ee93cx6.address < ee93cx6.capacity)) ? ee93cx6.State_address : State_finished;
			} else if (ee93cx6.opcode == Opcode_read) {
				if (ee93cx6.address < ee93cx6.capacity) {
					if (ee93cx6.wordSize == 16) {
						ee93cx6.data = ee93cx6.storage[(ee93cx6.address << 1) | 0] | ee93cx6.storage[(ee93cx6.address << 1) | 1] << 8;
					} else {
						ee93cx6.data = ee93cx6.storage[ee93cx6.address];
					}
				}
				ee93cx6.state = (CS && (++ee93cx6.address <= ee93cx6.capacity)) ? ee93cx6.State_address : State_finished;
			}
		}
		if (ee93cx6.state == State_finished) {
			ee93cx6.output = FALSE;
			ee93cx6.state = State_standBy;
		}
	}

	ee93cx6.lastCLK = CLK;
}

BYTE ee93cx6_read(void) {
	return (ee93cx6.output);
}

/*
int MAPINT EEPROM_93Cx6::saveLoad (STATE_TYPE stateMode, int offset, unsigned char *stateData) {
	SAVELOAD_BYTE(stateMode, offset, stateData, opcode);
	SAVELOAD_WORD(stateMode, offset, stateData, data);
	SAVELOAD_WORD(stateMode, offset, stateData, address);
	SAVELOAD_BOOL(stateMode, offset, stateData, lastCLK);
	SAVELOAD_BOOL(stateMode, offset, stateData, writeEnabled);
	SAVELOAD_BOOL(stateMode, offset, stateData, output);
	return offset;
}
*/

