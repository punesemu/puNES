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
#include "save_slot.h"
#include "EE93Cx6.h"

INLINE static void prg_fix_558(void);
INLINE static void wram_fix_558(void);

struct _m558 {
	BYTE reg[4];
} m558;
struct _m558tmp {
	BYTE cc93c66;
} m558tmp;

void map_init_558(void) {
	EXTCL_AFTER_MAPPER_INIT(558);
	EXTCL_CPU_INIT_PC(558);
	EXTCL_CPU_WR_MEM(558);
	EXTCL_CPU_RD_MEM(558);
	EXTCL_SAVE_MAPPER(558);
	mapper.internal_struct[0] = (BYTE *)&m558;
	mapper.internal_struct_size[0] = sizeof(m558);

	memset(&m558, 0x00, sizeof(m558));

	m558tmp.cc93c66 = wram_nvram_size() == S512B;

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_558(void) {
	prg_fix_558();
	wram_fix_558();
}
void extcl_cpu_init_pc_558(void) {
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (m558tmp.cc93c66) {
			ee93cx6_init(wram_nvram_pnt(), wram_nvram_size(), 8);
		}
	}
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
			return;
		case 0x5100:
			if (!(m558.reg[3] & 0x01)) {
				value = (value & 0xFC) | ((value & 0x01) << 1) | ((value & 0x02) >> 1);
			}
			m558.reg[1] = value;
			prg_fix_558();
			return;
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
		default:
			return;
	}
}
BYTE extcl_cpu_rd_mem_558(WORD address, UNUSED(BYTE openbus)) {
	switch (address & 0xF000) {
		case 0x5000:
			if (m558tmp.cc93c66) {
				return (ee93cx6_read() ? 0x04 : 0x00);
			}
			return (m558.reg[2] & 0x04);
		default:
			return (address >= 0x8000 ? prgrom_rd(address) : wram_rd(address));
	}
}
BYTE extcl_save_mapper_558(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m558.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_558(void) {
	WORD high = m558.reg[1] << 4;
	WORD low = (m558.reg[0] & 0x0F) | (m558.reg[3] & 0x04? 0x00: 0x03);
	WORD bank = high | low;

	memmap_auto_32k(MMCPU(0x8000), bank);
}
INLINE static void wram_fix_558(void) {
	if (m558tmp.cc93c66) {
		if (wram_ram_size()) {
			memmap_wram_ram_wp_8k(MMCPU(0x6000), 0, TRUE, TRUE);
		} else {
			memmap_disable_8k(MMCPU(0x6000));
		}
	} else {
		memmap_auto_8k(MMCPU(0x6000), 0);
	}
}
