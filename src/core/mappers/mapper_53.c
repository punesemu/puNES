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
#include "../../c++/crc/crc.h"

INLINE static void m53_update_6000(void);

struct _m53 {
	BYTE reg[2];
} m53;
struct _m53tmp {
	BYTE *prg_6000;
	BYTE eprom_first;
} m53tmp;

void map_init_53(void) {
	EXTCL_CPU_WR_MEM(53);
	EXTCL_CPU_RD_MEM(53);
	EXTCL_SAVE_MAPPER(53);
	mapper.internal_struct[0] = (BYTE *)&m53;
	mapper.internal_struct_size[0] = sizeof(m53);

	if (info.reset >= HARD) {
		memset(&m53, 0x00, sizeof(m53));
		// Supervision 16-in-1 [U][p1][!].unf
		m53tmp.eprom_first = (prg_size() >= 0x8000) && (emu_crc32((void *)prg_rom(), 0x8000) == 0x63794E25);
		extcl_cpu_wr_mem_53(0x6000, 0x00);
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_53(WORD address, BYTE value) {
	if (address < 0x6000) {
		return;
	}

	if (address >= 0x8000) {
		m53.reg[1] = value;
	} else {
		m53.reg[0] = value;

		if (m53.reg[0] & 0x20) {
			mirroring_H();
		} else  {
			mirroring_V();
		}
	}

	m53_update_6000();

	if (m53.reg[0] & 0x10) {
		WORD bank, tmp = (m53.reg[0] << 3) & 0x78;

		bank = (tmp | (m53.reg[1] & 0x07)) + (m53tmp.eprom_first ? 0x02 : 0x00);
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, bank);

		bank = (tmp | 0x07) + (m53tmp.eprom_first ? 0x02 : 0x00);
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 2, bank);
	} else {
		value = m53tmp.eprom_first ? 0x00 : 0x80;
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);

		value = m53tmp.eprom_first ? 0x01 : 0x81;
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 2, value);
	}
	map_prg_rom_8k_update();
}
BYTE extcl_cpu_rd_mem_53(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (m53tmp.prg_6000[address & 0x1FFF]);
}
BYTE extcl_save_mapper_53(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m53.reg);
	if (mode == SAVE_SLOT_READ) {
		m53_update_6000();
	}

	return (EXIT_OK);
}

INLINE static void m53_update_6000(void) {
	WORD value = (((m53.reg[0] << 4) & 0xF0) | 0x0F) + (m53tmp.eprom_first ? 0x04 : 0x00);

	control_bank(info.prg.rom.max.banks_8k)
	m53tmp.prg_6000 = prg_pnt(value << 13);
}
