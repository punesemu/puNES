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
#include "mem_map.h"
#include "cpu.h"
#include "ines.h"
#include "save_slot.h"
#include "nes20db.h"
#include "EE93Cx6.h"

void prg_fix_vrc2and4_529(void);
void chr_swap_vrc2and4_529(WORD address, WORD value);

INLINE static void prg_ram_fix(void);
INLINE static BYTE prg_ram_check(void);

struct _m529tmp {
	BYTE *prg_6000;
	BYTE cc93c56;
} m529tmp;

void map_init_529(void) {
	EXTCL_AFTER_MAPPER_INIT(529);
	EXTCL_CPU_WR_MEM(529);
	EXTCL_CPU_RD_MEM(529);
	EXTCL_SAVE_MAPPER(529);
	EXTCL_CPU_EVERY_CYCLE(VRC2and4);
	mapper.internal_struct[0] = (BYTE *)&vrc2and4;
	mapper.internal_struct_size[0] = sizeof(vrc2and4);

	init_VRC2and4(VRC24_VRC4, 0x04, 0x08, TRUE);
	VRC2and4_prg_fix = prg_fix_vrc2and4_529;
	VRC2and4_chr_swap = chr_swap_vrc2and4_529;

	if (prg_ram_check()) {
		info.prg.ram.banks_8k_plus = 1;
		info.prg.ram.bat.banks = 1;
	}
}
void extcl_after_mapper_init_529(void) {
	extcl_after_mapper_init_VRC2and4();

	if ((m529tmp.cc93c56 = prg_ram_check())) {
		ee93cx6_init(prg.ram_plus_8k, 256, 16);
	}
	prg_ram_fix();
	info.mapper.ram_plus_op_controlled_by_mapper = m529tmp.prg_6000 != NULL;
}
void extcl_cpu_wr_mem_529(WORD address, BYTE value) {
	if (address < 0x8000) {
		if ((address >= 0x6000) && (address <= 0x7FFF)) {
			if (m529tmp.prg_6000) {
				if (m529tmp.cc93c56 && (address >= 0x7F00)) {
					return;
				}
				m529tmp.prg_6000[address & 0x1FFF] = value;
			}
			return;
		}
	}
	if (address & 0x0800) {
		if (m529tmp.cc93c56) {
			// D~[.... .ECD]
			//          ||+- Serial Data Input to 93C56 EEPROM
			//          |+-- Serial Clock to 93C56 EEPROM
			//          +--- Chip Select to 93C56 EEPROM
			ee93cx6_write((value & 0x04) >> 2, (value & 0x02) >> 1, value & 0x01);
			return;
		}
	}
	extcl_cpu_wr_mem_VRC2and4(address, value);
}
BYTE extcl_cpu_rd_mem_529(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xF000) {
		case 0x5000:
			if (m529tmp.cc93c56) {
				return (ee93cx6_read() ? 0x01 : 0x00);
			}
			return (0x01);
		case 0x6000:
		case 0x7000:
			if (m529tmp.prg_6000) {
				if (m529tmp.cc93c56 && (address >= 0x7F00)) {
					return (0xFF);
				}
				return (m529tmp.prg_6000[address & 0x1FFF]);
			}
			break;
	}
	return (openbus);
}

BYTE extcl_save_mapper_529(BYTE mode, BYTE slot, FILE *fp) {
	extcl_save_mapper_VRC2and4(mode, slot, fp);

	return (EXIT_OK);
}

void prg_fix_vrc2and4_529(void) {
	WORD bank = 0;

	bank = vrc2and4.prg[1];
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, bank);

	bank = 0xFF;
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, bank);

	map_prg_rom_8k_update();
}
void chr_swap_vrc2and4_529(WORD address, WORD value) {
	chr_swap_VRC2and4_base(address, (value & 0x1FF));
}

INLINE static void prg_ram_fix(void) {
	m529tmp.prg_6000 = prg.ram_plus_8k ? prg.ram_plus_8k + (m529tmp.cc93c56 ? 256 : 0) : NULL;
}
INLINE static BYTE prg_ram_check(void) {
	if (info.format == NES_2_0) {
		size_t ee_size = info.mapper.nes20db.in_use
			? nes20db.prgnvram.size
			: (size_t)((ines.flags[FL10] & 0xF0) ? (64 << (ines.flags[FL10] >> 4)): 0);

		if (ee_size == 256) {
			return (TRUE);
		}
	} else {
		if (info.prg.ram.banks_8k_plus && info.prg.ram.bat.banks) {
			return (TRUE);
		}
	}
	return (FALSE);
}
