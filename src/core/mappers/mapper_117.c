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
#include "irqA12.h"
#include "save_slot.h"

struct _m117 {
	BYTE delay;
} m117;

void map_init_117(void) {
	EXTCL_CPU_WR_MEM(117);
	EXTCL_SAVE_MAPPER(117);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	EXTCL_IRQ_A12_CLOCK(117);
	EXTCL_CPU_EVERY_CYCLE(117);
	mapper.internal_struct[0] = (BYTE *)&m117;
	mapper.internal_struct_size[0] = sizeof(m117);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m117, 0x00, sizeof(m117));

	irqA12.present = TRUE;
}
void extcl_cpu_wr_mem_117(WORD address, BYTE value) {
	switch (address) {
		case 0x8000:
		case 0x8001:
		case 0x8002:
		case 0x8003:
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, address & 0x0003, value);
			map_prg_rom_8k_update();
			return;
		case 0xA000:
		case 0xA001:
		case 0xA002:
		case 0xA003:
		case 0xA004:
		case 0xA005:
		case 0xA006:
		case 0xA007:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[address & 0x0007] = chr_pnt(value << 10);
			return;
		case 0xC001:
			irqA12.reload = value;
			return;
		case 0xC002:
			irq.high &= ~EXT_IRQ;
			return;
		case 0xC003:
			irqA12.counter = irqA12.reload;
			return;
		case 0xD000:
			if (value & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			return;
		case 0xE000:
			irqA12.enable = value & 0x01;
			return;
	}
}
BYTE extcl_save_mapper_117(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m117.delay);

	return (EXIT_OK);
}
void extcl_irq_A12_clock_117(void) {
	if (irqA12.enable && irqA12.counter && !(--irqA12.counter)) {
		m117.delay = 2;
	}
}
void extcl_cpu_every_cycle_117(void) {
	if (m117.delay && !(--m117.delay)) {
		irq.high |= EXT_IRQ;
	}
}
