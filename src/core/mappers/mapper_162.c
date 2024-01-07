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
#include "ppu_inline.h"
#include "save_slot.h"

INLINE static void prg_fix_162(void);
INLINE static void mode1_bpp(WORD address);

struct _m162 {
	BYTE reg[4];
	BYTE pa9;
	BYTE pa13;
} m162;

void map_init_162(void) {
	EXTCL_AFTER_MAPPER_INIT(162);
	EXTCL_CPU_WR_MEM(162);
	EXTCL_CPU_RD_MEM(162);
	EXTCL_SAVE_MAPPER(162);
	EXTCL_WR_NMT(162);
	EXTCL_WR_CHR(162);
	EXTCL_RD_CHR(162);
	EXTCL_PPU_000_TO_255(162);
	EXTCL_PPU_256_TO_319(162);
	EXTCL_PPU_320_TO_34X(162);
	map_internal_struct_init((BYTE *)&m162, sizeof(m162));

	memset(&m162, 0x00, sizeof(m162));

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_162(void) {
	prg_fix_162();
}
void extcl_cpu_wr_mem_162(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xFF00) {
		case 0x5000:
			m162.reg[0] = value;
			prg_fix_162();
			break;
		case 0x5100:
			m162.reg[1] = value;
			prg_fix_162();
			break;
		case 0x5200:
			m162.reg[2] = value;
			prg_fix_162();
			return;
		case 0x5300:
			m162.reg[3] = value;
			prg_fix_162();
			return;
	}
}
BYTE extcl_cpu_rd_mem_162(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return (0x00);
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_162(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m162.reg);
	save_slot_ele(mode, slot, m162.pa9);
	save_slot_ele(mode, slot, m162.pa13);
	return (EXIT_OK);
}
BYTE extcl_rd_chr_162(BYTE nidx, WORD address) {
	if ((m162.reg[0] & 0x80) && !m162.pa13) {
		address = (m162.pa9 << 12) | (address & 0x0FFF);
	}
	return (chr_rd(nidx, address));
}
void extcl_wr_nmt_162(BYTE nidx, WORD address, UNUSED(BYTE value)) {
	mode1_bpp(address);
	nmt_wr(nidx, address, value);
}
void extcl_wr_chr_162(BYTE nidx, WORD address, UNUSED(BYTE value)) {
	mode1_bpp(address);
	chr_wr(nidx, address, value);
}
void extcl_ppu_000_to_255_162(BYTE nidx) {
	if (nes[nidx].p.r2001.visible) {
		extcl_ppu_320_to_34x_162(nidx);
	}
}
void extcl_ppu_256_to_319_162(BYTE nidx) {
	if ((nes[nidx].p.ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if ((!nes[nidx].p.spr_ev.count_plus || (nes[nidx].p.spr_ev.tmp_spr_plus == nes[nidx].p.spr_ev.count_plus)) && (nes[nidx].p.r2000.size_spr == 16)) {
		nes[nidx].p.ppu.spr_adr = nes[nidx].p.r2000.spt_adr;
	} else {
		ppu_spr_adr((nes[nidx].p.ppu.frame_x & 0x0038) >> 3);
	}
	mode1_bpp(nes[nidx].p.ppu.spr_adr);
}
void extcl_ppu_320_to_34x_162(BYTE nidx) {
	if ((nes[nidx].p.ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if (nes[nidx].p.ppu.frame_x == 323) {
		ppu_spr_adr(7);
	}

	ppu_bck_adr(nes[nidx].p.r2000.bpt_adr, nes[nidx].p.r2006.value);

	mode1_bpp(0x2000 | (nes[nidx].p.r2006.value & 0x0FFF));
	mode1_bpp(nes[nidx].p.ppu.bck_adr);
}

INLINE static void prg_fix_162(void) {
	WORD bank = ((m162.reg[2] & 0x03) << 4) | (m162.reg[0] & 0x0C);

	// D~7654 3210
	//   ---------
	//   .... .A.B
	//         | +- PRG A15 mode:
	//         |     0: PRG A15=$5100.1
	//         |     1: PRG A15=1       if A=0
	//         |        PRG A15=$5000.0 if A=1
	//         +--- PRG A16 mode:
	//               0: PRG A16=1
	//               1: PRG A16=$5000.1
	switch (m162.reg[3] & 0x05) {
		case 0x00:
			bank |= (0x02 | ((m162.reg[1] & 0x02) >> 1));
			break;
		case 0x01:
			bank |= 0x03;
			break;
		case 0x04:
			bank |= ((m162.reg[0] & 0x02) | ((m162.reg[1] & 0x02) >> 1));
			break;
		case 0x05:
			bank |= (m162.reg[0] & 0x03);
			break;
	}
	memmap_auto_32k(0, MMCPU(0x8000), bank);
}
INLINE static void mode1_bpp(WORD address) {
	BYTE pa13 = (address & 0x2000) >> 13;

	if (!m162.pa13 && pa13) {
		m162.pa9 = (address & 0x0200) != 0;
	}
	m162.pa13 = pa13;
}
