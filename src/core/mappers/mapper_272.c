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
#include "save_slot.h"

INLINE static void prg_fix_272(void);
INLINE static void chr_fix_272(void);
INLINE static void mirroring_fix_272(void);

struct _m272 {
	BYTE prg[2];
	BYTE chr[8];
	BYTE mirroring[2];
	struct _m272_irq {
		BYTE enabled;
		BYTE counter;
		WORD index;
	} irq;
} m272;

void map_init_272(void) {
	EXTCL_AFTER_MAPPER_INIT(272);
	EXTCL_CPU_WR_MEM(272);
	EXTCL_SAVE_MAPPER(272);
	EXTCL_PPU_000_TO_34X(272);
	map_internal_struct_init((BYTE *)&m272, sizeof(m272));

	if (info.reset >= HARD) {
		memset(&m272, 0x00, sizeof(m272));
	}
}
void extcl_after_mapper_init_272(void) {
	prg_fix_272();
	chr_fix_272();
	mirroring_fix_272();
}
void extcl_cpu_wr_mem_272(BYTE nidx, WORD address, BYTE value) {
	switch (address & 0xC000) {
		case 0x8000:
			if ((address & 0x000C) == 0x0004) {
				m272.mirroring[0] = value & 0x03;
				mirroring_fix_272();
			} else if ((address & 0x000C) == 0x000C) {
				nes[nidx].c.irq.high |= EXT_IRQ;
			}
			break;
		case 0xC000:
			if ((address & 0x000C) == 0x0004) {
				nes[nidx].c.irq.high &= ~EXT_IRQ;
			} else if ((address & 0x000C) == 0x0008) {
				m272.irq.enabled = TRUE;
			} else if ((address & 0x000C) == 0x000C) {
				nes[nidx].c.irq.high &= ~EXT_IRQ;
				m272.irq.enabled = FALSE;
				m272.irq.counter = 0;
			}
			break;
	}
	switch (address & 0xF000) {
		case 0x8000:
			m272.prg[0] = value;
			prg_fix_272();
			break;
		case 0x9000:
			m272.mirroring[1] = value & 0x01;
			mirroring_fix_272();
			break;
		case 0xA000:
			m272.prg[1] = value;
			prg_fix_272();
			break;
		case 0xF000:
			break;
		default: {
			BYTE index = ((address - 0xB000) >> 11) | ((address & 0x02) >> 1);

			if (address & 0x0001) {
				m272.chr[index] = (m272.chr[index] & 0x0F) | ((value & 0x0F) << 4);
			} else {
				m272.chr[index] = (m272.chr[index] & 0xF0) | (value & 0x0F);
			}
			chr_fix_272();
			break;
		}
	}
}
BYTE extcl_save_mapper_272(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m272.prg);
	save_slot_ele(mode, slot, m272.chr);
	save_slot_ele(mode, slot, m272.mirroring);
	save_slot_ele(mode, slot, m272.irq.enabled);
	save_slot_ele(mode, slot, m272.irq.counter);
	return (EXIT_OK);
}
void extcl_ppu_000_to_34x_272(BYTE nidx) {
	if (nes[nidx].p.r2001.visible && ((nes[nidx].p.ppu.frame_y >= nes[nidx].p.ppu_sclines.vint)) &&
		(nes[nidx].p.ppu.frame_y < nes[nidx].p.ppu_sclines.total)) {
		if (++m272.irq.index == 6) {
			if (m272.irq.enabled && (++m272.irq.counter == 84)) {
				nes[nidx].c.irq.high |= EXT_IRQ;
				m272.irq.counter = 0;
			}
		} else if (m272.irq.index == 8) {
			m272.irq.index = 0;
		}
	}
	if (nes[nidx].p.ppu.frame_x == 340) {
		m272.irq.index = 0;
	}
}

INLINE static void prg_fix_272(void) {
	memmap_auto_8k(0, MMCPU(0x8000), m272.prg[0]);
	memmap_auto_8k(0, MMCPU(0xA000), m272.prg[1]);
	memmap_auto_16k(0, MMCPU(0xC000), 0xFF);
}
INLINE static void chr_fix_272(void) {
	memmap_auto_1k(0, MMPPU(0x0000), m272.chr[0]);
	memmap_auto_1k(0, MMPPU(0x0400), m272.chr[1]);
	memmap_auto_1k(0, MMPPU(0x0800), m272.chr[2]);
	memmap_auto_1k(0, MMPPU(0x0C00), m272.chr[3]);
	memmap_auto_1k(0, MMPPU(0x1000), m272.chr[4]);
	memmap_auto_1k(0, MMPPU(0x1400), m272.chr[5]);
	memmap_auto_1k(0, MMPPU(0x1800), m272.chr[6]);
	memmap_auto_1k(0, MMPPU(0x1C00), m272.chr[7]);
}
INLINE static void mirroring_fix_272(void) {
	switch (m272.mirroring[0]) {
		default:
			if (m272.mirroring[1]) {
				mirroring_H(0);
			} else {
				mirroring_V(0);
			}
			break;
		case 2:
			mirroring_SCR0(0);
			break;
		case 3:
			mirroring_SCR1(0);
			break;
	}
}
