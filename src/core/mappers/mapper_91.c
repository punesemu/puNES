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
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"
#include "cpu.h"
#include "ppu.h"

struct _m91 {
	struct _m91_irq {
		BYTE active;
		BYTE count;
	} irq;
} m91;

void map_init_91(void) {
	EXTCL_CPU_WR_MEM(91);
	EXTCL_SAVE_MAPPER(91);
	EXTCL_PPU_256_TO_319(91);
	mapper.internal_struct[0] = (BYTE *)&m91;
	mapper.internal_struct_size[0] = sizeof(m91);

	memset(&m91, 0x00, sizeof(m91));

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_91(WORD address, BYTE value) {
	if (address < 0x6000) {
		return;
	}
	if (address <= 0x6FFF) {
		DBWORD bank;

		control_bank(info.chr.rom[0].max.banks_2k)
		bank = value << 11;

		switch (address & 0x0003) {
			case 0:
				chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
				chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
				return;
			case 1:
				chr.bank_1k[2] = chr_chip_byte_pnt(0, bank);
				chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0400);
				return;
			case 2:
				chr.bank_1k[4] = chr_chip_byte_pnt(0, bank);
				chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x0400);
				return;
			case 3:
				chr.bank_1k[6] = chr_chip_byte_pnt(0, bank);
				chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x0400);
				return;
		}
	}
	if (address < 0x7FFF) {
		switch (address & 0x0003) {
			case 0:
				control_bank(info.prg.rom[0].max.banks_8k)
				map_prg_rom_8k(1, 0, value);
				map_prg_rom_8k_update();
				return;
			case 1:
				control_bank(info.prg.rom[0].max.banks_8k)
				map_prg_rom_8k(1, 1, value);
				map_prg_rom_8k_update();
				return;
			case 2:
				m91.irq.active = 0;
				m91.irq.count = 0;
				irq.high &= ~EXT_IRQ;
				return;
			case 3:
				m91.irq.active = 1;
				irq.high &= ~EXT_IRQ;
				return;
		}
	}
}
BYTE extcl_save_mapper_91(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m91.irq.active);
	save_slot_ele(mode, slot, m91.irq.count);

	return (EXIT_OK);
}
void extcl_ppu_256_to_319_91(void) {
	if (ppu.frame_x != 319) {
		return;
	}

	if (m91.irq.active && (m91.irq.count < 8)) {
		m91.irq.count++;
		if (m91.irq.count >= 8) {
			irq.high |= EXT_IRQ;
		}
	}
}
