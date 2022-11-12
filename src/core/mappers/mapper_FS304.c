/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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
#include "info.h"
#include "mem_map.h"
#include "ines.h"
#include "ppu.h"
#include "ppu_inline.h"
#include "save_slot.h"

INLINE static void prg_fix_FS304(void);
INLINE static void mode1_bpp(WORD address);
INLINE static BYTE mode1_bpp_rd(WORD address);

struct _fs304 {
	BYTE reg[4];
	BYTE pa9;
	BYTE pa13;
} fs304;

void map_init_FS304(void) {
	EXTCL_AFTER_MAPPER_INIT(FS304);
	EXTCL_CPU_WR_MEM(FS304);
	EXTCL_CPU_RD_MEM(FS304);
	EXTCL_SAVE_MAPPER(FS304);
	EXTCL_WR_NMT(FS304);
	EXTCL_WR_CHR(FS304);
	EXTCL_RD_CHR(FS304);
	EXTCL_PPU_000_TO_255(FS304);
	EXTCL_PPU_256_TO_319(FS304);
	EXTCL_PPU_320_TO_34X(FS304);
	mapper.internal_struct[0] = (BYTE *)&fs304;
	mapper.internal_struct_size[0] = sizeof(fs304);

	memset(&fs304, 0x00, sizeof(fs304));

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_FS304(void) {
	prg_fix_FS304();
}
void extcl_cpu_wr_mem_FS304(WORD address, BYTE value) {
	switch (address & 0xFF00) {
		case 0x5000:
			fs304.reg[0] = value;
			prg_fix_FS304();
			break;
		case 0x5100:
			fs304.reg[1] = value;
			prg_fix_FS304();
			break;
		case 0x5200:
			fs304.reg[2] = value;
			prg_fix_FS304();
			return;
		case 0x5300:
			fs304.reg[3] = value;
			prg_fix_FS304();
			return;
	}
}
BYTE extcl_cpu_rd_mem_FS304(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return (0x00);
	}
	return (openbus);
}
BYTE extcl_save_mapper_FS304(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, fs304.reg);
	save_slot_ele(mode, slot, fs304.pa9);
	save_slot_ele(mode, slot, fs304.pa13);

	return (EXIT_OK);
}
void extcl_wr_nmt_FS304(WORD address, BYTE value) {
	mode1_bpp(address);
	ntbl.bank_1k[(address & 0x0FFF) >> 10][address & 0x3FF] = value;
}
void extcl_wr_chr_FS304(WORD address, BYTE value) {
	mode1_bpp(address);
	if (mapper.write_vram) {
		chr.bank_1k[address >> 10][address & 0x3FF] = value;
	}
}
BYTE extcl_rd_chr_FS304(WORD address) {
	if ((fs304.reg[0] & 0x80) && !fs304.pa13) {
		return (mode1_bpp_rd(address));
	}
	return (chr.bank_1k[address >> 10][address & 0x3FF]);
}
void extcl_ppu_000_to_255_FS304(void) {
	if (r2001.visible) {
		extcl_ppu_320_to_34x_FS304();
	}
}
void extcl_ppu_256_to_319_FS304(void) {
	if ((ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if ((!spr_ev.count_plus || (spr_ev.tmp_spr_plus == spr_ev.count_plus)) && (r2000.size_spr == 16)) {
		ppu.spr_adr = r2000.spt_adr;
	} else {
		ppu_spr_adr((ppu.frame_x & 0x0038) >> 3);
	}
	mode1_bpp(ppu.spr_adr);
}
void extcl_ppu_320_to_34x_FS304(void) {
	if ((ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if (ppu.frame_x == 323) {
		ppu_spr_adr(7);
	}

	ppu_bck_adr(r2000.bpt_adr, r2006.value);

	mode1_bpp(0x2000 | (r2006.value & 0x0FFF));
	mode1_bpp(ppu.bck_adr);
}

INLINE static void prg_fix_FS304(void) {
	WORD bank = ((fs304.reg[2] & 0x03) << 4) | (fs304.reg[0] & 0x0C);

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
	switch (fs304.reg[3] & 0x05) {
	case 0x00:
		bank |= (0x02 | ((fs304.reg[1] & 0x02) >> 1));
		break;
	case 0x01:
		bank |= 0x03;
		break;
	case 0x04:
		bank |= ((fs304.reg[0] & 0x02) | ((fs304.reg[1] & 0x02) >> 1));
		break;
	case 0x05:
		bank |= (fs304.reg[0] & 0x03);
		break;
	}
	_control_bank(bank, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, bank);
	map_prg_rom_8k_update();
}
INLINE static void mode1_bpp(WORD address) {
	BYTE pa13 = (address & 0x2000) >> 13;

	if (!fs304.pa13 && pa13) {
		fs304.pa9 = (address & 0x0200) != 0;
	}
	fs304.pa13 = pa13;
}
INLINE static BYTE mode1_bpp_rd(WORD address) {
	address = (fs304.pa9 << 12) | (address & 0x0FFF);
	return (chr.bank_1k[address >> 10][address & 0x3FF]);
}

