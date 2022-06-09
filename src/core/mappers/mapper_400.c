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
#include "ines.h"
#include "save_slot.h"

INLINE static void prg_fix_400(void);
INLINE static void chr_fix_400(void);
INLINE static void mirroring_fix_400(void);

struct _m400 {
	BYTE reg[2];
	BYTE led;
} m400;

void map_init_400(void) {
	EXTCL_AFTER_MAPPER_INIT(400);
	EXTCL_CPU_WR_MEM(400);
	EXTCL_SAVE_MAPPER(400);
	mapper.internal_struct[0] = (BYTE *)&m400;
	mapper.internal_struct_size[0] = sizeof(m400);

	memset(&m400, 0x00, sizeof(m400));

	m400.reg[0] = 0x80;

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_400(void) {
	prg_fix_400();
	chr_fix_400();
	mirroring_fix_400();
}
void extcl_cpu_wr_mem_400(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x7000:
			if (address & 0x0800) {
				m400.reg[0] = value;
			}
			break;
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
			m400.led = value;
			break;
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			// bus conflict
			m400.reg[1] = value & prg_rom_rd(address);
			break;
		default:
			return;
	}
	prg_fix_400();
	chr_fix_400();
	mirroring_fix_400();
}
BYTE extcl_save_mapper_400(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m400.reg);
	save_slot_ele(mode, slot, m400.led);

	return (EXIT_OK);
}

INLINE static void prg_fix_400(void) {
	WORD bank;

	bank = (m400.reg[0] & 0xF8) | (m400.reg[1] & 0x07);
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, bank);

	bank = (m400.reg[0] & 0xF8) | 0x07;
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, bank);

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_400(void) {
	DBWORD bank = m400.reg[1] >> 5;

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
INLINE static void mirroring_fix_400(void) {
	if (m400.reg[0] == 0x80) {
		if (ines.flags[FL6] & 0x08) {
			mirroring_FSCR();
		} else {
			if (ines.flags[FL6] & 0x01) {
				mirroring_V();
			} else {
				mirroring_H();
			}
		}
	} else if (m400.reg[0] & 0x20) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
