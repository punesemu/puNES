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

INLINE static void sync_031(void);

struct _m031 {
	WORD regs[8];
	BYTE *rom_4k[8];
} m031;

void map_init_031(void) {
	EXTCL_CPU_WR_MEM(031);
	EXTCL_CPU_RD_MEM(031);
	EXTCL_SAVE_MAPPER(031);

	if (info.reset >= HARD) {
		memset(&m031, 0x00, sizeof(m031));
		m031.regs[7] = 0xFF;
		sync_031();
	}

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;
}
void extcl_cpu_wr_mem_031(WORD address, BYTE value) {
	if ((address < 0x5000) || (address > 0x5FFF)) {
		return;
	}
	m031.regs[address & 0x0007] = value;
	sync_031();
}
BYTE extcl_cpu_rd_mem_031(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (address < 0x8000) {
		return (openbus);
	}
	return (m031.rom_4k[(address >> 12) & 0x07][address & 0x0FFF]);
}
BYTE extcl_save_mapper_031(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m031.regs);

	if (mode == SAVE_SLOT_READ) {
		sync_031();
	}

	return (EXIT_OK);
}

INLINE static void sync_031(void) {
	BYTE i, value;

	for (i = 0; i < 8; ++i) {
		value = m031.regs[i];
		control_bank(info.prg.rom.max.banks_4k);
		m031.rom_4k[i] = prg_pnt(value << 12);
	}
}
