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

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_228(void);
INLINE static void chr_fix_228(void);
INLINE static void mirroring_fix_228(void);

struct _m228 {
	WORD reg[2];
} m228;

void map_init_228(void) {
	EXTCL_AFTER_MAPPER_INIT(228);
	EXTCL_CPU_WR_MEM(228);
	EXTCL_SAVE_MAPPER(228);
	mapper.internal_struct[0] = (BYTE *)&m228;
	mapper.internal_struct_size[0] = sizeof(m228);

	if (info.reset >= HARD) {
		memset(&m228, 0x00, sizeof(m228));
	}
}
void extcl_after_mapper_init_228(void) {
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (prgrom_size() == 0x180000) {
			size_t size = 0x200000;
			BYTE *buffer = malloc(size);

			memset(buffer, 0x00, size);
			memcpy(buffer, prgrom_pnt(), prgrom_size());

			for (int i = 0x000000; i < 0x080000; i++) {
				buffer[0x180000 +i] = buffer[0x100000 +i];
				buffer[0x100000 +i] = (i >> 8) & 0xFF;
			}

			prgrom_set_size(size);
			prgrom_init(0x00);
			memcpy(prgrom_pnt(), buffer, size);
			free(buffer);
		}
	}
	prg_fix_228();
	chr_fix_228();
	mirroring_fix_228();
}
void extcl_cpu_wr_mem_228(WORD address, BYTE value) {
	m228.reg[0] = address;
	m228.reg[1] = value;
	prg_fix_228();
	chr_fix_228();
	mirroring_fix_228();
}

BYTE extcl_save_mapper_228(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m228.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_228(void) {
	WORD bank = (m228.reg[0] >> 6) & 0x7F;

	if (m228.reg[0] & 0x20) {
		memmap_auto_16k(MMCPU(0x8000), bank);
		memmap_auto_16k(MMCPU(0xC000), bank);
	} else {
		memmap_auto_32k(MMCPU(0x8000), (bank >> 1));
	}
}
INLINE static void chr_fix_228(void) {
	memmap_auto_8k(MMPPU(0x0000), (((m228.reg[0] << 2) & 0x3C) | (m228.reg[1] & 0x03)));
}
INLINE static void mirroring_fix_228(void) {
	if (m228.reg [0] & 0x2000) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
