/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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
#include "cpu.h"
#include "mem_map.h"
#include "save_slot.h"

#define cityfight_chr_rom_1k(slot, vl)\
	value = vl;\
	control_bank(info.chr.rom[0].max.banks_1k)\
	chr.bank_1k[slot] = chr_chip_byte_pnt(0, value << 10);\
	cityfight.chr_map[slot] = value

INLINE static void cityfight_prg_update(void);

void map_init_CITYFIGHT(void) {
	EXTCL_CPU_WR_MEM(CITYFIGHT);
	EXTCL_SAVE_MAPPER(CITYFIGHT);
	EXTCL_CPU_EVERY_CYCLE(CITYFIGHT);
	mapper.internal_struct[0] = (BYTE *) &cityfight;
	mapper.internal_struct_size[0] = sizeof(cityfight);

	memset(&cityfight, 0x00, sizeof(cityfight));

	cityfight_prg_update();
}
void extcl_cpu_wr_mem_CITYFIGHT(WORD address, BYTE value) {
	switch (address & 0xF00C) {
		case 0x9000:
			switch (value & 0x03) {
				case 0:
					mirroring_V();
					return;
				case 1:
					mirroring_H();
					return;
				case 2:
					mirroring_SCR0();
					return;
				case 3:
					mirroring_SCR1();
					return;
			}
			cityfight.reg[1] = value & 0x0C;
			cityfight_prg_update();
			return;;
		case 0x9004:
		case 0x9008:
		case 0x900C:
			if (address & 0x0800) {
				cpu_wr_mem(0x4011, (value & 0x0F) << 3);
			} else {
				cityfight.reg[1] = value & 0x0C;
				cityfight_prg_update();
			}
			return;;
		case 0xC000:
		case 0xC004:
		case 0xC008:
		case 0xC00C:
			cityfight.reg[0] = value & 0x01;
			cityfight_prg_update();
			return;
		case 0xD000:
			cityfight_chr_rom_1k(0, (cityfight.chr_map[0] & 0xF0) | (value & 0x0F));
			return;
		case 0xD004:
			cityfight_chr_rom_1k(0, (cityfight.chr_map[0] & 0x0F) | (value << 4));
			return;
		case 0xD008:
			cityfight_chr_rom_1k(1, (cityfight.chr_map[1] & 0xF0) | (value & 0x0F));
			return;
		case 0xD00C:
			cityfight_chr_rom_1k(1, (cityfight.chr_map[1] & 0x0F) | (value << 4));
			return;
		case 0xA000:
			cityfight_chr_rom_1k(2, (cityfight.chr_map[2] & 0xF0) | (value & 0x0F));
			return;
		case 0xA004:
			cityfight_chr_rom_1k(2, (cityfight.chr_map[2] & 0x0F) | (value << 4));
			return;
		case 0xA008:
			cityfight_chr_rom_1k(3, (cityfight.chr_map[3] & 0xF0) | (value & 0x0F));
			return;
		case 0xA00C:
			cityfight_chr_rom_1k(3, (cityfight.chr_map[3] & 0x0F) | (value << 4));
			return;
		case 0xB000:
			cityfight_chr_rom_1k(4, (cityfight.chr_map[4] & 0xF0) | (value & 0x0F));
			return;
		case 0xB004:
			cityfight_chr_rom_1k(4, (cityfight.chr_map[4] & 0x0F) | (value << 4));
			return;
		case 0xB008:
			cityfight_chr_rom_1k(5, (cityfight.chr_map[5] & 0xF0) | (value & 0x0F));
			return;
		case 0xB00C:
			cityfight_chr_rom_1k(5, (cityfight.chr_map[5] & 0x0F) | (value << 4));
			return;
		case 0xE000:
			cityfight_chr_rom_1k(6, (cityfight.chr_map[6] & 0xF0) | (value & 0x0F));
			return;
		case 0xE004:
			cityfight_chr_rom_1k(6, (cityfight.chr_map[6] & 0x0F) | (value << 4));
			return;
		case 0xE008:
			cityfight_chr_rom_1k(7, (cityfight.chr_map[7] & 0xF0) | (value & 0x0F));
			return;
		case 0xE00C:
			cityfight_chr_rom_1k(7, (cityfight.chr_map[7] & 0x0F) | (value << 4));
			return;
		case 0xF000:
			cityfight.irq.count = ((cityfight.irq.count & 0x01E0) | ((value & 0x0F) << 1));
			return;;
		case 0xF004:
			cityfight.irq.count = ((cityfight.irq.count & 0x001E) | ((value & 0x0F) << 5));
			return;
		case 0xF008:
			cityfight.irq.enable = value & 0x02;
			irq.high &= ~EXT_IRQ;
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_CITYFIGHT(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, cityfight.reg);
	save_slot_ele(mode, slot, cityfight.chr_map);
	save_slot_ele(mode, slot, cityfight.irq.enable);
	save_slot_ele(mode, slot, cityfight.irq.count);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_CITYFIGHT(void) {
	if (cityfight.irq.enable) {
		cityfight.irq.count--;
		if (cityfight.irq.count <= 0) {
			irq.high |= EXT_IRQ;
		}
	}
}

INLINE static void cityfight_prg_update(void) {
	BYTE value;

	value = cityfight.reg[1] >> 2;
	control_bank(info.prg.rom[0].max.banks_32k)
	map_prg_rom_8k(4, 0, value);

	if (!cityfight.reg[0]) {
		value = cityfight.reg[1];
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 2, value);
	}

	map_prg_rom_8k_update();
}
