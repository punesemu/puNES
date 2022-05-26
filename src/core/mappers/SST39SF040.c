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

#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "save_slot.h"

enum _sst39sf040modes {
	SST39SF040_WAIT,
	SST39SF040_WRITE,
	SST39SF040_ERASE
};

struct _sst39sf040 {
	BYTE *data;
	size_t size;

	BYTE mode;
	BYTE sequence;
	BYTE software_id;
} sst39sf040;

void sst39sf040_init(BYTE *data, size_t size) {
	memset(&sst39sf040, 0x00, sizeof(sst39sf040));

	sst39sf040.data = data;
	sst39sf040.size = size;
	sst39sf040.mode = SST39SF040_WAIT;
	sst39sf040.software_id = FALSE;
}
void sst39sf040_reset() {
	sst39sf040.mode = SST39SF040_WAIT;
	sst39sf040.sequence = 0;
}
void sst39sf040_write(DBWORD address, BYTE value) {
	WORD cmd = address & 0x7FFF;

	if (sst39sf040.mode == SST39SF040_WAIT) {
		if (sst39sf040.sequence == 0) {
			if ((cmd == 0x5555) && (value == 0xAA)) {
				sst39sf040.sequence++;
			} else if (value == 0xF0) {
				sst39sf040_reset();
				sst39sf040.software_id = FALSE;
			}
		} else if ((sst39sf040.sequence == 1) && (cmd == 0x2AAA) && (value == 0x55)) {
			sst39sf040.sequence++;
		} else if ((sst39sf040.sequence == 2) && (cmd == 0x5555)) {
			sst39sf040.sequence++;
			switch (value) {
				case 0x80:
					sst39sf040.mode = SST39SF040_ERASE;
					break;
				case 0x90:
					sst39sf040_reset();
					sst39sf040.software_id = TRUE;
					break;
				case 0xA0:
					sst39sf040.mode = SST39SF040_WRITE;
					break;
				case 0xF0:
					sst39sf040_reset();
					sst39sf040.software_id = FALSE;
					break;
			}
		} else {
			sst39sf040.sequence = 0;
		}
	} else if (sst39sf040.mode == SST39SF040_WRITE) {
		if (address < sst39sf040.size) {
			sst39sf040.data[address] &= value;
		}
		sst39sf040_reset();
	} else if (sst39sf040.mode == SST39SF040_ERASE) {
		if (sst39sf040.sequence == 3) {
			if ((cmd == 0x5555) && (value == 0xAA)) {
				sst39sf040.sequence++;
			} else {
				sst39sf040_reset();
			}
		} else if (sst39sf040.sequence == 4) {
			if ((cmd == 0x2AAA) && (value == 0x55)) {
				sst39sf040.sequence++;
			} else {
				sst39sf040_reset();
			}
		} else if (sst39sf040.sequence == 5) {
			if ((cmd == 0x5555) && (value == 0x10)) {
				memset(sst39sf040.data, 0xFF, sst39sf040.size);
			} else if (value == 0x30) {
				DBWORD offset = (address & 0x7F000);

				if ((offset + 0x1000) <= sst39sf040.size) {
					memset(sst39sf040.data + offset, 0xFF, 0x1000);
				}
			}
			sst39sf040_reset();
		}
	}
}
BYTE sst39sf040_read(DBWORD address) {
	if (sst39sf040.software_id) {
		switch (address & 0x1FF) {
			case 0x00:
				return (0xBF);
			case 0x01:
				return (0xB7);
			default:
				return (0xFF);
		}
	}
	return (sst39sf040.data[address]);
}
