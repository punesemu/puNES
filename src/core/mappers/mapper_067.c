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

INLINE static void prg_fix_067(void);
INLINE static void chr_fix_067(void);
INLINE static void mirroring_fix_067(void);

struct _m067 {
	WORD prg;
	WORD chr[4];
	BYTE mirroring;
	struct _m067_irq {
		BYTE enable;
		BYTE toggle;
		WORD count;
		BYTE delay;
	} irq;
} m067;

void map_init_067(void) {
	EXTCL_AFTER_MAPPER_INIT(067);
	EXTCL_CPU_WR_MEM(067);
	EXTCL_SAVE_MAPPER(067);
	EXTCL_CPU_EVERY_CYCLE(067);
	mapper.internal_struct[0] = (BYTE *)&m067;
	mapper.internal_struct_size[0] = sizeof(m067);

	if (info.reset >= HARD) {
		memset(&m067, 0x00, sizeof(m067));

		m067.chr[0] = 0;
		m067.chr[1] = 1;
		m067.chr[2] = 2;
		m067.chr[3] = 3;
	}
}
void extcl_after_mapper_init_067(void) {
	prg_fix_067();
	chr_fix_067();
	mirroring_fix_067();
}
void extcl_cpu_wr_mem_067(WORD address, BYTE value) {
	switch (address & 0xF800) {
		case 0x8800:
		case 0x9800:
		case 0xA800:
		case 0xB800:
			m067.chr[(address & 0x3000) >> 12] = value;
			chr_fix_067();
			return;
		case 0xC800:
			if (m067.irq.toggle) {
				m067.irq.count = (m067.irq.count & 0xFF00) | value;
			} else {
				m067.irq.count = (m067.irq.count & 0x00FF) | (value << 8);
			}
			m067.irq.toggle ^= 1;
			return;
		case 0xD800:
			m067.irq.toggle = 0;
			m067.irq.enable = value & 0x10;
			irq.high &= ~EXT_IRQ;
			return;
		case 0xE800:
			m067.mirroring = value;
			mirroring_fix_067();
			return;
		case 0xF800:
			m067.prg = value;
			prg_fix_067();
			return;
		default:
			irq.high &= ~EXT_IRQ;
			return;
	}
}
BYTE extcl_save_mapper_067(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m067.prg);
	save_slot_ele(mode, slot, m067.chr);
	save_slot_ele(mode, slot, m067.mirroring);
	save_slot_ele(mode, slot, m067.irq.enable);
	save_slot_ele(mode, slot, m067.irq.toggle);
	save_slot_ele(mode, slot, m067.irq.count);
	save_slot_ele(mode, slot, m067.irq.delay);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_067(void) {
	if (m067.irq.delay && !(--m067.irq.delay)) {
		irq.high |= EXT_IRQ;
	}

	if (m067.irq.enable && m067.irq.count && !(--m067.irq.count)) {
		m067.irq.enable = FALSE;
		m067.irq.count = 0xFFFF;
		m067.irq.delay = 1;
	}
}

INLINE static void prg_fix_067(void) {
	memmap_auto_16k(MMCPU(0x8000), m067.prg);
	memmap_auto_16k(MMCPU(0xC000), 0xFF);
}
INLINE static void chr_fix_067(void) {
	memmap_auto_2k(MMPPU(0x0000), m067.chr[0]);
	memmap_auto_2k(MMPPU(0x0800), m067.chr[1]);
	memmap_auto_2k(MMPPU(0x1000), m067.chr[2]);
	memmap_auto_2k(MMPPU(0x1800), m067.chr[3]);
}
INLINE static void mirroring_fix_067(void) {
	switch (m067.mirroring & 0x03) {
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
