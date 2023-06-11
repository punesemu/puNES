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
#include "save_slot.h"

INLINE static void prg_fix_053(void);
INLINE static void wram_fix_053(void);
INLINE static void mirroring_fix_053(void);

struct _m053 {
	BYTE reg[2];
} m053;

void map_init_053(void) {
	EXTCL_AFTER_MAPPER_INIT(053);
	EXTCL_CPU_WR_MEM(053);
	EXTCL_SAVE_MAPPER(053);
	mapper.internal_struct[0] = (BYTE *)&m053;
	mapper.internal_struct_size[0] = sizeof(m053);

	memset(&m053, 0x00, sizeof(m053));

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_053(void) {
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		size_t size = prgrom_size() - 0x8000;

		// Supervision 16-in-1 [U][p1][!].unf
		if (size == 0x200000) {
			BYTE *buffer = malloc(size);

			miscrom_set_size(0x8000);
			miscrom_init();

			memcpy(miscrom_pnt(), prgrom_pnt(), miscrom_size());
			memcpy(buffer, prgrom_pnt_byte(0x8000), size);

			prgrom_set_size(size);
			prgrom_init(0x00);
			memcpy(prgrom_pnt(), buffer, size);
			free(buffer);
		}
	}
	prg_fix_053();
	wram_fix_053();
	mirroring_fix_053();
}
void extcl_cpu_wr_mem_053(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (!(m053.reg[0] & 0x10)) {
			m053.reg[0] = value;
			prg_fix_053();
			wram_fix_053();
			mirroring_fix_053();
		}
		return;
	}
	if (address >= 0x8000) {
		m053.reg[1] = value;
		prg_fix_053();
	}
}
BYTE extcl_save_mapper_053(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m053.reg);

	if (mode == SAVE_SLOT_READ) {
		wram_fix_053();
	}

	return (EXIT_OK);
}

INLINE static void prg_fix_053(void) {
	if (m053.reg[0] & 0x10) {
		WORD base = m053.reg[0] << 3;

		memmap_auto_16k(MMCPU(0x8000), (base | (m053.reg[1] & 0x07)));
		memmap_auto_16k(MMCPU(0xC000), (base | 0x07));
	} else {
		memmap_other_32k(MMCPU(0x8000), 0, miscrom_pnt(), miscrom_size(), TRUE, FALSE);
	}
}
INLINE static void wram_fix_053(void) {
	memmap_prgrom_8k(MMCPU(0x6000), ((m053.reg[0] << 4) | 0x0F));
}
INLINE static void mirroring_fix_053(void) {
	if (m053.reg[0] & 0x20) {
		mirroring_H();
	} else  {
		mirroring_V();
	}
}
