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
#include "cpu.h"
#include "save_slot.h"

void (*KS202_prg_fix)(void);
void (*KS202_prg_swap)(WORD address, WORD value);
void (*KS202_wram_fix)(void);
void (*KS202_wram_swap)(WORD address, WORD value);

_ks202 ks202;

// promemoria
//void map_init_KS202(void) {
//	EXTCL_AFTER_MAPPER_INIT(KS202);
//	EXTCL_CPU_WR_MEM(KS202);
//	EXTCL_SAVE_MAPPER(KS202);
//	EXTCL_CPU_EVERY_CYCLE(KS202);
//}
void extcl_after_mapper_init_KS202(void) {
	KS202_prg_fix();
	KS202_wram_fix();
}
void extcl_cpu_wr_mem_KS202(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			ks202.irq.reload = (ks202.irq.reload & 0xFFF0) | (value & 0x0F);
			return;
		case 0x9000:
			ks202.irq.reload = (ks202.irq.reload & 0xFF0F) | ((value & 0x0F) << 4);
			return;
		case 0xA000:
			ks202.irq.reload = (ks202.irq.reload & 0xF0FF) | ((value & 0x0F) << 8);
			return;
		case 0xB000:
			ks202.irq.reload = (ks202.irq.reload & 0x0FFF) | ((value & 0x0F) << 12);
			return;
		case 0xC000:
			ks202.irq.enabled = value & 0x0F;
			if (ks202.irq.enabled) {
				ks202.irq.count = ks202.irq.reload;
			}
			irq.high &= ~EXT_IRQ;
			return;
		case 0xD000:
			irq.high &= ~EXT_IRQ;
			return;
		case 0xE000:
			ks202.index = value;
			return;
		case 0xF000: {
			BYTE index = ks202.index & 0x07;

			switch (index) {
				case 1:
				case 2:
				case 3:
					ks202.reg[index - 1] = value;
					KS202_prg_fix();
					return;
				case 4:
					ks202.reg[3] = value;
					KS202_wram_fix();
					return;
				case 5:
					ks202.reg[4] = value & 0x04;
					KS202_wram_fix();
					return;
				default:
					return;
			}
		}
	}
}
BYTE extcl_save_mapper_KS202(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, ks202.index);
	save_slot_ele(mode, slot, ks202.reg);
	save_slot_ele(mode, slot, ks202.irq.enabled);
	save_slot_ele(mode, slot, ks202.irq.count);
	save_slot_ele(mode, slot, ks202.irq.reload);

	if (mode == SAVE_SLOT_READ) {
		KS202_wram_fix();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_KS202(void) {
	if (!ks202.irq.enabled) {
		return;
	}
	if (++ks202.irq.count == 0xFFFF) {
		ks202.irq.count = ks202.irq.reload;
		irq.delay = TRUE;
		irq.high |= EXT_IRQ;
	}
}

void init_KS202(void) {
	if (info.reset >= HARD) {
		memset(&ks202, 0x00, sizeof(ks202));

		ks202.reg[0] = 0;
		ks202.reg[1] = 1;
		ks202.reg[2] = 2;
		ks202.reg[3] = 3;
	}

	irq.high &= ~EXT_IRQ;

	KS202_prg_fix = prg_fix_KS202_base;
	KS202_prg_swap = prg_swap_KS202_base;
	KS202_wram_fix = wram_fix_KS202_base;
	KS202_wram_swap = wram_swap_KS202_base;
}
void prg_fix_KS202_base(void) {
	KS202_prg_swap(0x8000, ks202.reg[0]);
	KS202_prg_swap(0xA000, ks202.reg[1]);
	KS202_prg_swap(0xC000, ks202.reg[2]);
	KS202_prg_swap(0xE000, 0xFF);
}
void prg_swap_KS202_base(WORD address, WORD value) {
	memmap_auto_8k(MMCPU(address), value);
}
void wram_fix_KS202_base(void) {
	KS202_wram_swap(0x6000, ks202.reg[4] ? ks202.reg[3] : 0);
}
void wram_swap_KS202_base(WORD address, WORD value) {
	if (ks202.reg[4]) {
		memmap_prgrom_8k(MMCPU(address), value);
	} else {
		memmap_auto_8k(MMCPU(address), value);
	}

}