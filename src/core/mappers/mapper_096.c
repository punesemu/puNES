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

INLINE static void prg_fix_096(void);
INLINE static void chr_fix_096(void);

struct _m096 {
	BYTE reg;
	BYTE chr;
} m096;

void map_init_096(void) {
	EXTCL_AFTER_MAPPER_INIT(096);
	EXTCL_CPU_WR_MEM(096);
	EXTCL_SAVE_MAPPER(096);
	EXTCL_UPDATE_R2006(096);
	EXTCL_RD_NMT(096);
	map_internal_struct_init((BYTE *)&m096, sizeof(m096));

	if (info.reset >= HARD) {
		memset(&m096, 0x00, sizeof(m096));
	}
}
void extcl_after_mapper_init_096(void) {
	prg_fix_096();
	chr_fix_096();
}
void extcl_cpu_wr_mem_096(BYTE nidx, WORD address, BYTE value) {
	// bus conflict
	m096.reg = value & prgrom_rd(nidx, address);
	prg_fix_096();
	chr_fix_096();
}
BYTE extcl_save_mapper_096(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m096.reg);
	save_slot_ele(mode, slot, m096.chr);
	return (EXIT_OK);
}
void extcl_update_r2006_096(UNUSED(BYTE nidx), WORD new_r2006, UNUSED(WORD old_r2006)) {
	if ((new_r2006 >= 0x2000) && ((new_r2006 & 0x03FF) < 0x03C0)) {
		BYTE value = (m096.chr & 0x04) | ((new_r2006 >> 8) & 0x03);

		if (m096.chr != value) {
			m096.chr = value;
			chr_fix_096();
		}
	}
}
BYTE extcl_rd_nmt_096(BYTE nidx, WORD address) {
	if ((address & 0x03FF) < 0x03C0) {
		BYTE value = (m096.chr & 0x04) | ((address >> 8) & 0x03);

		if (m096.chr != value) {
			m096.chr = value;
			chr_fix_096();
		}
	}
	return (nmt_rd(nidx, address));
}

INLINE static void prg_fix_096(void) {
	memmap_auto_32k(0, MMCPU(0x8000), (m096.reg & 0x03));
}
INLINE static void chr_fix_096(void) {
	memmap_auto_4k(0, MMPPU(0x0000), ((m096.reg & 0x04) | (m096.chr & 0x03)));
	memmap_auto_4k(0, MMPPU(0x1000), ((m096.reg & 0x04) | 0x03));
}
