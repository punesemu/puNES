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
#include "cpu.h"
#include "save_slot.h"

void (*FCG_prg_fix)(void);
void (*FCG_prg_swap)(WORD address, WORD value);
void (*FCG_chr_fix)(void);
void (*FCG_chr_swap)(WORD address, WORD value);
void (*FCG_mirroring_fix)(void);

_fcg fcg;

// promemoria
//void map_init_FCG(void) {
//	EXTCL_AFTER_MAPPER_INIT(FCG);
//	EXTCL_CPU_WR_MEM(FCG);
//	EXTCL_SAVE_MAPPER(FCG);
//	EXTCL_CPU_EVERY_CYCLE(FCG);
//}

void init_FCG(void) {
	if (info.reset >= HARD) {
		memset(&fcg, 0x00, sizeof(fcg));

		fcg.chr[1] = 1;
		fcg.chr[2] = 2;
		fcg.chr[3] = 3;
		fcg.chr[4] = 4;
		fcg.chr[5] = 5;
		fcg.chr[6] = 6;
		fcg.chr[7] = 7;
	}

	info.mapper.extend_wr = TRUE;

	FCG_prg_fix = prg_fix_FCG_base;
	FCG_prg_swap = prg_swap_FCG_base;
	FCG_chr_fix = chr_fix_FCG_base;
	FCG_chr_swap = chr_swap_FCG_base;
	FCG_mirroring_fix = mirroring_fix_FCG_base;
}
void extcl_after_mapper_init_FCG(void) {
	FCG_prg_fix();
	FCG_chr_fix();
	FCG_mirroring_fix();
}
void extcl_cpu_wr_mem_FCG(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		switch (address & 0x0F) {
			case 0x00:
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
			case 0x06:
			case 0x07:
				fcg.chr[address & 0x07] = value;
				FCG_chr_fix();
				return;
			case 0x08:
				fcg.prg = value;
				FCG_prg_fix();
				return;
			case 0x09:
				fcg.mirroring = value;
				FCG_mirroring_fix();
				return;
			case 0x0A:
				fcg.irq.enabled = value & 0x01;
				if (fcg.irq.enabled && !fcg.irq.count) {
					fcg.irq.delay = 1;
				} else {
					irq.high &= ~EXT_IRQ;
				}
				return;
			case 0x0B:
				fcg.irq.count = (fcg.irq.count & 0xFF00) | value;
				return;
			case 0x0C:
				fcg.irq.count = (fcg.irq.count & 0x00FF) | (value << 8);
				return;
			default:
				return;
		}
	}
}
BYTE extcl_save_mapper_FCG(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, fcg.prg);
	save_slot_ele(mode, slot, fcg.chr);
	save_slot_ele(mode, slot, fcg.mirroring);
	save_slot_ele(mode, slot, fcg.irq.enabled);
	save_slot_ele(mode, slot, fcg.irq.count);
	save_slot_ele(mode, slot, fcg.irq.delay);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_FCG(void) {
	if (fcg.irq.delay && !(--fcg.irq.delay)) {
		irq.high |= EXT_IRQ;
	}
	if (fcg.irq.enabled && !(--fcg.irq.count)) {
		fcg.irq.delay = 1;
	}
}

void prg_fix_FCG_base(void) {
	FCG_prg_swap(0x8000, fcg.prg);
	FCG_prg_swap(0xC000, 0xFF);
}
void prg_swap_FCG_base(WORD address, WORD value) {
	memmap_auto_16k(MMCPU(address), value);
}
void chr_fix_FCG_base(void) {
	FCG_chr_swap(0x0000, fcg.chr[0]);
	FCG_chr_swap(0x0400, fcg.chr[1]);
	FCG_chr_swap(0x0800, fcg.chr[2]);
	FCG_chr_swap(0x0C00, fcg.chr[3]);
	FCG_chr_swap(0x1000, fcg.chr[4]);
	FCG_chr_swap(0x1400, fcg.chr[5]);
	FCG_chr_swap(0x1800, fcg.chr[6]);
	FCG_chr_swap(0x1C00, fcg.chr[7]);
}
void chr_swap_FCG_base(WORD address, WORD value) {
	memmap_auto_1k(MMPPU(address), value);
}
void mirroring_fix_FCG_base(void) {
	switch (fcg.mirroring & 0x03) {
		case 0:
			mirroring_V();
			break;
		case 1:
			mirroring_H();
			break;
		case 2:
			mirroring_SCR0();
			break;
		case 3:
			mirroring_SCR1();
			break;
	}
}