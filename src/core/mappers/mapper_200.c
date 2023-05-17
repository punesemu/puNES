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
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_200(void);
INLINE static void chr_fix_200(void);
INLINE static void mirroring_fix_200(void);
INLINE static void tmp_fix_200(BYTE max, BYTE index, const WORD *ds);

struct _m200 {
	WORD reg;
} m200;
struct _m200tmp {
	BYTE max;
	BYTE index;
	const WORD *dipswitch;
} m200tmp;

void map_init_200(void) {
	EXTCL_AFTER_MAPPER_INIT(200);
	EXTCL_CPU_WR_MEM(200);
	EXTCL_CPU_RD_MEM(200);
	EXTCL_SAVE_MAPPER(200);

	if (info.reset == RESET) {
		if (m200tmp.dipswitch) {
			m200tmp.index = (m200tmp.index + 1) % m200tmp.max;
		}
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		memset(&m200tmp, 0x00, sizeof(m200tmp));

		if (info.crc32.prg == 0x478E9E8A) { // 4-in-1 (Multi)[Unknown][Mapper 204].nes
			static const WORD ds[] = { 0x5A5A, 0x005A, 0x5A00, 0x0000 };

			tmp_fix_200(LENGTH(ds), 0, &ds[0]);
		} else if (info.crc32.prg == 0x0CA12D86) { // 1200-in-1 (Alt Games) [p1].nes
			static const WORD ds[] = {
				0x0000, 0x0800, 0x1000, 0x1800, 0x2000, 0x2800, 0x3000, 0x3800,
				0x4000, 0x4800, 0x5000, 0x5800, 0x6000, 0x6800, 0x7000, 0x7800,
				0x8000, 0x8800, 0x9000, 0x9800, 0xA000, 0xA800, 0xB000, 0xB800,
				0xC000, 0xC800, 0xD000, 0xD800, 0xE000, 0xE800, 0xF000, 0xF800
			};

			tmp_fix_200(LENGTH(ds), 0, &ds[0]);
		}
	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_200(void) {
	prg_fix_200();
	chr_fix_200();
	mirroring_fix_200();
}
void extcl_cpu_wr_mem_200(WORD address, UNUSED(BYTE value)) {
	m200.reg = address;
	prg_fix_200();
	chr_fix_200();
	mirroring_fix_200();
}
BYTE extcl_cpu_rd_mem_200(WORD address, BYTE openbus) {
	if ((address < 0x8000) || !m200tmp.dipswitch) {
		return (openbus);
	}
	switch (m200.reg & 0xFF0F) {
		case 0xF004:
			return (m200tmp.dipswitch[m200tmp.index] & 0x00FF);
		case 0xF008:
			return ((m200tmp.dipswitch[m200tmp.index] & 0xFF00) >> 8);
		default:
			return (openbus);
	}
}
BYTE extcl_save_mapper_200(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m200.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_200(void) {
	WORD bank = m200.reg;

	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, bank);
	map_prg_rom_8k(2, 2, bank);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_200(void) {
	DBWORD bank = m200.reg;

	_control_bank(bank, info.chr.rom.max.banks_8k)
	bank <<= 13;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
}
INLINE static void mirroring_fix_200(void) {
	if (m200.reg & 0x0008) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
INLINE static void tmp_fix_200(BYTE max, BYTE index, const WORD *ds) {
	m200tmp.max = max;
	m200tmp.index = index;
	m200tmp.dipswitch = ds;
}