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
#include "ppu_inline.h"

INLINE static void prg_fix_117(void);
INLINE static void chr_fix_117(void);
INLINE static void wram_fix_117(void);
INLINE static void mirroring_fix_117(void);

INLINE static void irq_clock_117(BYTE nidx);

struct _m117 {
	BYTE prg[4];
	BYTE chr[8];
	BYTE mirroring;
	struct _m117_irq {
		BYTE mode;
		BYTE enable;
		BYTE reload;
		BYTE a12_filter;
		union _m117_irq_counter {
			BYTE b[2];
			WORD w[1];
		} counter;
	} irq;
} m117;

void map_init_117(void) {
	EXTCL_AFTER_MAPPER_INIT(117);
	EXTCL_CPU_WR_MEM(117);
	EXTCL_SAVE_MAPPER(117);
	EXTCL_PPU_000_TO_255(117);
	EXTCL_PPU_256_TO_319(117);
	EXTCL_PPU_320_TO_34X(117);
	EXTCL_UPDATE_R2006(117);
	EXTCL_CPU_EVERY_CYCLE(117);
	map_internal_struct_init((BYTE *)&m117, sizeof(m117));

	if (info.reset >= HARD) {
		memset(&m117, 0x00, sizeof(m117));

		m117.prg[0] = 0xFC;
		m117.prg[1] = 0xFD;
		m117.prg[2] = 0xFE;
		m117.prg[3] = 0xFF;

		m117.chr[0] = 0;
		m117.chr[0] = 1;
		m117.chr[0] = 2;
		m117.chr[0] = 3;
		m117.chr[0] = 4;
		m117.chr[0] = 5;
		m117.chr[0] = 6;
		m117.chr[0] = 7;
	}
}
void extcl_after_mapper_init_117(void) {
	prg_fix_117();
	chr_fix_117();
	wram_fix_117();
	mirroring_fix_117();
}
void extcl_cpu_wr_mem_117(BYTE nidx, WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			m117.prg[address & 0x03] = value;
			prg_fix_117();
			return;
		case 0xA000:
			if (!(address & 0x0008)) {
				m117.chr[address & 0x07] = value;
				chr_fix_117();
			}
			return;
		case 0xC000:
			switch (address & 0x03) {
				case 0:
					m117.irq.counter.b[0] = value;
					break;
				case 1:
					m117.irq.counter.b[1] = value;
					m117.irq.reload = TRUE;
					break;
				case 2:
					m117.irq.enable = FALSE;
					break;
				case 3:
					m117.irq.enable = TRUE;
					break;
			}
			nes[nidx].c.irq.high &= ~EXT_IRQ;
			return;
		case 0xD000:
			m117.mirroring = value;
			mirroring_fix_117();
			return;
		case 0xE000:
			m117.irq.mode = value;
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_117(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m117.prg);
	save_slot_ele(mode, slot, m117.chr);
	save_slot_ele(mode, slot, m117.mirroring);
	save_slot_ele(mode, slot, m117.irq.mode);
	save_slot_ele(mode, slot, m117.irq.enable);
	save_slot_ele(mode, slot, m117.irq.reload);
	save_slot_ele(mode, slot, m117.irq.a12_filter);
	save_slot_ele(mode, slot, m117.irq.counter.w);
	return (EXIT_OK);
}
void extcl_ppu_000_to_255_117(BYTE nidx) {
	if (nes[nidx].p.r2001.visible) {
		extcl_ppu_320_to_34x_117(nidx);
	}
}
void extcl_ppu_256_to_319_117(BYTE nidx) {
	if ((nes[nidx].p.ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if ((!nes[nidx].p.spr_ev.count_plus || (nes[nidx].p.spr_ev.tmp_spr_plus == nes[nidx].p.spr_ev.count_plus)) && (nes[nidx].p.r2000.size_spr == 16)) {
		nes[nidx].p.ppu.spr_adr = nes[nidx].p.r2000.spt_adr;
	} else {
		ppu_spr_adr((nes[nidx].p.ppu.frame_x & 0x0038) >> 3);
	}

	if ((nes[nidx].p.ppu.spr_adr & 0x1000) > (nes[nidx].p.ppu.bck_adr & 0x1000)) {
		irq_clock_117(nidx);
	}
}
void extcl_ppu_320_to_34x_117(BYTE nidx) {
	if ((nes[nidx].p.ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if (nes[nidx].p.ppu.frame_x == 323) {
		ppu_spr_adr(7);
	}

	ppu_bck_adr(nes[nidx].p.r2000.bpt_adr, nes[nidx].p.r2006.value);

	if ((nes[nidx].p.ppu.bck_adr & 0x1000) > (nes[nidx].p.ppu.spr_adr & 0x1000)) {
		irq_clock_117(nidx);
	}
}
void extcl_update_r2006_117(BYTE nidx, WORD new_r2006, UNUSED(WORD old_r2006)) {
	if ((new_r2006 & 0x1000) > (old_r2006 & 0x1000)) {
		irq_clock_117(nidx);
	}
}
void extcl_cpu_every_cycle_117(BYTE nidx) {
	if (m117.irq.a12_filter) {
		m117.irq.a12_filter--;
	}
	if (m117.irq.enable && !(m117.irq.mode & 0x02) && m117.irq.counter.w[0] && !--m117.irq.counter.w[0]){
		nes[nidx].c.irq.high |= EXT_IRQ;
	}
}

INLINE static void prg_fix_117(void) {
	memmap_auto_8k(0, MMCPU(0x8000), m117.prg[0]);
	memmap_auto_8k(0, MMCPU(0xA000), m117.prg[1]);
	memmap_auto_8k(0, MMCPU(0xC000), m117.prg[2]);
	memmap_auto_8k(0, MMCPU(0xE000), 0xFF);
}
INLINE static void chr_fix_117(void) {
	memmap_auto_1k(0, MMPPU(0x0000), m117.chr[0]);
	memmap_auto_1k(0, MMPPU(0x0400), m117.chr[1]);
	memmap_auto_1k(0, MMPPU(0x0800), m117.chr[2]);
	memmap_auto_1k(0, MMPPU(0x0C00), m117.chr[3]);
	memmap_auto_1k(0, MMPPU(0x1000), m117.chr[4]);
	memmap_auto_1k(0, MMPPU(0x1400), m117.chr[5]);
	memmap_auto_1k(0, MMPPU(0x1800), m117.chr[6]);
	memmap_auto_1k(0, MMPPU(0x1C00), m117.chr[7]);
}
INLINE static void wram_fix_117(void) {
	memmap_prgrom_8k(0, MMCPU(0x6000), m117.prg[3]);
}
INLINE static void mirroring_fix_117(void) {
	switch (m117.mirroring & 0x03) {
		case 0:
			mirroring_V(0);
			return;
		case 1:
			mirroring_H(0);
			return;
		case 2:
			mirroring_SCR0(0);
			return;
		case 3:
			mirroring_SCR1(0);
			return;
	}
}

INLINE static void irq_clock_117(BYTE nidx) {
	if (m117.irq.mode & 0x01) {
		if (!m117.irq.a12_filter && (m117.irq.mode & 0x02)) {
			if (!m117.irq.counter.b[0] || m117.irq.reload) {
				m117.irq.counter.b[0] = m117.irq.counter.b[1];
			} else {
				m117.irq.counter.b[0]--;
			}
			if (!m117.irq.counter.b[0] && m117.irq.enable) {
				nes[nidx].c.irq.high |= EXT_IRQ;
			}
			m117.irq.reload = FALSE;
		}
		m117.irq.a12_filter = 5;
	}
}
