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

void (*VRC2and4_prg_fix)(void);
void (*VRC2and4_prg_swap)(WORD address, WORD value);
void (*VRC2and4_chr_fix)(void);
void (*VRC2and4_chr_swap)(WORD address, WORD value);
void (*VRC2and4_wram_fix)(void);
void (*VRC2and4_mirroring_fix)(void);
void (*VRC2and4_wired_fix)(void);
void (*VRC2and4_misc_03)(WORD address, BYTE value);

_vrc2and4 vrc2and4;
struct _vrc2and4tmp {
	BYTE type;
	WORD A0;
	WORD A1;
	BYTE prg6000_wired;
	BYTE irq_repeated;
} vrc2and4tmp;

// promemoria
//void map_init_VRC2and4(void) {
//	EXTCL_AFTER_MAPPER_INIT(VRC2and4);
//	EXTCL_CPU_WR_MEM(VRC2and4);
//	EXTCL_CPU_RD_MEM(VRC2and4); // VRC2
//	EXTCL_SAVE_MAPPER(VRC2and4);
//	EXTCL_CPU_EVERY_CYCLE(VRC2and4); // VRC4
//}

void extcl_after_mapper_init_VRC2and4(void) {
	if (vrc2and4tmp.type == VRC24_VRC2) {
		if ((info.format == NES_2_0) && !prg_wram_size()) {
			vrc2and4tmp.prg6000_wired = TRUE;
			info.mapper.extend_wr = TRUE;
		}
	}
	VRC2and4_prg_fix();
	VRC2and4_chr_fix();
	VRC2and4_wram_fix();
	VRC2and4_mirroring_fix();
}
void extcl_cpu_wr_mem_VRC2and4(WORD address, BYTE value) {
	WORD bank = address & 0xF000;
	int index = 0;

	switch (bank) {
		case 0x6000:
			if (vrc2and4tmp.prg6000_wired) {
				vrc2and4.wired = (vrc2and4.wired & 0xF8) | (value & 0x07);
				VRC2and4_wired_fix();
			}
			return;
		case 0x8000:
		case 0xA000:
			vrc2and4.prg[(address >> 13) & 0x01] = value;
			VRC2and4_prg_fix();
			return;
		case 0x9000:
			index = ((address & vrc2and4tmp.A1 ? 2 : 0) | (address & vrc2and4tmp.A0 ? 1 : 0)) &
					((vrc2and4tmp.type == VRC24_VRC4) ? 0x03 : 0x00) ;
			switch (index) {
				case 0:
				case 1:
					vrc2and4.mirroring = value;
					VRC2and4_mirroring_fix();
					return;
				case 2:
					vrc2and4.wram_protect = !(value & 0x01);
					vrc2and4.swap_mode = value & 0x02;
					VRC2and4_wram_fix();
					VRC2and4_prg_fix();
					return;
				case 3:
					VRC2and4_misc_03(address, value);
					break;
				default:
					return;
			}
			return;
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000: {
			WORD base = 0, mask = 0;

			if (address & vrc2and4tmp.A0) {
				base = value << 4;
				mask = 0x000F;
			} else {
				base = value & 0x0F;
				mask = 0x0FF0;
			}
			index = ((bank - 0xB000) >> 11) | (address & vrc2and4tmp.A1 ? 1 : 0);
			vrc2and4.chr[index] = base | (vrc2and4.chr[index] & mask);
			VRC2and4_chr_fix();
			return;
		}
		case 0xF000:
			if (vrc2and4tmp.type == VRC24_VRC4) {
				index = (address & vrc2and4tmp.A1 ? 2 : 0) | (address & vrc2and4tmp.A0 ? 1 : 0);
				switch (index) {
					case 0:
						vrc2and4.irq.reload = (vrc2and4.irq.reload & 0xF0) | (value & 0x0F);
						return;
					case 1:
						vrc2and4.irq.reload = (vrc2and4.irq.reload & 0x0F) | ((value & 0x0F) << 4);
						return;
					case 2:
						vrc2and4.irq.acknowledge = value & 0x01;
						vrc2and4.irq.enabled = value & 0x02;
						vrc2and4.irq.mode = value & 0x04;
						if (vrc2and4.irq.enabled) {
							vrc2and4.irq.prescaler = 0;
							vrc2and4.irq.count = vrc2and4.irq.reload;
						}
						irq.high &= ~EXT_IRQ;
						return;
					case 3:
						if (vrc2and4tmp.irq_repeated) {
							vrc2and4.irq.enabled = vrc2and4.irq.acknowledge;
						}
						irq.high &= ~EXT_IRQ;
						return;
					default:
						break;
				}
			}
			return;
		default:
			return;
	}
}
BYTE extcl_cpu_rd_mem_VRC2and4(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xF000) {
		case 0x6000:
			return (vrc2and4tmp.prg6000_wired ? (openbus & 0xFE) | ((vrc2and4.wired & 0x08) >> 3) : openbus);
		default:
			return (openbus);
	}
}
BYTE extcl_save_mapper_VRC2and4(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, vrc2and4.prg);
	save_slot_ele(mode, slot, vrc2and4.chr);
	save_slot_ele(mode, slot, vrc2and4.mirroring);
	save_slot_ele(mode, slot, vrc2and4.swap_mode);
	save_slot_ele(mode, slot, vrc2and4.wram_protect);
	save_slot_ele(mode, slot, vrc2and4.wired);
	save_slot_ele(mode, slot, vrc2and4.irq.enabled);
	save_slot_ele(mode, slot, vrc2and4.irq.reload);
	save_slot_ele(mode, slot, vrc2and4.irq.mode);
	save_slot_ele(mode, slot, vrc2and4.irq.acknowledge);
	save_slot_ele(mode, slot, vrc2and4.irq.count);
	save_slot_ele(mode, slot, vrc2and4.irq.prescaler);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_VRC2and4(void) {
	if (!vrc2and4.irq.enabled) {
		return;
	}

	if (!vrc2and4.irq.mode) {
		if (vrc2and4.irq.prescaler < 338) {
			vrc2and4.irq.prescaler += 3;
			return;
		}
		vrc2and4.irq.prescaler -= 338;
	}

	if (vrc2and4.irq.count != 0xFF) {
		vrc2and4.irq.count++;
		return;
	}

	vrc2and4.irq.count = vrc2and4.irq.reload;
	irq.delay = TRUE;
	irq.high |= EXT_IRQ;
}

