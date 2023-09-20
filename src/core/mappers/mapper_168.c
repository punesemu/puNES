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

INLINE static void prg_fix_168(void);
INLINE static void chr_fix_168(void);

struct _m168 {
	BYTE reg;
	BYTE chr_protect;
	struct m168_irq {
		BYTE disabled;
		WORD counter;
	} irq;
} m168;

void map_init_168(void) {
	EXTCL_AFTER_MAPPER_INIT(168);
	EXTCL_CPU_WR_MEM(168);
	EXTCL_SAVE_MAPPER(168);
	EXTCL_CPU_EVERY_CYCLE(168);
	mapper.internal_struct[0] = (BYTE *) &m168;
	mapper.internal_struct_size[0] = sizeof(m168);

	if (info.reset >= HARD) {
		memset(&m168, 0x00, sizeof(m168));

		m168.chr_protect = TRUE;
	}

	if (info.format != NES_2_0) {
		if (vram_nvram_size() < S32K) {
			vram_set_ram_size(0);
			vram_set_nvram_size(S64K);
		}
	}
}
void extcl_after_mapper_init_168(void) {
	prg_fix_168();
	chr_fix_168();
}
void extcl_cpu_wr_mem_168(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
			m168.reg = value;
			prg_fix_168();
			chr_fix_168();
			return;
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			if (m168.irq.disabled && !(address & 0x0080)) {
				m168.chr_protect = FALSE;
				chr_fix_168();
			}
			m168.irq.disabled = address & 0x0080;
			if (m168.irq.disabled) {
				cpudata.irq.high &= ~EXT_IRQ;
			}
			return;
	}
}
BYTE extcl_save_mapper_168(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m168.reg);
	save_slot_ele(mode, slot, m168.chr_protect);
	save_slot_ele(mode, slot, m168.irq.disabled);
	save_slot_ele(mode, slot, m168.irq.counter);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_168(void) {
	if (m168.irq.disabled) {
		m168.irq.counter = 0;
	} else {
		if (++m168.irq.counter & 1024) {
			cpudata.irq.high |= EXT_IRQ;
			return;
		}
		cpudata.irq.high &= ~EXT_IRQ;
	}
}

INLINE static void prg_fix_168() {
	memmap_auto_16k(MMCPU(0x8000), (m168.reg >> 6));
	memmap_auto_16k(MMCPU(0xC000), 0xFF);
}
INLINE static void chr_fix_168() {
	if (vram_nvram_size() >= S64K) {
		memmap_vram_wp_4k(MMPPU(0x0000), 0, !m168.chr_protect, !m168.chr_protect);
		memmap_vram_wp_4k(MMPPU(0x1000), (m168.reg & 0xF) ^ 0x08, !m168.chr_protect, !m168.chr_protect);
	} else {
		memmap_vram_4k(MMPPU(0x0000), 0);
		memmap_vram_wp_4k(MMPPU(0x1000), (m168.reg & 0xF) ^ 0x08, !m168.chr_protect, !m168.chr_protect);
	}
}
