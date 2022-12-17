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

#include <string.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

INLINE static void ks7057_update(void);

struct _ks7057 {
	BYTE reg[8];
} ks7057;
struct _ks7057tmp {
	BYTE *prg_6000;
	BYTE *prg_6800;
	BYTE *prg_7000;
	BYTE *prg_7800;
	BYTE *prg_8000;
	BYTE *prg_8800;
	BYTE *prg_9000;
	BYTE *prg_9800;
} ks7057tmp;

void map_init_KS7057(void) {
	EXTCL_CPU_WR_MEM(KS7057);
	EXTCL_CPU_RD_MEM(KS7057);
	EXTCL_SAVE_MAPPER(KS7057);
	mapper.internal_struct[0] = (BYTE *)&ks7057;
	mapper.internal_struct_size[0] = sizeof(ks7057);

	memset(&ks7057, 0x00, sizeof(ks7057));

	info.mapper.extend_rd = TRUE;

	ks7057_update();
}
void extcl_cpu_wr_mem_KS7057(WORD address, BYTE value) {
	switch (address & 0xF003) {
		case 0x8000:
		case 0x8001:
		case 0x8002:
		case 0x8003:
		case 0x9000:
		case 0x9001:
		case 0x9002:
		case 0x9003:
			if (value & 0x01) {
				mirroring_V();
			} else {
				mirroring_H();
			}
			return;
		case 0xB000:
			ks7057.reg[0] = (ks7057.reg[0] & 0xF0) | (value & 0x0F);
			break;
		case 0xB001:
			ks7057.reg[0] = (ks7057.reg[0] & 0x0F) | (value << 4);
			break;
		case 0xB002:
			ks7057.reg[1] = (ks7057.reg[1] & 0xF0) | (value & 0x0F);
			break;
		case 0xB003:
			ks7057.reg[1] = (ks7057.reg[1] & 0x0F) | (value << 4);
			break;
		case 0xC000:
			ks7057.reg[2] = (ks7057.reg[2] & 0xF0) | (value & 0x0F);
			break;
		case 0xC001:
			ks7057.reg[2] = (ks7057.reg[2] & 0x0F) | (value << 4);
			break;
		case 0xC002:
			ks7057.reg[3] = (ks7057.reg[3] & 0xF0) | (value & 0x0F);
			break;
		case 0xC003:
			ks7057.reg[3] = (ks7057.reg[3] & 0x0F) | (value << 4);
			break;
		case 0xD000:
			ks7057.reg[4] = (ks7057.reg[4] & 0xF0) | (value & 0x0F);
			break;
		case 0xD001:
			ks7057.reg[4] = (ks7057.reg[4] & 0x0F) | (value << 4);
			break;
		case 0xD002:
			ks7057.reg[5] = (ks7057.reg[5] & 0xF0) | (value & 0x0F);
			break;
		case 0xD003:
			ks7057.reg[5] = (ks7057.reg[5] & 0x0F) | (value << 4);
			break;
		case 0xE000:
			ks7057.reg[6] = (ks7057.reg[6] & 0xF0) | (value & 0x0F);
			break;
		case 0xE001:
			ks7057.reg[6] = (ks7057.reg[6] & 0x0F) | (value << 4);
			break;
		case 0xE002:
			ks7057.reg[7] = (ks7057.reg[7] & 0xF0) | (value & 0x0F);
			break;
		case 0xE003:
			ks7057.reg[7] = (ks7057.reg[7] & 0x0F) | (value << 4);
			break;
		case 0xF000:
		case 0xF001:
		case 0xF002:
		case 0xF003:
			return;
	}
	ks7057_update();
}
BYTE extcl_cpu_rd_mem_KS7057(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xF800) {
		case 0x6000:
			return (ks7057tmp.prg_6000[address & 0x07FF]);
		case 0x6800:
			return (ks7057tmp.prg_6800[address & 0x07FF]);
		case 0x7000:
			return (ks7057tmp.prg_7000[address & 0x07FF]);
		case 0x7800:
			return (ks7057tmp.prg_7800[address & 0x07FF]);
		case 0x8000:
			return (ks7057tmp.prg_8000[address & 0x07FF]);
		case 0x8800:
			return (ks7057tmp.prg_8800[address & 0x07FF]);
		case 0x9000:
			return (ks7057tmp.prg_9000[address & 0x07FF]);
		case 0x9800:
			return (ks7057tmp.prg_9800[address & 0x07FF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_KS7057(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, ks7057.reg);

	if (mode == SAVE_SLOT_READ) {
		ks7057_update();
	}

	return (EXIT_OK);
}

INLINE static void ks7057_update(void) {
	WORD value;

	// 0x6000
	value = ks7057.reg[4];
	control_bank(info.prg.rom.max.banks_2k)
	ks7057tmp.prg_6000 = prg_pnt(value << 11);
	// 0x6800
	value = ks7057.reg[5];
	control_bank(info.prg.rom.max.banks_2k)
	ks7057tmp.prg_6800 = prg_pnt(value << 11);

	// 0x7000
	value = ks7057.reg[6];
	control_bank(info.prg.rom.max.banks_2k)
	ks7057tmp.prg_7000 = prg_pnt(value << 11);
	// 0x7800
	value = ks7057.reg[7];
	control_bank(info.prg.rom.max.banks_2k)
	ks7057tmp.prg_7800 = prg_pnt(value << 11);

	// 0x8000
	value = ks7057.reg[0];
	control_bank(info.prg.rom.max.banks_2k)
	ks7057tmp.prg_8000 = prg_pnt(value << 11);
	// 0x8800
	value = ks7057.reg[1];
	control_bank(info.prg.rom.max.banks_2k)
	ks7057tmp.prg_8800 = prg_pnt(value << 11);

	// 0x9000
	value = ks7057.reg[2];
	control_bank(info.prg.rom.max.banks_2k)
	ks7057tmp.prg_9000 = prg_pnt(value << 11);
	// 0x9800
	value = ks7057.reg[3];
	control_bank(info.prg.rom.max.banks_2k)
	ks7057tmp.prg_9800 = prg_pnt(value << 11);

	// 0xA000 - 0xB000
	value = 0x0D;
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, value);
	prg.rom_8k[1] = prg_pnt(mapper.rom_map_to[1] << 13);

	// 0xC000 - 0xD000
	// 0xE000 - 0xF000
	value = 7;
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, value);
	prg.rom_8k[2] = prg_pnt(mapper.rom_map_to[2] << 13);
	prg.rom_8k[3] = prg_pnt(mapper.rom_map_to[3] << 13);
}
