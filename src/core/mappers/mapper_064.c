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
#include "irqA12.h"
#include "ppu.h"
#include "save_slot.h"
#include "ppu_inline.h"

enum _m064_irq_enum {
	A12_MODE,
	CPU_MODE,
};

INLINE static void prg_fix_064(void);
INLINE static void chr_fix_064(void);
INLINE static void mirroring_fix_064(void);

INLINE static void irq_clock_064(void);

struct _m064 {
	WORD prg[3];
	WORD chr[8];
	BYTE mirroring;
	BYTE index;
	struct _m064_irq {
		BYTE mode;
		BYTE enable;
		BYTE prescaler;
		BYTE plus_clock;
		BYTE latch;
		BYTE delay;
		BYTE counter;
		BYTE reload;
		BYTE a12;
		BYTE a12_filter;
	} irq;
} m064;

void map_init_064(void) {
	EXTCL_AFTER_MAPPER_INIT(064);
	EXTCL_CPU_WR_MEM(064);
	EXTCL_SAVE_MAPPER(064);
	EXTCL_PPU_000_TO_255(064);
	EXTCL_PPU_256_TO_319(064);
	EXTCL_PPU_320_TO_34X(064);
	EXTCL_UPDATE_R2006(064);
	EXTCL_CPU_EVERY_CYCLE(064);
	mapper.internal_struct[0] = (BYTE *)&m064;
	mapper.internal_struct_size[0] = sizeof(m064);

	if (info.reset >= HARD) {
		memset(&m064, 0x00, sizeof(m064));

		m064.prg[0] = 0;
		m064.prg[1] = 1;
		m064.prg[2] = 0xFE;

		m064.chr[0] = 0;
		m064.chr[1] = 1;
		m064.chr[2] = 2;
		m064.chr[3] = 3;
		m064.chr[4] = 4;
		m064.chr[5] = 5;
		m064.chr[6] = 6;
		m064.chr[7] = 7;
	}
}
void extcl_after_mapper_init_064(void) {
	prg_fix_064();
	chr_fix_064();
	mirroring_fix_064();
}
void extcl_cpu_wr_mem_064(WORD address, BYTE value) {
	switch (address & 0xF001) {
		case 0x8000:
			m064.index = value;
			prg_fix_064();
			chr_fix_064();
			mirroring_fix_064();
			return;
		case 0x8001: {
			BYTE index = m064.index & 0x0F;

			switch (index) {
				case 0x00:
				case 0x01:
				case 0x02:
				case 0x03:
				case 0x04:
				case 0x05:
					m064.chr[index] = value;
					chr_fix_064();
					mirroring_fix_064();
					return;
				case 0x06:
				case 0x07:
					m064.prg[index & 0x01] = value;
					prg_fix_064();
					return;
				case 0x08:
				case 0x09:
					m064.chr[index - 2] = value;
					chr_fix_064();
					mirroring_fix_064();
					return;
				case 0x0F:
					m064.prg[2] = value;
					prg_fix_064();
					return;
			}
			return;
		}
		case 0xA000:
			m064.mirroring = value;
			mirroring_fix_064();
			return;
		case 0xC000:
			m064.irq.latch = value;
			return;
		case 0xC001:
			m064.irq.mode =!!(value & 0x01);
			m064.irq.prescaler = 0;
			m064.irq.counter = 0;
			m064.irq.reload = TRUE;
			m064.irq.plus_clock = m064.irq.a12_filter ? 0 : 1;
			return;
		case 0xE000:
			m064.irq.enable = FALSE;
			return;
		case 0xE001:
			m064.irq.enable = TRUE;
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_064(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m064.prg);
	save_slot_ele(mode, slot, m064.chr);
	save_slot_ele(mode, slot, m064.index);
	save_slot_ele(mode, slot, m064.mirroring);
	save_slot_ele(mode, slot, m064.irq.mode);
	save_slot_ele(mode, slot, m064.irq.enable);
	save_slot_ele(mode, slot, m064.irq.prescaler);
	save_slot_ele(mode, slot, m064.irq.plus_clock);
	save_slot_ele(mode, slot, m064.irq.latch);
	save_slot_ele(mode, slot, m064.irq.delay);
	save_slot_ele(mode, slot, m064.irq.counter);
	save_slot_ele(mode, slot, m064.irq.reload);
	save_slot_ele(mode, slot, m064.irq.a12);
	save_slot_ele(mode, slot, m064.irq.a12_filter);

	return (EXIT_OK);
}
void extcl_ppu_000_to_255_064(void) {
	if (ppudata.r2001.visible) {
		extcl_ppu_320_to_34x_064();
	}
}
void extcl_ppu_256_to_319_064(void) {
	if ((ppudata.ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if ((!ppudata.spr_ev.count_plus || (ppudata.spr_ev.tmp_spr_plus == ppudata.spr_ev.count_plus)) && (ppudata.r2000.size_spr == 16)) {
		ppudata.ppu.spr_adr = ppudata.r2000.spt_adr;
	} else {
		ppu_spr_adr((ppudata.ppu.frame_x & 0x0038) >> 3);
	}

	m064.irq.a12 = (ppudata.ppu.spr_adr & 0x1000) >> 12;
}
void extcl_ppu_320_to_34x_064(void) {
	if ((ppudata.ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if (ppudata.ppu.frame_x == 323) {
		ppu_spr_adr(7);
	}

	ppu_bck_adr(ppudata.r2000.bpt_adr, ppudata.r2006.value);

	m064.irq.a12 = (ppudata.ppu.bck_adr & 0x1000) >> 12;
}
void extcl_update_r2006_064(WORD new_r2006, UNUSED(WORD old_r2006)) {
	m064.irq.a12 = (new_r2006 & 0x1000) >> 12;
}
void extcl_cpu_every_cycle_064(void) {
	if (m064.irq.delay && !(--m064.irq.delay)) {
		irq.high |= EXT_IRQ;
	}
	m064.irq.prescaler++;
	if (!(m064.irq.prescaler & 0x03) && (m064.irq.mode == CPU_MODE)) {
		irq_clock_064();
	}
	if (m064.irq.a12) {
		if (!m064.irq.a12_filter && (m064.irq.mode == A12_MODE)) {
			irq_clock_064();
		}
		m064.irq.a12_filter = 16;
	} else if (m064.irq.a12_filter) {
		m064.irq.a12_filter--;
	}
	if (!m064.irq.enable) {
		irq.high &= ~EXT_IRQ;
	}
}

INLINE static void prg_fix_064(void) {
	if (m064.index & 0x40) {
		memmap_auto_8k(MMCPU(0x8000), m064.prg[2]);
		memmap_auto_8k(MMCPU(0xA000), m064.prg[0]);
		memmap_auto_8k(MMCPU(0xC000), m064.prg[1]);
	} else {
		memmap_auto_8k(MMCPU(0x8000), m064.prg[0]);
		memmap_auto_8k(MMCPU(0xA000), m064.prg[1]);
		memmap_auto_8k(MMCPU(0xC000), m064.prg[2]);
	}
	memmap_auto_8k(MMCPU(0xE000), 0xFF);
}
INLINE static void chr_fix_064(void) {
	WORD swap = (m064.index & 0x80) << 5;

	if (m064.index & 0x20) {
		memmap_auto_1k(MMPPU(0x0000 ^ swap), m064.chr[0]);
		memmap_auto_1k(MMPPU(0x0400 ^ swap), m064.chr[6]);
		memmap_auto_1k(MMPPU(0x0800 ^ swap), m064.chr[1]);
		memmap_auto_1k(MMPPU(0x0C00 ^ swap), m064.chr[7]);
	} else {
		memmap_auto_2k(MMPPU(0x0000 ^ swap), (m064.chr[0] >> 1));
		memmap_auto_2k(MMPPU(0x0800 ^ swap), (m064.chr[1] >> 1));
	}
	memmap_auto_1k(MMPPU(0x1000 ^ swap), m064.chr[2]);
	memmap_auto_1k(MMPPU(0x1400 ^ swap), m064.chr[3]);
	memmap_auto_1k(MMPPU(0x1800 ^ swap), m064.chr[4]);
	memmap_auto_1k(MMPPU(0x1C00 ^ swap), m064.chr[5]);
}
INLINE static void mirroring_fix_064(void) {
	if (info.mapper.id == 158) {
		WORD swap = (m064.index & 0x80) << 5;

		if (m064.index & 0x20) {
			memmap_nmt_1k(MMPPU(0x2000 ^ swap), (m064.chr[0] >> 7));
			memmap_nmt_1k(MMPPU(0x2400 ^ swap), (m064.chr[6] >> 7));
			memmap_nmt_1k(MMPPU(0x2800 ^ swap), (m064.chr[1] >> 7));
			memmap_nmt_1k(MMPPU(0x2C00 ^ swap), (m064.chr[7] >> 7));
		} else {
			memmap_nmt_1k(MMPPU(0x2000 ^ swap), (m064.chr[0] >> 7));
			memmap_nmt_1k(MMPPU(0x2400 ^ swap), (m064.chr[0] >> 7));
			memmap_nmt_1k(MMPPU(0x2800 ^ swap), (m064.chr[1] >> 7));
			memmap_nmt_1k(MMPPU(0x2C00 ^ swap), (m064.chr[1] >> 7));
		}
		memmap_nmt_1k(MMPPU(0x3000 ^ swap), (m064.chr[2] >> 7));
		memmap_nmt_1k(MMPPU(0x3400 ^ swap), (m064.chr[3] >> 7));
		memmap_nmt_1k(MMPPU(0x3800 ^ swap), (m064.chr[4] >> 7));
		memmap_nmt_1k(MMPPU(0x3C00 ^ swap), (m064.chr[5] >> 7));
	} else if (m064.mirroring & 0x01) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}

INLINE static void irq_clock_064(void) {
	if (!m064.irq.counter) {
		m064.irq.counter = m064.irq.latch + (m064.irq.reload ? m064.irq.plus_clock : 0);
		if (!m064.irq.counter && m064.irq.reload && m064.irq.enable) {
			// il + 1 e' il solito ritardo
			m064.irq.delay = 1 + 1;
		}
	} else if (!(--m064.irq.counter) && m064.irq.enable) {
		// il + 1 e' il solito ritardo
		m064.irq.delay = 1 + 1;
	}
	m064.irq.reload = FALSE;
}
