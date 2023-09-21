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

void (*VRC6_prg_fix)(void);
void (*VRC6_prg_swap)(WORD address, WORD value);
void (*VRC6_chr_fix)(void);
void (*VRC6_chr_swap)(WORD address, WORD value);
void (*VRC6_wram_fix)(void);
void (*VRC6_mirroring_fix)(void);
void (*VRC6_nmt_swap)(WORD address, WORD value);

INLINE static WORD chr_control(const WORD slot, const WORD value);
INLINE static void vrc6_square_wr(int index, BYTE value, _vrc6_square *square);
INLINE static void vrc6_square_tick(_vrc6_square *square);

_vrc6 vrc6;
struct _vrc6tmp {
	WORD A0;
	WORD A1;
	BYTE irq_delay;
} vrc6tmp;

// promemoria
//void map_init_VRC6(void) {
//	EXTCL_AFTER_MAPPER_INIT(VRC6);
//	EXTCL_CPU_WR_MEM(VRC6);
//	EXTCL_SAVE_MAPPER(VRC6);
//	EXTCL_CPU_EVERY_CYCLE(VRC6);
//	EXTCL_APU_TICK(VRC6);
//}

void extcl_after_mapper_init_VRC6(void) {
	VRC6_prg_fix();
	VRC6_chr_fix();
	VRC6_wram_fix();
	VRC6_mirroring_fix();
}
void extcl_cpu_wr_mem_VRC6(BYTE cidx, WORD address, BYTE value) {
	WORD bank = address & 0xF000;
	int index = 0;

	switch (bank) {
		case 0x8000:
		case 0xC000:
			vrc6.prg[(address >> 14) & 0x01] = value;
			VRC6_prg_fix();
			return;
		case 0x9000:
			index = (address & vrc6tmp.A1 ? 2 : 0) | (address & vrc6tmp.A0 ? 1 : 0);
			vrc6_square_wr(index, value, &vrc6.S3);
			return;
		case 0xA000:
			index = (address & vrc6tmp.A1 ? 2 : 0) | (address & vrc6tmp.A0 ? 1 : 0);
			vrc6_square_wr(index, value, &vrc6.S4);
			return;
		case 0xB000:
			index = (address & vrc6tmp.A1 ? 2 : 0) | (address & vrc6tmp.A0 ? 1 : 0);
			switch (index) {
				case 0x00:
					vrc6.saw.accumulator = value & 0x3F;
					break;
				case 0x01:
					vrc6.saw.frequency = (vrc6.saw.frequency & 0x0F00) | value;
					break;
				case 0x02:
					vrc6.saw.frequency = (vrc6.saw.frequency & 0x00FF) | ((value & 0x0F) << 8);
					vrc6.saw.enabled = value & 0x80;
					break;
				case 0x03:
					vrc6.reg = value;
					VRC6_chr_fix();
					VRC6_wram_fix();
					VRC6_mirroring_fix();
					break;
			}
			return;
		case 0xD000:
		case 0xE000:
			index = ((bank - 0xD000) >> 10) | (address & vrc6tmp.A1 ? 2 : 0) | (address & vrc6tmp.A0 ? 1 : 0);
			vrc6.chr[index] = value;
			VRC6_chr_fix();
			VRC6_mirroring_fix();
			return;
		case 0xF000:
			index = (address & vrc6tmp.A1 ? 2 : 0) | (address & vrc6tmp.A0 ? 1 : 0);

			switch (index) {
				case 0x00:
					vrc6.irq.reload = value;
					break;
				case 0x01:
					vrc6.irq.acknowledge = value & 0x01;
					vrc6.irq.enabled = value & 0x02;
					vrc6.irq.mode = value & 0x04;
					if (vrc6.irq.enabled) {
						vrc6.irq.prescaler = 0;
						vrc6.irq.count = vrc6.irq.reload;
					}
					nes[cidx].c.irq.high &= ~EXT_IRQ;
					break;
				case 0x02:
					vrc6.irq.enabled = vrc6.irq.acknowledge;
					nes[cidx].c.irq.high &= ~EXT_IRQ;
					break;
				default:
					break;
			}
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_VRC6(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, vrc6.reg);
	save_slot_ele(mode, slot, vrc6.prg);
	save_slot_ele(mode, slot, vrc6.chr);

	save_slot_ele(mode, slot, vrc6.irq.enabled);
	save_slot_ele(mode, slot, vrc6.irq.reload);
	save_slot_ele(mode, slot, vrc6.irq.mode);
	save_slot_ele(mode, slot, vrc6.irq.acknowledge);
	save_slot_ele(mode, slot, vrc6.irq.count);
	save_slot_ele(mode, slot, vrc6.irq.prescaler);
	save_slot_ele(mode, slot, vrc6.irq.delay);

	save_slot_ele(mode, slot, vrc6.S3.enabled);
	save_slot_ele(mode, slot, vrc6.S3.duty);
	save_slot_ele(mode, slot, vrc6.S3.step);
	save_slot_ele(mode, slot, vrc6.S3.volume);
	save_slot_ele(mode, slot, vrc6.S3.mode);
	save_slot_ele(mode, slot, vrc6.S3.timer);
	save_slot_ele(mode, slot, vrc6.S3.frequency);
	save_slot_ele(mode, slot, vrc6.S3.output);

	save_slot_ele(mode, slot, vrc6.S4.enabled);
	save_slot_ele(mode, slot, vrc6.S4.duty);
	save_slot_ele(mode, slot, vrc6.S4.step);
	save_slot_ele(mode, slot, vrc6.S4.volume);
	save_slot_ele(mode, slot, vrc6.S4.mode);
	save_slot_ele(mode, slot, vrc6.S4.timer);
	save_slot_ele(mode, slot, vrc6.S4.frequency);
	save_slot_ele(mode, slot, vrc6.S4.output);

	save_slot_ele(mode, slot, vrc6.saw.enabled);
	save_slot_ele(mode, slot, vrc6.saw.accumulator);
	save_slot_ele(mode, slot, vrc6.saw.step);
	save_slot_ele(mode, slot, vrc6.saw.internal);
	save_slot_ele(mode, slot, vrc6.saw.timer);
	save_slot_ele(mode, slot, vrc6.saw.frequency);
	save_slot_ele(mode, slot, vrc6.saw.output);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_VRC6(BYTE cidx) {
	if (vrc6.irq.delay && !(--vrc6.irq.delay)) {
		nes[cidx].c.irq.high |= EXT_IRQ;
	}

	if (!vrc6.irq.enabled) {
		return;
	}

	if (!vrc6.irq.mode) {
		if (vrc6.irq.prescaler < 338) {
			vrc6.irq.prescaler += 3;
			return;
		}
		vrc6.irq.prescaler -= 338;
	}

	if (vrc6.irq.count != 0xFF) {
		vrc6.irq.count++;
		return;
	}

	vrc6.irq.count = vrc6.irq.reload;
	vrc6.irq.delay = vrc6tmp.irq_delay;
}
void extcl_apu_tick_VRC6(void) {
	vrc6_square_tick(&vrc6.S3);
	vrc6_square_tick(&vrc6.S4);
	if (--vrc6.saw.timer == 0) {
		vrc6.saw.timer = vrc6.saw.frequency + 1;
		vrc6.clocked = TRUE;

		if (vrc6.saw.step && !(vrc6.saw.step & 0x01)) {
			vrc6.saw.internal += vrc6.saw.accumulator;
		}
		if (++vrc6.saw.step == 14) {
			vrc6.saw.internal = vrc6.saw.step = 0;
		}
		if (vrc6.saw.enabled) {
			vrc6.saw.output = vrc6.saw.internal;
		}
	}
}

void init_NSF_VRC6(WORD A0, WORD A1) {
	memset(&vrc6, 0x00, sizeof(vrc6));

	vrc6.S3.timer = 1;
	vrc6.S3.duty = 1;
	vrc6.S4.timer = 1;
	vrc6.S4.duty = 1;
	vrc6.saw.timer = 1;

	vrc6tmp.A0 = A0;
	vrc6tmp.A1 = A1;
}
void init_VRC6(WORD A0, WORD A1, BYTE reset) {
	if (reset >= HARD) {
		memset(&vrc6, 0x00, sizeof(vrc6));

		vrc6.prg[1] = 0xFE;

		vrc6.chr[0] = 0;
		vrc6.chr[1] = 1;
		vrc6.chr[2] = 2;
		vrc6.chr[3] = 3;
		vrc6.chr[4] = 4;
		vrc6.chr[5] = 5;
		vrc6.chr[6] = 6;
		vrc6.chr[7] = 7;
	}

	vrc6.irq.enabled = 0;
	vrc6.irq.reload = 0;
	vrc6.irq.mode = 0;
	vrc6.irq.acknowledge = 0;
	vrc6.irq.count = 0;
	vrc6.irq.delay = 0;
	vrc6.irq.prescaler = 0;

	vrc6.S3.timer = 1;
	vrc6.S3.duty = 1;
	vrc6.S4.timer = 1;
	vrc6.S4.duty = 1;
	vrc6.saw.timer = 1;

	nes[0].c.irq.high &= ~EXT_IRQ;

	vrc6tmp.A0 = A0;
	vrc6tmp.A1 = A1;
	vrc6tmp.irq_delay = 1;

	VRC6_prg_fix = prg_fix_VRC6_base;
	VRC6_prg_swap = prg_swap_VRC6_base;
	VRC6_chr_fix = chr_fix_VRC6_base;
	VRC6_chr_swap = chr_swap_VRC6_base;
	VRC6_wram_fix = wram_fix_VRC6_base;
	VRC6_mirroring_fix = mirroring_fix_VRC6_base;
	VRC6_nmt_swap = nmt_swap_VRC6_base;
}
void prg_fix_VRC6_base(void) {
	VRC6_prg_swap(0x8000, (vrc6.prg[0] << 1));
	VRC6_prg_swap(0xA000, ((vrc6.prg[0] << 1) | 0x01));
	VRC6_prg_swap(0xC000, vrc6.prg[1]);
	VRC6_prg_swap(0xE000, 0xFF);
}
void prg_swap_VRC6_base(WORD address, WORD value) {
	memmap_auto_8k(0, MMCPU(address), value);
}
void chr_fix_VRC6_base(void) {
	WORD bank[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	switch (vrc6.reg & 0x03) {
		case 0:
			bank[0] = vrc6.chr[0];
			bank[1] = vrc6.chr[1];
			bank[2] = vrc6.chr[2];
			bank[3] = vrc6.chr[3];
			bank[4] = vrc6.chr[4];
			bank[5] = vrc6.chr[5];
			bank[6] = vrc6.chr[6];
			bank[7] = vrc6.chr[7];
			break;
		case 1:
			bank[0] = vrc6.chr[0];
			bank[1] = vrc6.chr[0];
			bank[2] = vrc6.chr[1];
			bank[3] = vrc6.chr[1];
			bank[4] = vrc6.chr[2];
			bank[5] = vrc6.chr[2];
			bank[6] = vrc6.chr[3];
			bank[7] = vrc6.chr[3];
			if (vrc6.reg & 0x20) {
				bank[0] &= 0xFE;
				bank[1] |= 0x01;
				bank[2] &= 0xFE;
				bank[3] |= 0x01;
				bank[4] &= 0xFE;
				bank[5] |= 0x01;
				bank[6] &= 0xFE;
				bank[7] |= 0x01;
			}
			break;
		case 2:
		case 3:
		default:
			bank[0] = vrc6.chr[0];
			bank[1] = vrc6.chr[1];
			bank[2] = vrc6.chr[2];
			bank[3] = vrc6.chr[3];
			bank[4] = vrc6.chr[4];
			bank[5] = vrc6.chr[4];
			bank[6] = vrc6.chr[5];
			bank[7] = vrc6.chr[5];
			if (vrc6.reg & 0x20) {
				bank[4] &= 0xFE;
				bank[5] |= 0x01;
				bank[6] &= 0xFE;
				bank[7] |= 0x01;
			}
			break;
	}
	VRC6_chr_swap(0x0000, bank[0]);
	VRC6_chr_swap(0x0400, bank[1]);
	VRC6_chr_swap(0x0800, bank[2]);
	VRC6_chr_swap(0x0C00, bank[3]);
	VRC6_chr_swap(0x1000, bank[4]);
	VRC6_chr_swap(0x1400, bank[5]);
	VRC6_chr_swap(0x1800, bank[6]);
	VRC6_chr_swap(0x1C00, bank[7]);
}
void chr_swap_VRC6_base(WORD address, WORD value) {
	memmap_chrrom_1k(0, MMPPU(address), chr_control(address, value));
}
void wram_fix_VRC6_base(void) {
	memmap_auto_wp_8k(0, MMCPU(0x6000), 0, (vrc6.reg >> 7), (vrc6.reg >> 7));
}
void mirroring_fix_VRC6_base(void) {
	BYTE nmt = 0, mode = vrc6.reg & 0x03, mirroring = (vrc6.reg & 0x0C) >> 2;
	BYTE reg[4] = {0, 0, 0, 0 };

	if (mode == 1) {
		reg[0] = 4;
		reg[1] = 5;
		reg[2] = 6;
		reg[3] = 7;
	} else if ((mode >> 1) ^ (mirroring & 0x01)) {
		reg[0] = reg[2] = 6;
		reg[1] = reg[3] = 7;
	} else {
		reg[0] = reg[1] = 6;
		reg[2] = reg[3] = 7;
	}

	for (nmt = 0; nmt < 4; nmt++) {
		WORD bank = vrc6.chr[reg[nmt]];

		if (vrc6.reg & 0x20) {
			if (!mode || (mode == 3)) {
				bank &= ~1;
				switch (mirroring ^ (mode & 0x01)) {
					case 0:
						// mirroring_H();
						bank |= (nmt & 0x01) ? 1 : 0;
						break;
					case 1:
						// mirroring_V();
						bank |= (nmt & 0x02) ? 1 : 0;
						break;
					case 2:
						// mirroring_SCR0();
						bank |= 0;
						break;
					case 3:
						// mirroring_SCR1();
						bank |= 1;
						break;
					default:
						break;
				}
			}
		}
		VRC6_nmt_swap((0x2000 | (nmt << 10)), bank);
	}
}
void nmt_swap_VRC6_base(WORD address, WORD value) {
	if (vrc6.reg & 0x10) {
		memmap_nmt_chrrom_1k(0, MMPPU(address), chr_control(address, value));
		memmap_nmt_chrrom_1k(0, MMPPU(address ^ 0x1000), chr_control(address, value));
	} else if (vram_size(0)) {
		memmap_nmt_vram_1k(0, MMPPU(address), value);
		memmap_nmt_vram_1k(0, MMPPU(address ^ 0x1000), value);
	} else {
		memmap_nmt_1k(0, MMPPU(address), (value & ((info.mapper.mirroring == MIRRORING_FOURSCR) ? 0x03 : 0x01)));
		memmap_nmt_1k(0, MMPPU(address ^ 0x1000), (value & ((info.mapper.mirroring == MIRRORING_FOURSCR) ? 0x03 : 0x01)));
	}
}

INLINE static WORD chr_control(const WORD address, const WORD value) {
	const WORD slot = (address & 0x0F00) >> 10;

	return (chrrom_size() >= S512K ? (value << 1) | (slot & 0x01) : value);
}
INLINE static void vrc6_square_wr(int index, BYTE value, _vrc6_square *square) {
	switch (index) {
		case 0x00:
			square->volume = value & 0x0F;
			square->duty = (value & 0x70) >> 4;
			square->mode = value & 0x80;
			break;
		case 0x01:
			square->frequency = (square->frequency & 0x0F00) | value;
			break;
		case 0x02:
			square->frequency = (square->frequency & 0x00FF) | ((value & 0x0F) << 8);
			square->enabled = value & 0x80;
			break;
		default:
			break;
	}
}
INLINE static void vrc6_square_tick(_vrc6_square *square) {
	if (--square->timer == 0) {
        square->step = (square->step + 1) & 0x0F;
        square->timer = square->frequency + 1;
        if (square->enabled) {
            square->output = 0;
            if (square->mode || (square->step <= square->duty)) {
                square->output = square->volume;
			}
		}
        vrc6.clocked = TRUE;
	}
    if (!square->enabled) {
        square->output = 0;
	}
}
