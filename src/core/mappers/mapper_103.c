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
#include "fds.h"
#include "save_slot.h"

struct _m103 {
	WORD reg;
} m103;
struct _m103tmp {
	BYTE *prg_6000;
} m103tmp;

void map_init_103(void) {
	EXTCL_AFTER_MAPPER_INIT(103);
	EXTCL_CPU_WR_MEM(103);
	EXTCL_CPU_RD_MEM(103);
	EXTCL_SAVE_MAPPER(103);
	EXTCL_APU_TICK(103);
	mapper.internal_struct[0] = (BYTE *)&m103;
	mapper.internal_struct_size[0] = sizeof(m103);

	if (info.reset >= HARD) {
		memset(&m103, 0x00, sizeof(m103));
	}

	{
		BYTE value = 3;

		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}

	info.prg.ram.banks_8k_plus = 2;

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_103(void) {
	extcl_cpu_wr_mem_103(0x8000, 0x00);
}
void extcl_cpu_wr_mem_103(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			control_bank(info.prg.rom.max.banks_8k)
			m103tmp.prg_6000 = prg_pnt(value << 13);
			break;
		case 0xB000:
		case 0xC000:
		case 0xD000:
			if ((address >= 0xB800) && (address <= 0xD7FF)) {
				prg.ram_plus_8k[0x2000 + (address - 0xB800)] = value;
			}
			break;
		case 0xE000:
			if (value & 0x08) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			break;
		case 0xF000:
			m103.reg = value;
			break;
	}
}
BYTE extcl_cpu_rd_mem_103(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (m103.reg & 0x10) {
			return (m103tmp.prg_6000[address & 0x1FFF]);
		}
		return (openbus);
	}
	if ((address >= 0xB800) && (address <= 0xD7FF)) {
		if (!(m103.reg & 0x10)) {
			return (prg.ram_plus_8k[0x2000 + (address - 0xB800)]);
		}
	}
	return (openbus);
}
BYTE extcl_save_mapper_103(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m103.reg);

	return (EXIT_OK);
}
