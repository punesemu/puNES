/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

INLINE static void prg_fix_538(void);
INLINE static void wram_fix_538(void);

struct _m538 {
	BYTE reg;
} m538;

void map_init_538(void) {
	EXTCL_AFTER_MAPPER_INIT(538);
	EXTCL_CPU_WR_MEM(538);
	EXTCL_SAVE_MAPPER(538);
	map_internal_struct_init((BYTE *)&m538, sizeof(m538));

	if (info.reset >= HARD) {
		memset(&m538, 0x00, sizeof(m538));
	}
}
void extcl_after_mapper_init_538(void) {
	prg_fix_538();
	wram_fix_538();
}
void extcl_cpu_wr_mem_538(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0xC000) && (address <= 0xDFFF)) {
		m538.reg = value;
		prg_fix_538();
		wram_fix_538();
	}
}
BYTE extcl_save_mapper_538(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m538.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_538(void) {
	memmap_auto_8k(0, MMCPU(0x8000), ((m538.reg & 0x01) && (~m538.reg & 0x08) ? 0x0A : m538.reg & 0xFE));
	memmap_auto_8k(0, MMCPU(0xA000), 0x0D);
	memmap_auto_8k(0, MMCPU(0xC000), 0x0E);
	memmap_auto_8k(0, MMCPU(0xE000), 0x0F);
}
INLINE static void wram_fix_538(void) {
	memmap_prgrom_8k(0, MMCPU(0x6000), (m538.reg | 0x01));
}
