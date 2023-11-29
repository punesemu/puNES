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
	map_internal_struct_init((BYTE *)&m403, sizeof(m403));

	memset(&m403, 0x00, sizeof(m403));

//	if (!wram_size()) {
//		wram_set_ram_size(S8K);
//	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_403(void) {
	prg_fix_403();
	chr_fix_403();
	mirroring_fix_403();
}
void extcl_cpu_wr_mem_403(UNUSED(BYTE nidx), WORD address, BYTE value) {
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
	WORD bank = ((m403.reg[0] & 0x7E) >> 1);

	if (m403.reg[2] & 0x01) {
		memmap_auto_16k(0, MMCPU(0x8000), bank);
		memmap_auto_16k(0, MMCPU(0xC000), bank);
	} else {
		memmap_auto_32k(0, MMCPU(0x8000), (bank >> 1));
	}
}
INLINE static void chr_fix_403(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m403.reg[1] & 0x03));
}
INLINE static void mirroring_fix_403(void) {
	if (m403.reg[2] & 0x10) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
