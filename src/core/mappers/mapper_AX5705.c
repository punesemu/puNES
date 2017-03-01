/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

#define _ax5705_chr_rom_1k_update(slot)\
	control_bank(info.chr.rom[0].max.banks_1k)\
	chr.bank_1k[slot] = chr_chip_byte_pnt(0, value << 10);\
	ax5705.chr_map[slot] = value
#define ax5705_chr_rom_1k_update_high(slot)\
	value = (ax5705.chr_map[slot] & 0x0F) | ((((value & 0x04) >> 1) | ((value & 0x02) << 1) | (value & 0x09)) << 4);\
	_ax5705_chr_rom_1k_update(slot)
#define ax5705_chr_rom_1k_update_low(slot)\
	value = (ax5705.chr_map[slot] & 0xF0) | (value & 0x0F);\
	_ax5705_chr_rom_1k_update(slot)

void map_init_AX5705(void) {
	EXTCL_CPU_WR_MEM(AX5705);
	EXTCL_SAVE_MAPPER(AX5705);
	mapper.internal_struct[0] = (BYTE *) &ax5705;
	mapper.internal_struct_size[0] = sizeof(ax5705);

	if (info.reset >= HARD) {
		BYTE i;

		memset(&ax5705, 0x00, sizeof(ax5705));
		for (i = 0; i < 8; i++) {
			ax5705.chr_map[i] = i;
		}
	}
}
void extcl_cpu_wr_mem_AX5705(WORD address, BYTE value) {
	switch (address & 0xF00F) {
		case 0x8000:
			value = ((value & 0x02) << 2) | ((value & 0x08) >> 2) | (value & 0x05);
			control_bank(info.prg.rom[0].max.banks_8k)
			map_prg_rom_8k(1, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x8008:
			if (value & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			return;
		case 0xA000:
			value = ((value & 0x02) << 2) | ((value & 0x08) >> 2) | (value & 0x05);
			control_bank(info.prg.rom[0].max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			map_prg_rom_8k_update();
			return;
		case 0xA008:
			ax5705_chr_rom_1k_update_low(0);
			return;
		case 0xA009:
			ax5705_chr_rom_1k_update_high(0);
			return;
		case 0xA00A:
			ax5705_chr_rom_1k_update_low(1);
			return;
		case 0xA00B:
			ax5705_chr_rom_1k_update_high(1);
			return;
		case 0xC000:
			ax5705_chr_rom_1k_update_low(2);
			return;
		case 0xC001:
			ax5705_chr_rom_1k_update_high(2);
			return;
		case 0xC002:
			ax5705_chr_rom_1k_update_low(3);
			return;
		case 0xC003:
			ax5705_chr_rom_1k_update_high(3);
			return;
		case 0xC008:
			ax5705_chr_rom_1k_update_low(4);
			return;
		case 0xC009:
			ax5705_chr_rom_1k_update_high(4);
			return;
		case 0xC00A:
			ax5705_chr_rom_1k_update_low(5);
			return;
		case 0xC00B:
			ax5705_chr_rom_1k_update_high(5);
			return;
		case 0xE000:
			ax5705_chr_rom_1k_update_low(6);
			return;
		case 0xE001:
			ax5705_chr_rom_1k_update_high(6);
			return;
		case 0xE002:
			ax5705_chr_rom_1k_update_low(7);
			return;
		case 0xE003:
			ax5705_chr_rom_1k_update_high(7);
			return;
	}
}
BYTE extcl_save_mapper_AX5705(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, ax5705.chr_map);

	return (EXIT_OK);
}
