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
#include "save_slot.h"
#include "EE93Cx6.h"

INLINE static void prg_fix_558(void);
INLINE static void prg_ram_fix_558(void);
INLINE static BYTE prg_ram_check(void);

struct _m558 {
	BYTE reg[4];
} m558;
struct _m558tmp {
	BYTE *prg_6000;
	BYTE cc93c66;
} m558tmp;

void map_init_558(void) {
	EXTCL_AFTER_MAPPER_INIT(558);
	EXTCL_CPU_WR_MEM(558);
	EXTCL_CPU_RD_MEM(558);
	EXTCL_SAVE_MAPPER(558);
	mapper.internal_struct[0] = (BYTE *)&m558;
	mapper.internal_struct_size[0] = sizeof(m558);

	memset(&m558, 0x00, sizeof(m558));

	if (prg_ram_check()) {
		info.prg.ram.banks_8k_plus = 1;
		info.prg.ram.bat.banks = 1;
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_558(void) {
	prg_fix_558();

	if ((m558tmp.cc93c66 = prg_ram_check())) {
		ee93cx6_init(prg.ram_plus_8k, 512, 8);
	}

	prg_ram_fix_558();

	info.mapper.ram_plus_op_controlled_by_mapper = m558tmp.prg_6000 != NULL;
}
void extcl_cpu_wr_mem_558(WORD address, BYTE value) {
	switch (address & 0xFF00) {
		case 0x5000:
			// CPU D0 and D1 are connected in reverse to the mapper blob compared to other circuit boards using this ASIC.
			// This means that setting $5300.0 to 0, not to 1, will effectively swap PRG A15/A16 and PRG A19/A20.
			// Cartridges using this mapper must be dumped with $5300=$07 during dumping to obtain a readout that matches
			// that of a desoldered ROM chip.
			if (!(m558.reg[3] & 0x01)) {
				value = (value & 0xFC) | ((value & 0x01) << 1) | ((value & 0x02) >> 1);
			}
			m558.reg[0] = value;
			prg_fix_558();
			break;
		case 0x5100:
			if (!(m558.reg[3] & 0x01)) {
				value = (value & 0xFC) | ((value & 0x01) << 1) | ((value & 0x02) >> 1);
			}
			m558.reg[1] = value;
			prg_fix_558();
			break;
		case 0x5200:
			if (!(m558.reg[3] & 0x01)) {
				value = (value & 0xFC) | ((value & 0x01) << 1) | ((value & 0x02) >> 1);
			}
			m558.reg[2] = value;
			if (m558tmp.cc93c66) {
				// D~7654 3210
				//   ---------
				//   .... .SDC
				//         ||+- 93C66 EEPROM CLK ($5300.0=0)/DAT ($5300.0=1) output
				//         |+-- 93C66 EEPROM DAT ($5300.0=0)/CLK ($5300.0=1) output
				//         +--- 93C66 EEPROM CS output
				ee93cx6_write((m558.reg[2] & 0x04) >> 2, m558.reg[2] & 0x01, (m558.reg[2] & 0x02) >> 1);
			}
			return;
		case 0x5300:
			m558.reg[3] = value;
			prg_fix_558();
			return;
		case 0x6000: case 0x6100: case 0x6200: case 0x6300: case 0x6400: case 0x6500: case 0x6600: case 0x6700:
		case 0x6800: case 0x6900: case 0x6A00: case 0x6B00: case 0x6C00: case 0x6D00: case 0x6E00: case 0x6F00:
		case 0x7000: case 0x7100: case 0x7200: case 0x7300: case 0x7400: case 0x7500: case 0x7600: case 0x7700:
		case 0x7800: case 0x7900: case 0x7A00: case 0x7B00: case 0x7C00: case 0x7D00: case 0x7E00: case 0x7F00:
			if (m558tmp.prg_6000) {
				if (m558tmp.cc93c66 && (address >= 0x7E00)) {
					return;
				}
				m558tmp.prg_6000[address & 0x1FFF] = value;
			}
			break;
	}
}
BYTE extcl_cpu_rd_mem_558(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xF000) {
		case 0x5000:
			if (m558tmp.cc93c66) {
				return (ee93cx6_read() ? 0x04 : 0x00);
			}
			return (m558.reg[2] & 0x04);
		case 0x6000:
		case 0x7000:
			if (m558tmp.prg_6000) {
				if (m558tmp.cc93c66 && (address >= 0x7E00)) {
					return (0xFF);
				}
				return (m558tmp.prg_6000[address & 0x1FFF]);
			}
			break;
	}
	return (openbus);
}
BYTE extcl_save_mapper_558(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m558.reg);

	if (mode == SAVE_SLOT_READ) {
		prg_ram_fix_558();
	}

	return (EXIT_OK);
}

INLINE static void prg_fix_558(void) {
	WORD high = m558.reg[1] << 4;
	WORD low = (m558.reg[0] & 0x0F) | (m558.reg[3] & 0x04? 0x00: 0x03);
	WORD bank;

	bank = high | low;
	_control_bank(bank, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, bank);
	map_prg_rom_8k_update();
}
INLINE static void prg_ram_fix_558(void) {
	m558tmp.prg_6000 = prg.ram_plus_8k ? prg.ram_plus_8k + (m558tmp.cc93c66 == TRUE ? 512 : 0) : NULL;
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