void init_VRC2and4(BYTE type, WORD A0, WORD A1, BYTE irq_repeated) {
	if (info.reset >= HARD) {
		memset(&vrc2and4, 0x00, sizeof(vrc2and4));

		vrc2and4.prg[0] = 0;
		vrc2and4.prg[1] = 1;

		vrc2and4.chr[0] = 0;
		vrc2and4.chr[1] = 1;
		vrc2and4.chr[2] = 2;
		vrc2and4.chr[3] = 3;
		vrc2and4.chr[4] = 4;
		vrc2and4.chr[5] = 5;
		vrc2and4.chr[6] = 6;
		vrc2and4.chr[7] = 7;
	}

	vrc2and4.irq.enabled = 0;
	vrc2and4.irq.reload = 0;
	vrc2and4.irq.mode = 0;
	vrc2and4.irq.acknowledge = 0;
	vrc2and4.irq.count = 0;
	vrc2and4.irq.prescaler = 0;

	vrc2and4.wram_protect = ((info.format == NES_2_0) && (info.mapper.submapper != DEFAULT) && (info.mapper.submapper > 1));

	irq.high &= ~EXT_IRQ;

	vrc2and4tmp.type = type;
	vrc2and4tmp.A0 = A0;
	vrc2and4tmp.A1 = A1;
	vrc2and4tmp.prg6000_wired = FALSE;
	vrc2and4tmp.irq_repeated = irq_repeated;

	VRC2and4_prg_fix = prg_fix_VRC2and4_base;
	VRC2and4_prg_swap = prg_swap_VRC2and4_base;
	VRC2and4_chr_fix = chr_fix_VRC2and4_base;
	VRC2and4_chr_swap = chr_swap_VRC2and4_base;
	VRC2and4_wram_fix = wram_fix_VRC2and4_base;
	VRC2and4_mirroring_fix = mirroring_fix_VRC2and4_base;
	VRC2and4_wired_fix = wired_fix_VRC2and4_base;
	VRC2and4_misc_03 = misc_03_VRC2and4_base;
}
void prg_fix_VRC2and4_base(void) {
	if (vrc2and4.swap_mode) {
		VRC2and4_prg_swap(0x8000, ~1);
		VRC2and4_prg_swap(0xC000, vrc2and4.prg[0]);
	} else {
		VRC2and4_prg_swap(0x8000, vrc2and4.prg[0]);
		VRC2and4_prg_swap(0xC000, ~1);
	}
	VRC2and4_prg_swap(0xA000, vrc2and4.prg[1]);
	VRC2and4_prg_swap(0xE000, ~0);
}
void prg_swap_VRC2and4_base(WORD address, WORD value) {
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_fix_VRC2and4_base(void) {
	VRC2and4_chr_swap(0x0000, vrc2and4.chr[0]);
	VRC2and4_chr_swap(0x0400, vrc2and4.chr[1]);
	VRC2and4_chr_swap(0x0800, vrc2and4.chr[2]);
	VRC2and4_chr_swap(0x0C00, vrc2and4.chr[3]);
	VRC2and4_chr_swap(0x1000, vrc2and4.chr[4]);
	VRC2and4_chr_swap(0x1400, vrc2and4.chr[5]);
	VRC2and4_chr_swap(0x1800, vrc2and4.chr[6]);
	VRC2and4_chr_swap(0x1C00, vrc2and4.chr[7]);
}
void chr_swap_VRC2and4_base(WORD address, WORD value) {
	map_chr_rom_1k(address, value);
}
void wram_fix_VRC2and4_base(void) {
	wram_map_auto_wp_8k(0x6000, 0, !vrc2and4.wram_protect, !vrc2and4.wram_protect);
}
void mirroring_fix_VRC2and4_base(void) {
	switch (vrc2and4.mirroring & (vrc2and4tmp.type == VRC24_VRC4 ? 0x03 : 0x01)) {
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
void wired_fix_VRC2and4_base(void) {}
void misc_03_VRC2and4_base(UNUSED(WORD address), UNUSED(BYTE value)) {}
