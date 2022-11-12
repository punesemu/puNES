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

INLINE static void prg_fix_164(void);
INLINE static void prg_ram_fix_164(void);
INLINE static void mirroring_fix_164(void);
INLINE static BYTE prg_ram_check(void);
INLINE static void mode1_bpp(WORD address);
INLINE static BYTE mode1_bpp_rd(WORD address);

struct _m164 {
	BYTE reg[4];
	BYTE pa0;
	BYTE pa9;
	BYTE pa13;
} m164;
struct _m164tmp {
	BYTE *prg_6000;
	BYTE cc93c66;
} m164tmp;

void map_init_164(void) {
	EXTCL_AFTER_MAPPER_INIT(164);
	EXTCL_CPU_WR_MEM(164);
	EXTCL_CPU_RD_MEM(164);
	EXTCL_SAVE_MAPPER(164);
	EXTCL_WR_NMT(164);
	EXTCL_WR_CHR(164);
	EXTCL_RD_CHR(164);
	EXTCL_PPU_000_TO_255(164);
	EXTCL_PPU_256_TO_319(164);
	EXTCL_PPU_320_TO_34X(164);
	mapper.internal_struct[0] = (BYTE *)&m164;
	mapper.internal_struct_size[0] = sizeof(m164);

	memset(&m164, 0x00, sizeof(m164));

	if (prg_ram_check()) {
		info.prg.ram.banks_8k_plus = 1;
		info.prg.ram.bat.banks = 1;
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_164(void) {
	prg_fix_164();
	mirroring_fix_164();

	if ((m164tmp.cc93c66 = prg_ram_check())) {
		ee93cx6_init(prg.ram_plus_8k, 512, 8);
	}

	prg_ram_fix_164();

	info.mapper.ram_plus_op_controlled_by_mapper = m164tmp.prg_6000 != NULL;
}
void extcl_cpu_wr_mem_164(WORD address, BYTE value) {
	switch (address & 0xFF00) {
		case 0x5000:
			m164.reg[0] = value;
			prg_fix_164();
			mirroring_fix_164();
			break;
		case 0x5100:
			m164.reg[1] = value;
			prg_fix_164();
			break;
		case 0x5200:
			m164.reg[2] = value;
			if (m164tmp.cc93c66) {
				// D~7654 3210
				//   ---------
				//   .T.S .C.D
				//    | |  | +- 93C66 EEPROM DAT output
				//    | |  +--- 93C66 EEPROM CLK output
				//    | +------ 93C66 EEPROM #1 CS output
				//    +-------- 93C66 EEPROM #2 CS output
				ee93cx6_write((m164.reg[2] & 0x10) >> 4, (m164.reg[2] & 0x04) >> 2, m164.reg[2] & 0x01);
			}
			return;
		case 0x5300:
			m164.reg[3] = value;
			mirroring_fix_164();
			return;
		case 0x6000: case 0x6100: case 0x6200: case 0x6300: case 0x6400: case 0x6500: case 0x6600: case 0x6700:
		case 0x6800: case 0x6900: case 0x6A00: case 0x6B00: case 0x6C00: case 0x6D00: case 0x6E00: case 0x6F00:
		case 0x7000: case 0x7100: case 0x7200: case 0x7300: case 0x7400: case 0x7500: case 0x7600: case 0x7700:
		case 0x7800: case 0x7900: case 0x7A00: case 0x7B00: case 0x7C00: case 0x7D00: case 0x7E00: case 0x7F00:
			if (m164tmp.prg_6000) {
				if (m164tmp.cc93c66 && (address >= 0x7E00)) {
					return;
				}
				m164tmp.prg_6000[address & 0x1FFF] = value;
			}
			break;
	}
}
BYTE extcl_cpu_rd_mem_164(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xF000) {
		case 0x5000:
			if ((address & 0x0800) || !(address & 0x0400)) {
				return (openbus);
			} else if (m164tmp.cc93c66) {
				return (ee93cx6_read() ? 0x00 : 0x04);
			}
			return (m164.reg[2] & 0x04);
		case 0x6000:
		case 0x7000:
			if (m164tmp.prg_6000) {
				if (m164tmp.cc93c66 && (address >= 0x7E00)) {
					return (0xFF);
				}
				return (m164tmp.prg_6000[address & 0x1FFF]);
			}
			break;
	}
	return (openbus);
}
BYTE extcl_save_mapper_164(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m164.reg);
	save_slot_ele(mode, slot, m164.pa0);
	save_slot_ele(mode, slot, m164.pa9);
	save_slot_ele(mode, slot, m164.pa13);

	if (mode == SAVE_SLOT_READ) {
		prg_ram_fix_164();
	}

	return (EXIT_OK);
}
void extcl_wr_nmt_164(WORD address, BYTE value) {
	mode1_bpp(address);
	ntbl.bank_1k[(address & 0x0FFF) >> 10][address & 0x3FF] = value;
}
void extcl_wr_chr_164(WORD address, BYTE value) {
	mode1_bpp(address);
	if (mapper.write_vram) {
		chr.bank_1k[address >> 10][address & 0x3FF] = value;
	}
}
BYTE extcl_rd_chr_164(WORD address) {
	if ((m164.reg[0] & 0x80) && !m164.pa13) {
		return (mode1_bpp_rd(address));
	}
	return (chr.bank_1k[address >> 10][address & 0x3FF]);
}
void extcl_ppu_000_to_255_164(void) {
	if (r2001.visible) {
		extcl_ppu_320_to_34x_164();
	}
}
void extcl_ppu_256_to_319_164(void) {
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
void extcl_ppu_320_to_34x_164(void) {
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

INLINE static void prg_fix_164(void) {
	WORD high = (m164.reg[1] & 0x03) << 5;
	WORD low = (m164.reg[0] & 0x0F) | ((m164.reg[0] & 0x20) >> 1);
	WORD bank;

	// D~7654 3210
	//   ---------
	//   CSQM PPPp
	//   ||+|-++++- PRG A18..A14 if M=0
	//   || | ++++- PRG A18..A15 if M=1
	//   || +------ PRG banking mode
	//   ||          0: PRG A14..A18=QPPPp when CPU A14=0 (UxROM, 16 KiB switchable bank)
	//   ||             PRG A14..A18=11111 when CPU A14=1 and S=0 (fixed bank=1F)
	//   ||             PRG A14..A18=111p0 when CPU A14=1 and S=1 (fixed bank=1C or 1E)
	//   ||          1: PRG A14=CPU A14, PRG A15..A18=PPPp (BxROM, 32 KiB switchable bank)
	//   ||         Also selects nametable mirroring:
	//   ||          0: Forced vertical mirroring
	//   ||          1: Mirroring selected by $5300
	//   |+-------- See 'M' bit description
	if (m164.reg[0] & 0x10) {
		bank = (high >> 1) | (low & 0x0F);
		_control_bank(bank, info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, bank);
	} else {
		bank = high | low;
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, bank);

		bank = high | (m164.reg[0] & 0x40 ? 0x1C | ((m164.reg[0] & 0x01) << 1) : 0x1F);
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 2, bank);
	}
	map_prg_rom_8k_update();
}
INLINE static void prg_ram_fix_164(void) {
	m164tmp.prg_6000 = prg.ram_plus_8k ? prg.ram_plus_8k + (m164tmp.cc93c66 == TRUE ? 512 : 0) : NULL;
}
INLINE static void mirroring_fix_164(void) {
	if ((m164.reg[0] & 0x10) && !(m164.reg[3] & 0x80)) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
INLINE static BYTE prg_ram_check(void) {
	if (info.format == NES_2_0) {
		size_t ee_size = (ines.flags[FL10] & 0xF0) ? (64 << (ines.flags[FL10] >> 4)): 0;

		if (ee_size == 512) {
			return (TRUE);
		}
	} else {
		if (info.prg.ram.banks_8k_plus && info.prg.ram.bat.banks) {
			return (TRUE);
		}
	}
	return (FALSE);
}
INLINE static void mode1_bpp(WORD address) {
	BYTE pa13 = (address & 0x2000) >> 13;

	if (!m164.pa13 && pa13) {
		m164.pa0 = (address & 0x0001) != 0;
		m164.pa9 = (address & 0x0200) != 0;
	}
	m164.pa13 = pa13;
}
INLINE static BYTE mode1_bpp_rd(WORD address) {
	address = (m164.pa9 << 12) | (address & 0x0FF7) | (m164.pa0 << 3);
	return (chr.bank_1k[address >> 10][address & 0x3FF]);
}
