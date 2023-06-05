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

void prg_fix_txc_173(void);
void chr_fix_txc_173(void);

void map_init_173(void) {
	EXTCL_AFTER_MAPPER_INIT(TXC);
	EXTCL_CPU_WR_MEM(173);
	EXTCL_CPU_RD_MEM(173);
	EXTCL_SAVE_MAPPER(TXC);
	mapper.internal_struct[0] = (BYTE *)&txc;
	mapper.internal_struct_size[0] = sizeof(txc);

	init_TXC();
	TXC_prg_fix = prg_fix_txc_173;
	TXC_chr_fix = chr_fix_txc_173;
}
void extcl_cpu_wr_mem_173(WORD address, BYTE value) {
	extcl_cpu_wr_mem_TXC(address, (value & 0x0F));
}
BYTE extcl_cpu_rd_mem_173(WORD address, BYTE openbus) {
	if ((address >= 0x4020) && (address <= 0x5FFF)) {
		BYTE value = extcl_cpu_rd_mem_TXC(address, openbus);

		return (((openbus & 0xF0) | (value & 0x0F)));
	}
	return (openbus);
}

void prg_fix_txc_173(void) {
	memmap_auto_32k(MMCPU(0x8000), 0);
}
void chr_fix_txc_173(void) {
	if (chrrom_size() >= S16K) {
		memmap_auto_8k(MMPPU(0x0000), (((txc.output & 0x02) << 1) | (txc.Y << 1) | (txc.output & 0x01)));
	} else if (txc.Y) {
		memmap_auto_8k(MMPPU(0x0000), 0);
	} else {
		memmap_disable_8k(MMPPU(0x0000));
	}
}
