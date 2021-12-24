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
#include "cpu.h"
#include "irqA12.h"
#include "save_slot.h"

struct _m222 {
	BYTE count;
	BYTE delay;
} m222;

void map_init_222(void) {
	EXTCL_CPU_WR_MEM(222);
	EXTCL_SAVE_MAPPER(222);
	EXTCL_CPU_EVERY_CYCLE(222);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	EXTCL_IRQ_A12_CLOCK(222);

	mapper.internal_struct[0] = (BYTE *)&m222;
	mapper.internal_struct_size[0] = sizeof(m222);

	memset(&m222, 0x00, sizeof(m222));
	memset(&irqA12, 0x00, sizeof(irqA12));

	irqA12.present = TRUE;
}
void extcl_cpu_wr_mem_222(WORD address, BYTE value) {
	switch (address & 0xF003) {
		case 0x8000:
			control_bank(info.prg.rom[0].max.banks_8k)
			map_prg_rom_8k(1, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x9000:
			if (value & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			return;
		case 0xA000:
			control_bank(info.prg.rom[0].max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			map_prg_rom_8k_update();
			return;
		case 0xB000:
			control_bank(info.chr.rom[0].max.banks_1k)
			chr.bank_1k[0] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0xB002:
			control_bank(info.chr.rom[0].max.banks_1k)
			chr.bank_1k[1] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0xC000:
			control_bank(info.chr.rom[0].max.banks_1k)
			chr.bank_1k[2] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0xC002:
			control_bank(info.chr.rom[0].max.banks_1k)
			chr.bank_1k[3] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0xD000:
			control_bank(info.chr.rom[0].max.banks_1k)
			chr.bank_1k[4] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0xD002:
			control_bank(info.chr.rom[0].max.banks_1k)
			chr.bank_1k[5] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0xE000:
			control_bank(info.chr.rom[0].max.banks_1k)
			chr.bank_1k[6] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0xE002:
			control_bank(info.chr.rom[0].max.banks_1k)
			chr.bank_1k[7] = chr_chip_byte_pnt(0, value << 10);
			return;
		case 0xF000:
			//irqA12.latch = value;
			m222.count = value;
			irq.high &= ~EXT_IRQ;
			return;
	}
}
BYTE extcl_save_mapper_222(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m222.count);
	save_slot_ele(mode, slot, m222.delay);

	return (EXIT_OK);
}
void extcl_irq_A12_clock_222(void) {
	if (!m222.count || (++m222.count < 240)) {
		return;
	}

	m222.count = 0;
	m222.delay = 16;
}
void extcl_cpu_every_cycle_222(void) {
	if (m222.delay && !(--m222.delay)) {
		irq.high |= EXT_IRQ;
	}
}
