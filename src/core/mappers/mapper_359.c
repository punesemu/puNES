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
#include "ppu.h"
#include "save_slot.h"
#include "ppu_inline.h"

INLINE static void prg_fix_359(void);
INLINE static void chr_fix_359(void);
INLINE static void wram_fix_359(void);
INLINE static void mirroring_fix_359(void);

INLINE static void irq_clock_359(void);

struct _m359 {
	BYTE prg[6];
	BYTE chr[10];
	BYTE mirroring;
	struct _m359_irq {
		BYTE mode;
		BYTE auto_enable;
		BYTE enable;
		BYTE reload;
		BYTE a12_filter;
		union _m359_irq_counter {
			BYTE b[2];
			WORD w[1];
		} counter;
	} irq;
} m359;

void map_init_359(void) {
	EXTCL_CPU_WR_MEM(359);
	EXTCL_AFTER_MAPPER_INIT(359);
	EXTCL_CPU_WR_MEM(359);
	EXTCL_SAVE_MAPPER(359);
	EXTCL_PPU_000_TO_255(359);
	EXTCL_PPU_256_TO_319(359);
	EXTCL_PPU_320_TO_34X(359);
	EXTCL_UPDATE_R2006(359);
	EXTCL_CPU_EVERY_CYCLE(359);
	mapper.internal_struct[0] = (BYTE *)&m359;
	mapper.internal_struct_size[0] = sizeof(m359);

	if (info.reset >= HARD) {
		memset(&m359, 0x00, sizeof(m359));

		m359.prg[0] = 0xFC;
		m359.prg[1] = 0xFD;
		m359.prg[2] = 0xFE;
		m359.prg[2] = 0xFF;
		m359.prg[4] = 0x00;
		m359.prg[5] = 0x3F;

		m359.chr[1] = 0x01;
		m359.chr[2] = 0x02;
		m359.chr[3] = 0x03;
		m359.chr[4] = 0x04;
		m359.chr[5] = 0x05;
		m359.chr[6] = 0x06;
		m359.chr[7] = 0x07;
		m359.chr[8] = 0x00;
		m359.chr[9] = 0xFF;
	}
}
void extcl_after_mapper_init_359(void) {
	prg_fix_359();
	chr_fix_359();
	wram_fix_359();
	mirroring_fix_359();
}
void extcl_cpu_wr_mem_359(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			m359.prg[address & 0x03] = value;
			prg_fix_359();
			wram_fix_359();
			break;
		case 0x9000:
			switch (address & 0x03) {
				case 0:
					m359.prg[4] = value;
					prg_fix_359();
					wram_fix_359();
					break;
				case 1:
					switch (value & 0x03) {
						case 0:
							m359.prg[5] = 0x3F;
							break;
						case 1:
							m359.prg[5] = 0x1F;
							break;
						case 2:
							m359.prg[5] = 0x2F;
							break;
						case 3:
							m359.prg[5] = 0x0F;
							break;
					}
					m359.chr[9] = value & 0x40 ? 0xFF : 0x7F;
					prg_fix_359();
					wram_fix_359();
					chr_fix_359();
					break;
				case 2:
					m359.mirroring = value;
					mirroring_fix_359();
					break;
				case 3:
					m359.chr[8] = value;
					chr_fix_359();
					break;
			}
			break;
		case 0xA000:
		case 0xB000:
			m359.chr[((address & 0x1000) >> 10) | (address & 0x03)] = value;
			chr_fix_359();
			break;
		case 0xC000:
			switch (address & 0x03) {
				case 0:
					if (m359.irq.auto_enable) {
						m359.irq.enable = FALSE;
					}
					m359.irq.counter.b[0] = value;
					break;
				case 1:
					if (m359.irq.auto_enable) {
						m359.irq.enable = TRUE;
					}
					m359.irq.counter.b[1] = value;
					m359.irq.reload = TRUE;
					break;
				case 2:
					m359.irq.enable = value & 0x01;
					m359.irq.mode = value & 0x02;
					m359.irq.auto_enable = value & 0x04;
					break;
				case 3:
					m359.irq.enable = value & 0x01;
					break;
			}
			cpudata.irq.high &= ~EXT_IRQ;
			break;
	}
}
BYTE extcl_save_mapper_359(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m359.prg);
	save_slot_ele(mode, slot, m359.chr);
	save_slot_ele(mode, slot, m359.mirroring);
	save_slot_ele(mode, slot, m359.irq.mode);
	save_slot_ele(mode, slot, m359.irq.auto_enable);
	save_slot_ele(mode, slot, m359.irq.enable);
	save_slot_ele(mode, slot, m359.irq.reload);
	save_slot_ele(mode, slot, m359.irq.a12_filter);
	save_slot_ele(mode, slot, m359.irq.counter.w);

	return (EXIT_OK);
}
void extcl_ppu_000_to_255_359(void) {
	if (ppudata.r2001.visible) {
		extcl_ppu_320_to_34x_359();
	}
}
void extcl_ppu_256_to_319_359(void) {
	if ((ppudata.ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if ((!ppudata.spr_ev.count_plus || (ppudata.spr_ev.tmp_spr_plus == ppudata.spr_ev.count_plus)) && (ppudata.r2000.size_spr == 16)) {
		ppudata.ppu.spr_adr = ppudata.r2000.spt_adr;
	} else {
		ppu_spr_adr((ppudata.ppu.frame_x & 0x0038) >> 3);
	}

	if ((ppudata.ppu.spr_adr & 0x1000) > (ppudata.ppu.bck_adr & 0x1000)) {
		irq_clock_359();
	}
}
void extcl_ppu_320_to_34x_359(void) {
	if ((ppudata.ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if (ppudata.ppu.frame_x == 323) {
		ppu_spr_adr(7);
	}

	ppu_bck_adr(ppudata.r2000.bpt_adr, ppudata.r2006.value);

	if ((ppudata.ppu.bck_adr & 0x1000) > (ppudata.ppu.spr_adr & 0x1000)) {
		irq_clock_359();
	}
}
void extcl_update_r2006_359(WORD new_r2006, UNUSED(WORD old_r2006)) {
	if ((new_r2006 & 0x1000) > (old_r2006 & 0x1000)) {
		irq_clock_359();
	}
}
void extcl_cpu_every_cycle_359(void) {
	if (m359.irq.a12_filter) {
		m359.irq.a12_filter--;
	}
	if (m359.irq.enable && !m359.irq.mode && m359.irq.counter.w[0] && !--m359.irq.counter.w[0]){
		cpudata.irq.high |= EXT_IRQ;
	}
}

INLINE static void prg_fix_359(void) {
	WORD base = (m359.prg[4] & 0x38) << 1;
	WORD mask = m359.prg[5];

	memmap_auto_8k(MMCPU(0x8000), (base | (m359.prg[0] & mask)));
	memmap_auto_8k(MMCPU(0xA000), (base | (m359.prg[1] & mask)));
	memmap_auto_8k(MMCPU(0xC000), (base | (m359.prg[2] & mask)));
	memmap_auto_8k(MMCPU(0xE000), (base | (0xFF & mask)));
}
INLINE static void chr_fix_359(void) {
	if (chrrom_size()) {
		if (info.mapper.id == 540) {
			memmap_auto_2k(MMPPU(0x0000), m359.chr[0]);
			memmap_auto_2k(MMPPU(0x0800), m359.chr[1]);
			memmap_auto_2k(MMPPU(0x1000), m359.chr[6]);
			memmap_auto_2k(MMPPU(0x1800), m359.chr[7]);
		} else {
			WORD base = m359.chr[8] << 7;
			WORD mask = m359.chr[9];

			memmap_auto_1k(MMPPU(0x0000), (base | (m359.chr[0] & mask)));
			memmap_auto_1k(MMPPU(0x0400), (base | (m359.chr[1] & mask)));
			memmap_auto_1k(MMPPU(0x0800), (base | (m359.chr[2] & mask)));
			memmap_auto_1k(MMPPU(0x0C00), (base | (m359.chr[3] & mask)));
			memmap_auto_1k(MMPPU(0x1000), (base | (m359.chr[4] & mask)));
			memmap_auto_1k(MMPPU(0x1400), (base | (m359.chr[5] & mask)));
			memmap_auto_1k(MMPPU(0x1800), (base | (m359.chr[6] & mask)));
			memmap_auto_1k(MMPPU(0x1C00), (base | (m359.chr[7] & mask)));
		}
	}
}
INLINE static void wram_fix_359(void) {
	WORD base = (m359.prg[4] & 0x38) << 1;
	WORD mask = m359.prg[5];

	memmap_prgrom_8k(MMCPU(0x6000), (base | (m359.prg[3] & mask)));
}
INLINE static void mirroring_fix_359(void) {
	switch (m359.mirroring & 0x03) {
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

INLINE static void irq_clock_359(void) {
	if (!m359.irq.a12_filter && m359.irq.mode) {
		if (!m359.irq.counter.b[0] || m359.irq.reload) {
			m359.irq.counter.b[0] = m359.irq.counter.b[1];
		} else {
			m359.irq.counter.b[0]--;
		}
		if (!m359.irq.counter.b[0] && m359.irq.enable) {
			cpudata.irq.high |= EXT_IRQ;
		}
		m359.irq.reload = FALSE;
	}
	m359.irq.a12_filter = 5;
}

