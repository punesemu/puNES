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
#include "save_slot.h"

INLINE static void prg_fix_414(void);
INLINE static void chr_fix_414(void);
INLINE static void mirroring_fix_414(void);

INLINE static void tmp_fix_414(BYTE max, BYTE index, const WORD *ds);

struct _m414 {
	WORD reg[2];
} m414;
struct _m414tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const WORD *dipswitch;
} m414tmp;

void map_init_414(void) {
	EXTCL_AFTER_MAPPER_INIT(414);
	EXTCL_CPU_WR_MEM(414);
	EXTCL_SAVE_MAPPER(414);
	mapper.internal_struct[0] = (BYTE *)&m414;
	mapper.internal_struct_size[0] = sizeof(m414);

	if (info.reset >= HARD) {
		memset(&m414, 0x00, sizeof(m414));
	}

	if (info.reset == RESET) {
		if (m414tmp.ds_used) {
			m414tmp.index = (m414tmp.index + 1) % m414tmp.max;
		}
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memset(&m414tmp, 0x00, sizeof(m414tmp));

		{
			static const WORD ds[] = { 0x010, 0x000, 0x030, 0x070, 0x0F0 };

			tmp_fix_414(LENGTH(ds), 0, &ds[0]);
		}
	}
}
void extcl_after_mapper_init_414(void) {
	prg_fix_414();
	chr_fix_414();
	mirroring_fix_414();
}
void extcl_cpu_wr_mem_414(WORD address, BYTE value) {
	m414.reg[0] = address;
	// bus conflict
	m414.reg[1] = value & prgrom_rd(address);
	prg_fix_414();
	chr_fix_414();
	mirroring_fix_414();
}
BYTE extcl_save_mapper_414(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m414.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_414(void) {
	WORD bank = m414.reg[0] >> 1;

	if (!(m414.reg[0] & 0x0100) && (m414.reg[0] & m414tmp.dipswitch[m414tmp.index])) {
		memmap_disable_16k(MMCPU(0xC000));
	} else if (m414.reg[0] & 0x2000) {
		memmap_auto_32k(MMCPU(0x8000), (bank >> 1));
	} else {
		memmap_auto_16k(MMCPU(0x8000), bank);
		memmap_auto_16k(MMCPU(0xC000), bank);
	}
}
INLINE static void chr_fix_414(void) {
	memmap_auto_8k(MMPPU(0x0000), m414.reg[1]);
}
INLINE static void mirroring_fix_414(void) {
	if (m414.reg[0] & 0x01) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}

INLINE static void tmp_fix_414(BYTE max, BYTE index, const WORD *ds) {
	m414tmp.ds_used = TRUE;
	m414tmp.max = max;
	m414tmp.index = index;
	m414tmp.dipswitch = ds;
}
