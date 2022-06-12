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

INLINE static DBWORD wr_address_sst39sf040(WORD address);
INLINE static DBWORD rd_address_sst39sf040(WORD address);

struct _sst39sf040 {
	BYTE sequence;
	int32_t time_out;
} sst39sf040;
struct _sst39sf040tmp {
	BYTE *data;
	size_t size;
	BYTE manufacturer_id;
	BYTE model_id;
	WORD address1;
	WORD address2;
	size_t sector_size;
} sst39sf040tmp;

void sst39sf040_init(BYTE *data, size_t size, BYTE manufacter_id, BYTE model_id, WORD adr1, WORD adr2, int sector_size) {
	memset(&sst39sf040, 0x00, sizeof(sst39sf040));

	sst39sf040tmp.data = data;
	sst39sf040tmp.size = size;
	sst39sf040tmp.manufacturer_id = manufacter_id;
	sst39sf040tmp.model_id = model_id;
	sst39sf040tmp.address1 = adr1;
	sst39sf040tmp.address2 = adr2;
	sst39sf040tmp.sector_size = sector_size;
}
BYTE sst39sf040_read(WORD address) {
	if (sst39sf040.sequence == 0x90) {
		return (address & 0x0001 ? sst39sf040tmp.model_id : sst39sf040tmp.manufacturer_id);
	} else if (sst39sf040.time_out) {
		return ((sst39sf040tmp.data[rd_address_sst39sf040(address)] ^ ((sst39sf040.time_out & 0x01) << 6)) & 0x77);
	}
	return sst39sf040tmp.data[rd_address_sst39sf040(address)];
}
void sst39sf040_write(WORD address, BYTE value) {
	DBWORD chip_address = wr_address_sst39sf040(address);
	WORD cmd = address & 0x7FFF;
	size_t i;

	switch (sst39sf040.sequence) {
		default:
		case 0x00:
		case 0x80:
			if ((cmd == sst39sf040tmp.address1) && (value == 0xAA)) {
				sst39sf040.sequence++;
			}
			break;
		case 0x01:
		case 0x81:
			if ((cmd == sst39sf040tmp.address2) && (value == 0x55)) {
				sst39sf040.sequence++;
			}
			break;
		case 0x02:
			if (cmd == sst39sf040tmp.address1) {
				sst39sf040.sequence = value;
			}
			break;
		case 0x82:
			if (value == 0x30) {
				// cancellazione settore
				if (chip_address < sst39sf040tmp.size) {
					chip_address &= ~(sst39sf040tmp.sector_size - 1);
					for (i = 0; i < sst39sf040tmp.sector_size; i++) {
						sst39sf040tmp.data[chip_address + i] = 0xFF;
					}
					sst39sf040.time_out = sst39sf040tmp.sector_size;
				}
			} else if ((cmd == sst39sf040tmp.address1) && (value == 0x10)) {
				// cancellazione chip
				for (i = 0; i <= sst39sf040tmp.size; i++) {
					sst39sf040tmp.data[i] = 0xFF;
				}
				sst39sf040.time_out = sst39sf040tmp.size;
			}
			break;
		case 0x90:
			// software id
			if (value == 0xF0) {
				sst39sf040.sequence = 0;
			}
			break;
		case 0xA0:
			// salvataggio byte
			sst39sf040tmp.data[chip_address] = value;
			sst39sf040.sequence = 0;
			break;
	}
}
void sst39sf040_tick(void) {
	if (sst39sf040.time_out && !--sst39sf040.time_out) {
		sst39sf040.sequence = 0;
	}
}
BYTE sst39sf040_save_mapper(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, sst39sf040.sequence);
	save_slot_ele(mode, slot, sst39sf040.time_out);

	return (EXIT_OK);
}

INLINE static DBWORD wr_address_sst39sf040(WORD address) {
	return ((mapper.rom_map_to[(address >> 13) & 0x03] << 13) | (address & 0x1FFF));
}
INLINE static DBWORD rd_address_sst39sf040(WORD address) {
	return ((mapper.rom_map_to[(address >> 13) & 0x03] << 13) | (address & 0x1FFF));
}
