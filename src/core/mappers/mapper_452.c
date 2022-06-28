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
#include "cpu.h"
#include "save_slot.h"

INLINE static void prg_fix_452(void);
INLINE static void mirroring_fix_452(void);
INLINE static BYTE prg_ram_452(WORD address);

struct _m452 {
	WORD reg[2];
} m452;

void map_init_452(void) {
	EXTCL_AFTER_MAPPER_INIT(452);
	EXTCL_CPU_WR_MEM(452);
	EXTCL_CPU_RD_MEM(452);
	EXTCL_SAVE_MAPPER(452);
	mapper.internal_struct[0] = (BYTE *)&m452;
	mapper.internal_struct_size[0] = sizeof(m452);

	if (info.reset >= HARD) {
		memset(&m452, 0x00, sizeof(m452));
	}

	info.prg.ram.banks_8k_plus = 1;

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_452(void) {
	prg_fix_452();
	mirroring_fix_452();
}
void extcl_cpu_wr_mem_452(WORD address, BYTE value) {
	if (address <= 0xDFFF) {
		m452.reg[0] = address;
		m452.reg[1] = value;
		prg_fix_452();
		mirroring_fix_452();
	}
	if (prg_ram_452(address)) {
		prg.ram_plus_8k[address & 0x1FFF] = value;
	}
}
BYTE extcl_cpu_rd_mem_452(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	return (prg_ram_452(address) ? prg.ram_plus_8k[address & 0x1FFF] : openbus);
}
BYTE extcl_save_mapper_452(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m452.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_452(void) {
	WORD bank;

	if (m452.reg[1] & 0x0002) {
		bank = m452.reg[0] >> 1;
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 0, bank);
		map_prg_rom_8k(1, 1, bank);
		map_prg_rom_8k(1, 2, bank);
		map_prg_rom_8k(1, 3, bank);
	} else if (m452.reg[1] & 0x0008) {
		bank = m452.reg[0] >> 1;
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 0, bank);

		bank = (m452.reg[0] >> 1) | 0x0001;
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 1, bank);

		bank = (m452.reg[0] >> 1) | 0x0002;
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 2, bank);

		bank = (m452.reg[0] >> 1) | 0x0003 | (m452.reg[1] & 0x0004);
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 3, bank);
	} else {
		bank = m452.reg[0] >> 2;
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, bank);
		map_prg_rom_8k(2, 2, 0);
	}

	map_prg_rom_8k_update();
}
INLINE static void mirroring_fix_452(void) {
	if (m452.reg[1] & 0x0001) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
INLINE static BYTE prg_ram_452(WORD address) {
	BYTE wram = FALSE;

	switch (address & 0xF000) {
		case 0x8000:
		case 0x9000:
			if (((m452.reg[1] & 0x30) == 0x00) || (((m452.reg[1] & 0x30) == 0x20) && (m452.reg[1] & 0x0002))) {
				wram = TRUE;
			}
			break;
		case 0xA000:
		case 0xB000:
			if (((m452.reg[1] & 0x30) == 0x10) || (((m452.reg[1] & 0x30) == 0x30) && (m452.reg[1] & 0x0002))) {
				wram = TRUE;
			}
			break;
		case 0xC000:
		case 0xD000:
			if (((m452.reg[1] & 0x30) == 0x20) || (((m452.reg[1] & 0x30) == 0x00) && (m452.reg[1] & 0x0002))) {
				wram = TRUE;
			}
			break;
		case 0xE000:
		case 0xF000:
			if (((m452.reg[1] & 0x30) == 0x30) || (((m452.reg[1] & 0x30) == 0x10) && (m452.reg[1] & 0x0002))) {
				wram = TRUE;
			}
			break;
	}
	return (wram);
}
