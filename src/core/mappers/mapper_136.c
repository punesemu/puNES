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

#include "mappers.h"

void prg_fix_jv001_136(void);
void chr_fix_jv001_136(void);

void map_init_136(void) {
	EXTCL_AFTER_MAPPER_INIT(JV001);
	EXTCL_CPU_WR_MEM(136);
	EXTCL_CPU_RD_MEM(136);
	EXTCL_SAVE_MAPPER(JV001);
	mapper.internal_struct[0] = (BYTE *)&jv001;
	mapper.internal_struct_size[0] = sizeof(jv001);

	init_JV001();
	JV001_prg_fix = prg_fix_jv001_136;
	JV001_chr_fix = chr_fix_jv001_136;
}
void extcl_cpu_wr_mem_136(WORD address, BYTE value) {
	extcl_cpu_wr_mem_JV001(address, (value & 0x3F));
}
BYTE extcl_cpu_rd_mem_136(WORD address, BYTE openbus) {
	if ((address >= 0x4020) && (address <= 0x5FFF)) {
		BYTE value = extcl_cpu_rd_mem_JV001(address, openbus);

		return (((openbus & 0xC0) | (value & 0x3F)));
	}
	return (openbus);
}

void prg_fix_jv001_136(void) {
	memmap_auto_32k(MMCPU(0x8000), (jv001.output >> 4));
}
void chr_fix_jv001_136(void) {
	memmap_auto_8k(MMPPU(0x0000), jv001.output);
}
