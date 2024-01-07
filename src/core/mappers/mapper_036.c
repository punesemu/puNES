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

#include <string.h>
#include "mappers.h"
#include "save_slot.h"

void prg_fix_txc_036(void);
void chr_fix_txc_036(void);

struct _m036 {
	BYTE reg;
} m036;

void map_init_036(void) {
	EXTCL_AFTER_MAPPER_INIT(TXC);
	EXTCL_CPU_WR_MEM(036);
	EXTCL_CPU_RD_MEM(036);
	EXTCL_SAVE_MAPPER(036);
	map_internal_struct_init((BYTE *)&txc, sizeof(txc));

	if (info.reset >= HARD) {
		memset(&m036, 0x00, sizeof(m036));
	}

	init_TXC(info.reset);
	TXC_prg_fix = prg_fix_txc_036;
	TXC_chr_fix = chr_fix_txc_036;
}
void extcl_cpu_wr_mem_036(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x4FFF) && (address & 0x200)) {
		m036.reg = value;
	}
	extcl_cpu_wr_mem_TXC(nidx, address, ((value & 0x30) >> 4));
}
BYTE extcl_cpu_rd_mem_036(BYTE nidx, WORD address, BYTE openbus) {
	if ((address >= 0x4020) && (address <= 0x5FFF)) {
		BYTE value = extcl_cpu_rd_mem_TXC(nidx, address, openbus);

		return (((openbus & 0xCF) | ((value & 0x03) << 4)));
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_036(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m036.reg);
	return (extcl_save_mapper_TXC(mode, slot, fp));
}

void prg_fix_txc_036(void) {
	memmap_auto_32k(0, MMCPU(0x8000), (txc.output & 0x03));
}
void chr_fix_txc_036(void) {
	memmap_auto_8k(0, MMPPU(0x0000), m036.reg);
}
