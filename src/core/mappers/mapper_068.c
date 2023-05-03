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
#include "cpu.h"
#include "save_slot.h"

INLINE static void prg_fix_068(void);
INLINE static void chr_fix_068(void);
INLINE static void wram_fix_068(void);
INLINE static void mirroring_fix_068(void);

struct _m068 {
	WORD prg;
	WORD chr[4];
	WORD nmt[2];
	BYTE mirroring;
	struct _m068_external_rom {
		BYTE access;
		BYTE rd_openbus;
		uint32_t timer;
	} ext;
} m068;

void map_init_068(void) {
	EXTCL_AFTER_MAPPER_INIT(068);
	EXTCL_CPU_WR_MEM(068);
	EXTCL_CPU_RD_MEM(068);
	EXTCL_SAVE_MAPPER(068);
	EXTCL_CPU_EVERY_CYCLE(068);
	mapper.internal_struct[0] = (BYTE *)&m068;
	mapper.internal_struct_size[0] = sizeof(m068);

	if (info.reset >= HARD) {
		memset(&m068, 0x00, sizeof(m068));

		m068.chr[0] = 0;
		m068.chr[1] = 1;
		m068.chr[2] = 2;
		m068.chr[3] = 3;

		m068.nmt[0] = 0;
		m068.nmt[1] = 1;
	}

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_068(void) {
	prg_fix_068();
	chr_fix_068();
	wram_fix_068();
	mirroring_fix_068();
}
void extcl_cpu_wr_mem_068(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x6000:
		case 0x7000:
			if ((info.mapper.submapper == 1) && !cpu.prg_ram_wr_active) {
				m068.ext.access = TRUE;
				m068.ext.timer = 107520;
				prg_fix_068();
			}
			return;
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
			m068.chr[(address & 0x3000) >> 12] = value;
			chr_fix_068();
			return;
		case 0xC000:
		case 0xD000:
			m068.nmt[(address & 0x1000) >> 12] = value;
			mirroring_fix_068();
			return;
		case 0xE000:
			m068.mirroring = value;
			mirroring_fix_068();
			return;
		case 0xF000:
			m068.prg = value;
			prg_fix_068();
			wram_fix_068();
			return;
		default:
			return;
	}
}
BYTE extcl_cpu_rd_mem_068(WORD address, BYTE openbus, BYTE before) {
	switch (address & 0xF000) {
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
			return (m068.ext.rd_openbus ? before : openbus);
		default:
			return (openbus);
	}
}
BYTE extcl_save_mapper_068(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m068.prg);
	save_slot_ele(mode, slot, m068.chr);
	save_slot_ele(mode, slot, m068.nmt);
	save_slot_ele(mode, slot, m068.mirroring);
	save_slot_ele(mode, slot, m068.ext.timer);
	save_slot_ele(mode, slot, m068.ext.access);
	save_slot_ele(mode, slot, m068.ext.rd_openbus);

	if (mode == SAVE_SLOT_READ) {
		mirroring_fix_068();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_068(void) {
	if (m068.ext.timer && !--m068.ext.timer) {
		m068.ext.access = FALSE;
		prg_fix_068();
	}
}

INLINE static void prg_fix_068(void) {
	WORD bank[2] = { 0, 0 };

	m068.ext.rd_openbus = FALSE;

	if (info.mapper.submapper == 1) {
		bank[1] = 0x07;

		if (!(m068.prg & 0x08)) {
			if (m068.ext.access) {
				bank[0] = 0x08;
			} else {
				m068.ext.rd_openbus = TRUE;
			}
		} else {
			bank[0] = m068.prg & 0x07;
		}
	} else {
		bank[0] = m068.prg;
		bank[1] = 0xFF;
	}

	_control_bank(bank[0], info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, bank[0]);

	_control_bank(bank[1], info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, bank[1]);

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_068(void) {
	map_chr_rom_2k(0x0000, m068.chr[0]);
	map_chr_rom_2k(0x0800, m068.chr[1]);
	map_chr_rom_2k(0x1000, m068.chr[2]);
	map_chr_rom_2k(0x1800, m068.chr[3]);
}
INLINE static void wram_fix_068(void) {
	cpu.prg_ram_rd_active = (m068.prg & 0x10) >> 4;
	cpu.prg_ram_wr_active = cpu.prg_ram_rd_active;
}
INLINE static void mirroring_fix_068(void) {
	if (!(m068.mirroring & 0x10)) {
		switch (m068.mirroring & 0x03) {
			default:
			case 0:
				mirroring_V();
				return;
			case 1:
				mirroring_H();
				return;
			case 2:
				mirroring_SCR0();
				return;
			case 3:
				mirroring_SCR1();
				return;
		}
	} else {
		WORD bank[2] = { 0x80 | m068.nmt[0], 0x80 | m068.nmt[1]};

		switch (m068.mirroring & 0x03) {
			case 0:
				map_nmt_chr_rom_1k(0, bank[0]);
				map_nmt_chr_rom_1k(1, bank[1]);
				map_nmt_chr_rom_1k(2, bank[0]);
				map_nmt_chr_rom_1k(3, bank[1]);
				break;
			case 1:
				map_nmt_chr_rom_1k(0, bank[0]);
				map_nmt_chr_rom_1k(1, bank[0]);
				map_nmt_chr_rom_1k(2, bank[1]);
				map_nmt_chr_rom_1k(3, bank[1]);
				break;
			case 2:
				map_nmt_chr_rom_1k(0, bank[0]);
				map_nmt_chr_rom_1k(1, bank[0]);
				map_nmt_chr_rom_1k(2, bank[0]);
				map_nmt_chr_rom_1k(3, bank[0]);
				break;
			case 3:
				map_nmt_chr_rom_1k(0, bank[1]);
				map_nmt_chr_rom_1k(1, bank[1]);
				map_nmt_chr_rom_1k(2, bank[1]);
				map_nmt_chr_rom_1k(3, bank[1]);
				break;
		}
	}
}
