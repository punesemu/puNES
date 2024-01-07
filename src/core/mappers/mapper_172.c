/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#include "mappers.h"
#include "info.h"

void prg_fix_jv001_172(void);
void chr_fix_jv001_172(void);
void mirroring_fix_jv001_172(void);

void map_init_172(void) {
	EXTCL_AFTER_MAPPER_INIT(JV001);
	EXTCL_CPU_WR_MEM(172);
	EXTCL_CPU_RD_MEM(172);
	EXTCL_SAVE_MAPPER(JV001);
	map_internal_struct_init((BYTE *)&jv001, sizeof(jv001));

	init_JV001(info.reset);
	JV001_prg_fix = prg_fix_jv001_172;
	JV001_chr_fix = chr_fix_jv001_172;
	JV001_mirroring_fix = mirroring_fix_jv001_172;
}
void extcl_cpu_wr_mem_172(BYTE nidx, WORD address, BYTE value) {
	extcl_cpu_wr_mem_JV001(nidx, address, (((value & 0x01) << 5) | ((value & 0x02) << 3) | ((value & 0x04) << 1) |
		((value & 0x08) >> 1) | ((value & 0x10) >> 3) | ((value & 0x20) >> 5)));
}
BYTE extcl_cpu_rd_mem_172(BYTE nidx, WORD address, BYTE openbus) {
	if ((address >= 0x4020) && (address <= 0x5FFF)) {
		BYTE value = extcl_cpu_rd_mem_JV001(nidx, address, openbus);

		return (((value & 0x01) << 5) | ((value & 0x02) << 3) |
			((value & 0x04) << 1) | ((value & 0x08) << 1) |
			((value & 0x10) >> 3) | ((value & 0x20) >> 5) |
			(openbus & 0xC0));
	}
	return (wram_rd(nidx, address));
}

void prg_fix_jv001_172(void) {
	memmap_auto_32k(0, MMCPU(0x8000), 0);
}
void chr_fix_jv001_172(void) {
	memmap_auto_8k(0, MMPPU(0x0000), jv001.output);
}
void mirroring_fix_jv001_172(void) {
	if (jv001.X) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
