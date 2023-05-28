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
#include "mem_map.h"
#include "info.h"
#include "cpu.h"
#include "save_slot.h"

/* TODO : aggiungere l'emulazione del D7756C */

INLINE static void prg_fix_018(void);
INLINE static void chr_fix_018(void);
INLINE static void wram_fix_018(void);
INLINE static void mirroring_fix_018(void);

struct _m018 {
	WORD prg[4];
	WORD chr[8];
	BYTE mirroring;
	struct _m108_irq {
		BYTE enabled;
		WORD reload;
		WORD count;
		BYTE delay;
	} irq;
} m018;

void map_init_018(void) {
	EXTCL_AFTER_MAPPER_INIT(018);
	EXTCL_CPU_WR_MEM(018);
	EXTCL_SAVE_MAPPER(018);
	EXTCL_CPU_EVERY_CYCLE(018);

	if (info.reset >= HARD) {
		memset(&m018, 0x00, sizeof(m018));

		m018.prg[0] = 0x00;
		m018.prg[1] = 0x01;
		m018.prg[2] = 0xFE;
		m018.prg[3] = 0x00;
	}
}
void extcl_after_mapper_init_018(void) {
	prg_fix_018();
	chr_fix_018();
	wram_fix_018();
	mirroring_fix_018();
}
void extcl_cpu_wr_mem_018(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
		case 0x9000: {
			BYTE reg = ((address & 0x1000) >> 11) | ((address & 0x0002) >> 1);

			if (address & 0x01) {
				m018.prg[reg] = (m018.prg[reg] & 0x0F) | (value << 4);
			} else {
				m018.prg[reg] = (m018.prg[reg] & 0xF0) | (value & 0x0F);
			}
			prg_fix_018();
			wram_fix_018();
			return;
		}
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000: {
			BYTE reg = (((address - 0xA000) & 0x3000) >> 11) | ((address & 0x0002) >> 1);

			if (address & 0x01) {
				m018.chr[reg] = (m018.chr[reg] & 0x0F) | (value << 4);
			} else {
				m018.chr[reg] = (m018.chr[reg] & 0xF0) | (value & 0x0F);
			}
			chr_fix_018();
			return;
		}
		case 0xE000:
			switch (address & 0x03) {
				case 0:
					m018.irq.reload = (m018.irq.reload & 0xFFF0) | (value & 0x0F);
					return;
				case 1:
					m018.irq.reload = (m018.irq.reload & 0xFF0F) | (value & 0x0F) << 4;
					return;
				case 2:
					m018.irq.reload = (m018.irq.reload & 0xF0FF) | (value & 0x0F) << 8;
					return;
				case 3:
					m018.irq.reload = (m018.irq.reload & 0x0FFF) | (value & 0x0F) << 12;
					return;
			}
			return;
		case 0xF000:
			switch (address & 0x03) {
				case 0:
					m018.irq.count = m018.irq.reload;
					irq.high &= ~EXT_IRQ;
					return;
				case 1:
					m018.irq.enabled = value;
					irq.high &= ~EXT_IRQ;
					return;
				case 2:
					m018.mirroring = value;
					mirroring_fix_018();
					return;
				case 3:
					return;
			}
			return;
	}
}
BYTE extcl_save_mapper_018(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m018.prg);
	save_slot_ele(mode, slot, m018.chr);
	save_slot_ele(mode, slot, m018.mirroring);
	save_slot_ele(mode, slot, m018.irq.enabled);
	save_slot_ele(mode, slot, m018.irq.reload);
	save_slot_ele(mode, slot, m018.irq.count);
	save_slot_ele(mode, slot, m018.irq.delay);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_018(void) {
	// gestisco questo delay sempre per la sincronizzazzione con la CPU
	if (m018.irq.delay && !(--m018.irq.delay)) {
		irq.delay = TRUE;
		irq.high |= EXT_IRQ;
	}

	if (m018.irq.enabled & 0x01) {
		WORD mask = 0xFFFF;

		if (m018.irq.enabled & 0x08) {
			mask = 0x000F;
		} else if (m018.irq.enabled & 0x04) {
			mask = 0x00FF;
		} else if (m018.irq.enabled & 0x02) {
			mask = 0x0FFF;
		}
		if ((m018.irq.count & mask) && !(--m018.irq.count & mask)) {
			m018.irq.delay = 1;
		}
	}
}

INLINE static void prg_fix_018(void) {
	memmap_auto_8k(MMCPU(0x8000), m018.prg[0]);
	memmap_auto_8k(MMCPU(0xA000), m018.prg[1]);
	memmap_auto_8k(MMCPU(0xC000), m018.prg[2]);
	memmap_auto_8k(MMCPU(0xE000), 0xFF);
}
INLINE static void chr_fix_018(void) {
	memmap_auto_1k(MMPPU(0x0000), m018.chr[0]);
	memmap_auto_1k(MMPPU(0x0400), m018.chr[1]);
	memmap_auto_1k(MMPPU(0x0800), m018.chr[2]);
	memmap_auto_1k(MMPPU(0x0C00), m018.chr[3]);
	memmap_auto_1k(MMPPU(0x1000), m018.chr[4]);
	memmap_auto_1k(MMPPU(0x1400), m018.chr[5]);
	memmap_auto_1k(MMPPU(0x1800), m018.chr[6]);
	memmap_auto_1k(MMPPU(0x1C00), m018.chr[7]);
}
INLINE static void wram_fix_018(void) {
	BYTE rd = m018.prg[3] & 0x01;
	BYTE wr = rd ? (m018.prg[3] & 0x02) : FALSE;

	// 7  bit  0
	// ---------
	// .... ..WR
	//	      ||
	//        |+- PRG RAM chip enable (0: disable; 1: enable) (same as MMC3)
	//        +-- Write protection (0: deny writes; 1: allow writes) (opposite MMC3)
	memmap_auto_wp_8k(MMCPU(0x6000), 0, rd, wr);
}
INLINE static void mirroring_fix_018(void) {
	switch (m018.mirroring & 0x03) {
		case 0:
			mirroring_H();
			return;
		case 1:
			mirroring_V();
			return;
		case 2:
			mirroring_SCR0();
			return;
		case 3:
			mirroring_SCR1();
			return;
	}
}
