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
#include "save_slot.h"

INLINE static void prg_fix_559(void);
INLINE static void mirroring_fix_559(void);

struct _m559 {
	BYTE prg[3];
	BYTE mir[4];
	BYTE swap;
} m559;

void map_init_559(void) {
	map_init_VRC4(VRC4M559);

	EXTCL_AFTER_MAPPER_INIT(559);
	EXTCL_CPU_WR_MEM(559);
	EXTCL_SAVE_MAPPER(559);
	mapper.internal_struct[0] = (BYTE *)&m559;
	mapper.internal_struct_size[0] = sizeof(m559);
	mapper.internal_struct[1] = (BYTE *)&vrc4;
	mapper.internal_struct_size[1] = sizeof(vrc4);

	memset(&m559, 0x00, sizeof(m559));

	m559.prg[2] = 0xFE;
	m559.mir[0] = 0xE0;
	m559.mir[1] = 0xE0;
	m559.mir[2] = 0xE1;
	m559.mir[3] = 0xE1;
}
void extcl_after_mapper_init_559(void) {
	prg_fix_559();
	mirroring_fix_559();
}
void extcl_cpu_wr_mem_559(WORD address, BYTE value) {
	switch (address_VRC4(address)) {
		case 0x8000:
			m559.prg[0] = value;
			break;
		case 0xA000:
			m559.prg[1] = value;
			break;
		case 0x9002:
			m559.swap = value & 0x02;
			break;
		case 0x9003:
			if (address & 0x0004) {
				m559.mir[address & 0x03] = value;
			} else {
				m559.prg[2] = value;
			}
			break;
		case 0xB001:
		case 0xB003:
		case 0xC001:
		case 0xC003:
		case 0xD001:
		case 0xD003:
		case 0xE001:
		case 0xE003:
		case 0xF001:
		case 0xF003:
			value >>= 4;
			extcl_cpu_wr_mem_VRC4(address, value);
			break;
		default:
			extcl_cpu_wr_mem_VRC4(address, value);
			break;
	}
	prg_fix_559();
	mirroring_fix_559();
}
BYTE extcl_save_mapper_559(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m559.prg);
	save_slot_ele(mode, slot, m559.mir);
	save_slot_ele(mode, slot, m559.swap);

	if (mode == SAVE_SLOT_READ) {
		mirroring_fix_559();
	}

	return (EXIT_OK);
}

INLINE static void prg_fix_559(void) {
	WORD bank;

	bank = m559.prg[0] & 0x1F;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0 ^ m559.swap, bank);

	bank = m559.prg[1] & 0x1F;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, bank);

	bank = m559.prg[2];
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2, bank);

	bank = 0xFF & 0x1F;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, bank);

	map_prg_rom_8k_update();
}
INLINE static void mirroring_fix_559(void) {
	DBWORD bank;

	bank = m559.mir[0];
	_control_bank(bank, info.chr.rom.max.banks_1k)
	bank <<= 10;
	ntbl.bank_1k[0] = chr_pnt(bank);

	bank = m559.mir[1];
	_control_bank(bank, info.chr.rom.max.banks_1k)
	bank <<= 10;
	ntbl.bank_1k[1] = chr_pnt(bank);

	bank = m559.mir[2];
	_control_bank(bank, info.chr.rom.max.banks_1k)
	bank <<= 10;
	ntbl.bank_1k[2] = chr_pnt(bank);

	bank = m559.mir[3];
	_control_bank(bank, info.chr.rom.max.banks_1k)
	bank <<= 10;
	ntbl.bank_1k[3] = chr_pnt(bank);
}
