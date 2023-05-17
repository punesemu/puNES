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
#include "VRC7_snd.h"

void (*VRC7_prg_fix)(void);
void (*VRC7_prg_swap)(WORD address, WORD value);
void (*VRC7_chr_fix)(void);
void (*VRC7_chr_swap)(WORD address, WORD value);
void (*VRC7_wram_fix)(void);
void (*VRC7_mirroring_fix)(void);

_vrc7 vrc7;
struct _vrc7tmp {
	WORD A0;
	WORD A1;
	BYTE irq_delay;
} vrc7tmp;

// promemoria
//void map_init_VRC7(BYTE revision) {
//	EXTCL_AFTER_MAPPER_INIT(VRC7);
//	EXTCL_CPU_WR_MEM(VRC7);
//	EXTCL_SAVE_MAPPER(VRC7);
//	EXTCL_CPU_EVERY_CYCLE(VRC7);
//	EXTCL_APU_TICK(VRC7);
//}

void extcl_after_mapper_init_VRC7(void) {
	VRC7_prg_fix();
	VRC7_chr_fix();
	VRC7_wram_fix();
	VRC7_mirroring_fix();
}
void extcl_cpu_wr_mem_VRC7(WORD address, BYTE value) {
	WORD bank = address & 0xF000;
	int index = 0;

	switch (bank) {
		case 0x8000:
		case 0x9000:
			index = ((bank & 0x1000) >> 11) | (address & vrc7tmp.A0 ? 1 : 0);
			switch (index) {
				case 0x00:
				case 0x01:
				case 0x02:
					vrc7.prg[index] = value;
					VRC7_prg_fix();
					return;
				default:
					index = (address & vrc7tmp.A1) ? 1 : 0;
					opll_write_reg(index, value);
					return;
			}
			return;
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000:
			index = ((bank - 0xA000) >> 11) | (address & vrc7tmp.A0 ? 1 : 0);
			vrc7.chr[index] = value;
			VRC7_chr_fix();
			return;
		case 0xE000:
			if (address & vrc7tmp.A0) {
				vrc7.irq.reload = value;
			} else {
				vrc7.reg = value;
				VRC7_wram_fix();
				VRC7_mirroring_fix();
			}
			return;
		case 0xF000:
			if (address & vrc7tmp.A0) {
				vrc7.irq.enabled = vrc7.irq.acknowledge;
				irq.high &= ~EXT_IRQ;
			} else {
				vrc7.irq.acknowledge = value & 0x01;
				vrc7.irq.enabled = value & 0x02;
				vrc7.irq.mode = value & 0x04;
				if (vrc7.irq.enabled) {
					vrc7.irq.prescaler = 0;
					vrc7.irq.count = vrc7.irq.reload;
				}
				irq.high &= ~EXT_IRQ;
			}
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_VRC7(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, vrc7.reg);
	save_slot_ele(mode, slot, vrc7.prg);
	save_slot_ele(mode, slot, vrc7.chr);
	save_slot_ele(mode, slot, vrc7.irq.enabled);
	save_slot_ele(mode, slot, vrc7.irq.reload);
	save_slot_ele(mode, slot, vrc7.irq.mode);
	save_slot_ele(mode, slot, vrc7.irq.acknowledge);
	save_slot_ele(mode, slot, vrc7.irq.count);
	save_slot_ele(mode, slot, vrc7.irq.prescaler);
	save_slot_ele(mode, slot, vrc7.irq.delay);

	return (opll_save(mode, slot, fp));
}
void extcl_cpu_every_cycle_VRC7(void) {
	if (vrc7.irq.delay && !(--vrc7.irq.delay)) {
		irq.high |= EXT_IRQ;
	}

	if (!vrc7.irq.enabled) {
		return;
	}

	if (!vrc7.irq.mode) {
		if (vrc7.irq.prescaler < 338) {
			vrc7.irq.prescaler += 3;
			return;
		}
		vrc7.irq.prescaler -= 338;
	}

	if (vrc7.irq.count != 0xFF) {
		vrc7.irq.count++;
		return;
	}

	vrc7.irq.count = vrc7.irq.reload;
	vrc7.irq.delay = vrc7tmp.irq_delay;
}
void extcl_apu_tick_VRC7(void) {
	opll_update();
}

void init_NSF_VRC7(WORD A0, WORD A1) {
	memset(&vrc7, 0x00, sizeof(vrc7));

	opll_reset();

	vrc7tmp.A0 = A0;
	vrc7tmp.A1 = A1;
}
void init_VRC7(WORD A0, WORD A1) {
	if (info.reset >= HARD) {
		memset(&vrc7, 0x00, sizeof(vrc7));

		vrc7.prg[0] = 0;
		vrc7.prg[1] = 1;
		vrc7.prg[2] = 0xFE;

		vrc7.chr[0] = 0;
		vrc7.chr[1] = 1;
		vrc7.chr[2] = 2;
		vrc7.chr[3] = 3;
		vrc7.chr[4] = 4;
		vrc7.chr[5] = 5;
		vrc7.chr[6] = 6;
		vrc7.chr[7] = 7;
	}

	vrc7.irq.enabled = 0;
	vrc7.irq.reload = 0;
	vrc7.irq.mode = 0;
	vrc7.irq.acknowledge = 0;
	vrc7.irq.count = 0;
	vrc7.irq.delay = 0;
	vrc7.irq.prescaler = 0;

	irq.high &= ~EXT_IRQ;

	opll_reset();

	vrc7tmp.A0 = A0;
	vrc7tmp.A1 = A1;
	vrc7tmp.irq_delay = 1;

	VRC7_prg_fix = prg_fix_VRC7_base;
	VRC7_prg_swap = prg_swap_VRC7_base;
	VRC7_chr_fix = chr_fix_VRC7_base;
	VRC7_chr_swap = chr_swap_VRC7_base;
	VRC7_wram_fix = wram_fix_VRC7_base;
	VRC7_mirroring_fix = mirroring_fix_VRC7_base;
}
void prg_fix_VRC7_base(void) {
	VRC7_prg_swap(0x8000, vrc7.prg[0]);
	VRC7_prg_swap(0xA000, vrc7.prg[1]);
	VRC7_prg_swap(0xC000, vrc7.prg[2]);
	VRC7_prg_swap(0xE000, ~0);
}
void prg_swap_VRC7_base(WORD address, WORD value) {
	memmap_auto_8k(address, value);
}
void chr_fix_VRC7_base(void) {
	VRC7_chr_swap(0x0000, vrc7.chr[0]);
	VRC7_chr_swap(0x0400, vrc7.chr[1]);
	VRC7_chr_swap(0x0800, vrc7.chr[2]);
	VRC7_chr_swap(0x0C00, vrc7.chr[3]);
	VRC7_chr_swap(0x1000, vrc7.chr[4]);
	VRC7_chr_swap(0x1400, vrc7.chr[5]);
	VRC7_chr_swap(0x1800, vrc7.chr[6]);
	VRC7_chr_swap(0x1C00, vrc7.chr[7]);
}
void chr_swap_VRC7_base(WORD address, WORD value) {
	map_chr_rom_1k(address, value);
}
void wram_fix_VRC7_base(void) {
	memmap_auto_wp_8k(0x6000, 0, (vrc7.reg >> 7), (vrc7.reg >> 7));
}
void mirroring_fix_VRC7_base(void) {
	switch (vrc7.reg & 0x03) {
		default:
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
