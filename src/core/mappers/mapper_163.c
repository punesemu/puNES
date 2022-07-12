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
#include "EE93Cx6.h"

INLINE static void prg_fix_163(void);
INLINE static void mode1_bpp(WORD address);
INLINE static BYTE mode1_bpp_rd(WORD address);

struct _m163 {
	BYTE reg[4];
	BYTE pa9;
	BYTE pa13;
} m163;
struct _m163tmp {
	BYTE disable_swap;
} m163tmp;

void map_init_163(void) {
	EXTCL_AFTER_MAPPER_INIT(163);
	EXTCL_CPU_WR_MEM(163);
	EXTCL_CPU_RD_MEM(163);
	EXTCL_SAVE_MAPPER(163);
	EXTCL_WR_NMT(163);
	EXTCL_WR_CHR(163);
	EXTCL_RD_CHR(163);
	EXTCL_PPU_000_TO_255(163);
	EXTCL_PPU_256_TO_319(163);
	EXTCL_PPU_320_TO_34X(163);
	mapper.internal_struct[0] = (BYTE *)&m163;
	mapper.internal_struct_size[0] = sizeof(m163);

	memset(&m163, 0x00, sizeof(m163));

	m163tmp.disable_swap = FALSE;

	if ((info.crc32.total == 0x49F417F1) || // Diablo II (Unl) [T+Eng] pacsandave.nes
		(info.crc32.total == 0xEFF96E8A) || // World of Warcraft - Demon Hunter [South Crystal Technology].nes
		(info.crc32.total == 0x63C41F82) || // World of Warcraft - Demon Hunter (Asia) (Unl) [T-En by Pacnsacdave v1.0] [n].nes
		(info.crc32.total == 0x57414FB6) || // Martial arts world [South Jing Technology].nes
		(info.crc32.total == 0xA9C4712A)) { // Kou Dai Zuan Shi (NJ010)(Ch)[!].nes
		m163tmp.disable_swap = TRUE;
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_163(void) {
	prg_fix_163();
}
void extcl_cpu_wr_mem_163(WORD address, BYTE value) {
	switch (address & 0xFF00) {
		case 0x5000:
			if (m163.reg[3] & 0x01) {
				value = (value & 0xFC) | ((value & 0x01) << 1) | ((value & 0x02) >> 1);
			}
			m163.reg[0] = value;
			prg_fix_163();
			break;
		case 0x5100:
			if (m163.reg[3] & 0x01) {
				value = (value & 0xFC) | ((value & 0x01) << 1) | ((value & 0x02) >> 1);
			}
			if (address & 0x0001) {
				if ((m163.reg[1] & 0x01) && !(value & 0x01)) {
					m163.reg[1] ^= 0x04;
				}
			} else {
				m163.reg[1] = value;
			}
			prg_fix_163();
			break;
		case 0x5200:
			// 1 MiB games connect both ASIC PRG A19 and A20 outputs to ROM A19,
			// effectively exempting this register from the bit-swap.
			if ((m163.reg[3] & 0x01) && (prg.rom.size >= (2 * 1024 * 1024))) {
				value = (value & 0xFC) | ((value & 0x01) << 1) | ((value & 0x02) >> 1);
			}
			m163.reg[2] = value;
			prg_fix_163();
			return;
		case 0x5300:
			m163.reg[3] = value & ~m163tmp.disable_swap;
			prg_fix_163();
			return;
	}
}
BYTE extcl_cpu_rd_mem_163(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		if (!(address & 0x0800)) {
			return (~m163.reg[1] & 0x04);
		}
	}
	return (openbus);
}
BYTE extcl_save_mapper_163(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m163.reg);
	save_slot_ele(mode, slot, m163.pa9);
	save_slot_ele(mode, slot, m163.pa13);

	return (EXIT_OK);
}
void extcl_wr_nmt_163(WORD address, BYTE value) {
	mode1_bpp(address);
	ntbl.bank_1k[(address & 0x0FFF) >> 10][address & 0x3FF] = value;
}
void extcl_wr_chr_163(WORD address, BYTE value) {
	mode1_bpp(address);
	if (mapper.write_vram) {
		chr.bank_1k[address >> 10][address & 0x3FF] = value;
	}
}
BYTE extcl_rd_chr_163(WORD address) {
	if ((m163.reg[0] & 0x80) && !m163.pa13) {
		return (mode1_bpp_rd(address));
	}
	return (chr.bank_1k[address >> 10][address & 0x3FF]);
}
void extcl_ppu_000_to_255_163(void) {
	if (r2001.visible) {
		extcl_ppu_320_to_34x_163();
	}
}
void extcl_ppu_256_to_319_163(void) {
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
void extcl_ppu_320_to_34x_163(void) {
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

INLINE static void prg_fix_163(void) {
	WORD bank = ((m163.reg[2] & 0x03) << 4) | (m163.reg[0] & 0x0F) | (m163.reg[3] & 0x04 ? 0x00: 0x03);

	_control_bank(bank, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, bank);
	map_prg_rom_8k_update();
}
INLINE static void mode1_bpp(WORD address) {
	BYTE pa13 = (address & 0x2000) >> 13;

	if (!m163.pa13 && pa13) {
		m163.pa9 = !!(address & 0x0200);
	}
	m163.pa13 = pa13;
}
INLINE static BYTE mode1_bpp_rd(WORD address) {
	address = (m163.pa9 << 12) | (address & 0x0FFF);
	return (chr.bank_1k[address >> 10][address & 0x3FF]);
}
