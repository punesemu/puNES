/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

struct _futuremedia {
	BYTE delay;
} futuremedia;

void map_init_Futuremedia(void) {
	EXTCL_CPU_WR_MEM(Futuremedia);
	EXTCL_SAVE_MAPPER(Futuremedia);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	EXTCL_IRQ_A12_CLOCK(Futuremedia);
	EXTCL_CPU_EVERY_CYCLE(Futuremedia);
	mapper.internal_struct[0] = (BYTE *) &futuremedia;
	mapper.internal_struct_size[0] = sizeof(futuremedia);

	memset(&futuremedia, 0x00, sizeof(futuremedia));
	memset(&irqA12, 0x00, sizeof(irqA12));

	irqA12.present = TRUE;
}
void extcl_cpu_wr_mem_Futuremedia(WORD address, BYTE value) {
	switch (address) {
		case 0x8000:
		case 0x8001:
		case 0x8002:
		case 0x8003:
			control_bank(info.prg.rom[0].max.banks_8k)
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
			control_bank(info.chr.rom[0].max.banks_1k)
			chr.bank_1k[address & 0x0007] = chr_chip_byte_pnt(0, value << 10);
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
BYTE extcl_save_mapper_Futuremedia(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, futuremedia.delay);

	return (EXIT_OK);
}
void extcl_irq_A12_clock_Futuremedia(void) {
	if (irqA12.enable && irqA12.counter && !(--irqA12.counter)) {
		futuremedia.delay = 2;
	}
}
void extcl_cpu_every_cycle_Futuremedia(void) {
	if (futuremedia.delay && !(--futuremedia.delay)) {
		irq.high |= EXT_IRQ;
	}
}
