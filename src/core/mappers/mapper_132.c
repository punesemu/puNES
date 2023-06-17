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
#include "info.h"

void prg_fix_txc_132(void);
void chr_fix_txc_132(void);

void map_init_132(void) {
	EXTCL_AFTER_MAPPER_INIT(TXC);
	EXTCL_CPU_WR_MEM(132);
	EXTCL_CPU_RD_MEM(132);
	EXTCL_SAVE_MAPPER(TXC);
	mapper.internal_struct[0] = (BYTE *)&txc;
	mapper.internal_struct_size[0] = sizeof(txc);

	init_TXC(info.reset);
	TXC_prg_fix = prg_fix_txc_132;
	TXC_chr_fix = chr_fix_txc_132;
}
void extcl_cpu_wr_mem_132(WORD address, BYTE value) {
	extcl_cpu_wr_mem_TXC(address, (value & 0x0F));
}
BYTE extcl_cpu_rd_mem_132(WORD address, BYTE openbus) {
	if ((address > 0x4020) && (address <= 0x5FFF)) {
		BYTE value = extcl_cpu_rd_mem_TXC(address, openbus);

		return (((openbus & 0xF0) | (value & 0x0F)));
	}
	return (wram_rd(address));
}

void prg_fix_txc_132(void) {
	memmap_auto_32k(MMCPU(0x8000), ((txc.output & 0x04) >> 2));
}
void chr_fix_txc_132(void) {
	if (info.crc32.total == 0x2A5F4C5A) {
		// Jin Gwok Sei Chuen Saang (Ch) [U][!].unf
		// 戰國四川省 (Zhànguó Sìchuān Shěng, original version of AVE's Tiles of Fate) is set to Mapper 132 in GoodNES 3.23b.
		// That ROM image is actually a mapper hack with the PRG-ROM code unmodified but the CHR-ROM banks rearranged to work
		// as Mapper 132; the correct mapper is INES Mapper 173. That mapper hack only works on certain
		// emulators' implementation of Mapper 132, not on the above implementation based on studying the circuit board.
		memmap_auto_8k(MMPPU(0x0000), (txc.inverter | txc.staging));
	} else {
		memmap_auto_8k(MMPPU(0x0000), (txc.output & 0x03));
	}
}
