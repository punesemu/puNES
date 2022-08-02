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
#include "mem_map.h"
#include "vs_system.h"
#include "gui.h"
#include "save_slot.h"

INLINE static void prg_fix_403(void);
INLINE static void chr_fix_403(void);
INLINE static void mirroring_fix_403(void);

struct _m403 {
	BYTE reg[4];
} m403;

void map_init_403(void) {
	EXTCL_AFTER_MAPPER_INIT(403);
	EXTCL_CPU_WR_MEM(403);
	EXTCL_SAVE_MAPPER(403);
	mapper.internal_struct[0] = (BYTE *)&m403;
	mapper.internal_struct_size[0] = sizeof(m403);

	memset(&m403, 0x00, sizeof(m403));

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_403(void) {
	prg_fix_403();
	chr_fix_403();
	mirroring_fix_403();
}
void extcl_cpu_wr_mem_403(WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x4000:
			if (address & 0x0100) {
				m403.reg[address & 0x03] = value;
				prg_fix_403();
				chr_fix_403();
				mirroring_fix_403();
			}
			break;
		case 0x8000:
		case 0xA000:
		case 0xC000:
		case 0xE000:
			if (m403.reg[2] & 0x04) {
				m403.reg[1] = value;
				chr_fix_403();
			}
			break;
	}
}
BYTE extcl_save_mapper_403(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m403.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_403(void) {
	WORD bank;

	if (m403.reg[2] & 0x01) {
		bank = ((m403.reg[0] & 0x7E) >> 1);
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, bank);
		map_prg_rom_8k(2, 2, bank);
	} else {
		bank = ((m403.reg[0] & 0x7E) >> 2);
		_control_bank(bank, info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, bank);
	}

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_403(void) {
	DBWORD bank = m403.reg[1] & 0x03;

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
INLINE static void mirroring_fix_403(void) {
	if (m403.reg[2] & 0x10) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
