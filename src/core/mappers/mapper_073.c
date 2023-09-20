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

INLINE static void prg_fix_073(void);

struct _m073 {
	BYTE prg;
	struct _m073_irq {
		BYTE enabled;
		BYTE mode;
		BYTE acknowledge;
		WORD reload;
		WORD mask;
		WORD count;
	} irq;
} m073;

void map_init_073(void) {
	EXTCL_AFTER_MAPPER_INIT(073);
	EXTCL_CPU_WR_MEM(073);
	EXTCL_SAVE_MAPPER(073);
	EXTCL_CPU_EVERY_CYCLE(073);
	mapper.internal_struct[0] = (BYTE *)&m073;
	mapper.internal_struct_size[0] = sizeof(m073);

	if (info.reset >= HARD) {
		memset(&m073, 0x00, sizeof(m073));
	}
}
void extcl_after_mapper_init_073(void) {
	prg_fix_073();
}
void extcl_cpu_wr_mem_073(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			m073.irq.reload = (m073.irq.reload & 0xFFF0) | (value & 0x0F);
			return;
		case 0x9000:
			m073.irq.reload = (m073.irq.reload & 0xFF0F) | ((value & 0x0F) << 4);
			return;
		case 0xA000:
			m073.irq.reload = (m073.irq.reload & 0xF0FF) | ((value & 0x0F) << 8);
			return;
		case 0xB000:
			m073.irq.reload = (m073.irq.reload & 0x0FFF) | ((value & 0x0F) << 12);
			return;
		case 0xC000:
			m073.irq.acknowledge = value & 0x01;
			m073.irq.enabled = value & 0x02;
			m073.irq.mode = value & 0x04;
			m073.irq.mask = 0xFFFF;
			if (m073.irq.mode) {
				m073.irq.mask = 0x00FF;
			}
			if (m073.irq.enabled) {
				m073.irq.count = m073.irq.reload;
			}
			nes.c.irq.high &= ~EXT_IRQ;
			return;
		case 0xD000:
			m073.irq.enabled = m073.irq.acknowledge;
			nes.c.irq.high &= ~EXT_IRQ;
			return;
		case 0xF000:
			m073.prg = value;
			prg_fix_073();
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_073(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m073.prg);
	save_slot_ele(mode, slot, m073.irq.enabled);
	save_slot_ele(mode, slot, m073.irq.reload);
	save_slot_ele(mode, slot, m073.irq.mode);
	save_slot_ele(mode, slot, m073.irq.acknowledge);
	save_slot_ele(mode, slot, m073.irq.mask);
	save_slot_ele(mode, slot, m073.irq.count);

	return (EXIT_OK);
}

INLINE static void prg_fix_073(void) {
	memmap_auto_16k(MMCPU(0x8000), m073.prg);
	memmap_auto_16k(MMCPU(0xC000), 0xFF);
}
void extcl_cpu_every_cycle_073(void) {
	if (!m073.irq.enabled) {
		return;
	}
	if (!(++m073.irq.count & m073.irq.mask)) {
		m073.irq.count = m073.irq.reload;
		nes.c.irq.delay = TRUE;
		nes.c.irq.high |= EXT_IRQ;
	}
}
