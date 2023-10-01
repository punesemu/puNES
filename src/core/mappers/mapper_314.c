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
#include "cpu.h"
#include "save_slot.h"

INLINE static void prg_fix_314(void);
INLINE static void chr_fix_314(void);
INLINE static void mirroring_fix_314(void);

struct _m314 {
	BYTE reg[5];
} m314;

void map_init_314(void) {
	EXTCL_AFTER_MAPPER_INIT(314);
	EXTCL_CPU_WR_MEM(314);
	EXTCL_SAVE_MAPPER(314);
	mapper.internal_struct[0] = (BYTE *)&m314;
	mapper.internal_struct_size[0] = sizeof(m314);

	if (info.reset >= HARD) {
		memset(&m314, 0x00, sizeof(m314));

		m314.reg[0] = 0x80;
		m314.reg[1] = 0x43;
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_314(void) {
	prg_fix_314();
	chr_fix_314();
	mirroring_fix_314();
}
void extcl_cpu_wr_mem_314(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		m314.reg[address & 0x03] = value;
		prg_fix_314();
		chr_fix_314();
		mirroring_fix_314();
	} else if (address >= 0x8000) {
		// bus conflict
		m314.reg[4] = value & prgrom_rd(nidx, address);
		prg_fix_314();
	}
}
BYTE extcl_save_mapper_314(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m314.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_314(void) {
	WORD bank = 0;

	if (m314.reg[0] & 0x80) {
		if (m314.reg[1] & 0x80) {
			bank = (m314.reg[1] & 0x3F);
			memmap_auto_32k(0, MMCPU(0x8000), bank);
		} else {
			bank = ((m314.reg[1] & 0x3F) << 1) | ((m314.reg[1] & 0x40) >> 6);
			memmap_auto_16k(0, MMCPU(0x8000), bank);
			memmap_auto_16k(0, MMCPU(0xC000), bank);
		}
	} else {
		bank = ((m314.reg[1] & 0x7C) << 1) | (m314.reg[3] & 0x07);
		memmap_auto_16k(0, MMCPU(0x8000), bank);
		bank = (m314.reg[1] << 1) | 0x07;
		memmap_auto_16k(0, MMCPU(0xC000), bank);
	}
}
INLINE static void chr_fix_314(void) {
	memmap_auto_8k(0, MMPPU(0x0000), ((m314.reg[2] << 2) | ((m314.reg[0] & 0x06) >> 1)));
}
INLINE static void mirroring_fix_314(void) {
	if (m314.reg[0] & 0x20) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
