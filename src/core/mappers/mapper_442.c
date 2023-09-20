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
#include "ppu.h"
#include "ppu_inline.h"
#include "save_slot.h"

INLINE static void prg_fix_442(void);

INLINE static void mode1_bpp(WORD address);

struct _m442 {
	BYTE reg[8];
	BYTE pa0;
	BYTE pa9;
	BYTE pa13;
} m442;

void map_init_442(void) {
	EXTCL_AFTER_MAPPER_INIT(442);
	EXTCL_CPU_WR_MEM(442);
	EXTCL_SAVE_MAPPER(442);
	EXTCL_WR_NMT(442);
	EXTCL_WR_CHR(442);
	EXTCL_RD_CHR(442);
	EXTCL_PPU_000_TO_255(442);
	EXTCL_PPU_256_TO_319(442);
	EXTCL_PPU_320_TO_34X(442);
	mapper.internal_struct[0] = (BYTE *)&m442;
	mapper.internal_struct_size[0] = sizeof(m442);

	memset(&m442, 0x00, sizeof(m442));

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_442(void) {
	prg_fix_442();
}
void extcl_cpu_wr_mem_442(WORD address, UNUSED(BYTE value)) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		m442.reg[((address & 0x0700) >> 8)] = value;
		prg_fix_442();
	}
}
BYTE extcl_save_mapper_442(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m442.reg);
	save_slot_ele(mode, slot, m442.pa0);
	save_slot_ele(mode, slot, m442.pa9);
	save_slot_ele(mode, slot, m442.pa13);

	return (EXIT_OK);
}
void extcl_wr_chr_442(WORD address, UNUSED(BYTE value)) {
	mode1_bpp(address);
	chr_wr(address, value);
}
BYTE extcl_rd_chr_442(WORD address) {
	if ((m442.reg[0] & 0x80) && !m442.pa13) {
		address = (m442.pa9 << 12) | (address & 0x0FF7) | (m442.pa0 << 3);
	}
	return (chr_rd(address));
}
void extcl_wr_nmt_442(WORD address, UNUSED(BYTE value)) {
	mode1_bpp(address);
	nmt_wr(address, value);
}
void extcl_ppu_000_to_255_442(void) {
	if (ppudata.r2001.visible) {
		extcl_ppu_320_to_34x_442();
	}
}
void extcl_ppu_256_to_319_442(void) {
	if ((ppudata.ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if ((!ppudata.spr_ev.count_plus || (ppudata.spr_ev.tmp_spr_plus == ppudata.spr_ev.count_plus)) && (ppudata.r2000.size_spr == 16)) {
		ppudata.ppu.spr_adr = ppudata.r2000.spt_adr;
	} else {
		ppu_spr_adr((ppudata.ppu.frame_x & 0x0038) >> 3);
	}
	mode1_bpp(ppudata.ppu.spr_adr);
}
void extcl_ppu_320_to_34x_442(void) {
	if ((ppudata.ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if (ppudata.ppu.frame_x == 323) {
		ppu_spr_adr(7);
	}

	ppu_bck_adr(ppudata.r2000.bpt_adr, ppudata.r2006.value);

	mode1_bpp(0x2000 | (ppudata.r2006.value & 0x0FFF));
	mode1_bpp(ppudata.ppu.bck_adr);
}

INLINE static void prg_fix_442(void) {
	WORD bank = ((m442.reg[0] & 0x40) >> 1) | (m442.reg[0] & 0x1F);

	memmap_auto_32k(MMCPU(0x8000), bank);
}

INLINE static void mode1_bpp(WORD address) {
	BYTE pa13 = (address & 0x2000) >> 13;

	if (!m442.pa13 && pa13) {
		m442.pa0 = (address & 0x001) != 0;
		m442.pa9 = (address & 0x200) != 0;
	}
	m442.pa13 = pa13;
}
