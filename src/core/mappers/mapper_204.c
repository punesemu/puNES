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

INLINE static void prg_fix_204(void);
INLINE static void chr_fix_204(void);
INLINE static void mirroring_fix_204(void);

INLINE static void tmp_fix_204(BYTE max, BYTE index, const WORD *ds);

struct _m204 {
	WORD reg;
} m204;
struct _m204tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const WORD *dipswitch;
} m204tmp;

void map_init_204(void) {
	EXTCL_AFTER_MAPPER_INIT(204);
	EXTCL_CPU_WR_MEM(204);
	EXTCL_CPU_RD_MEM(204);
	EXTCL_SAVE_MAPPER(204);

	if (info.reset >= HARD) {
		memset(&m204, 0x00, sizeof(m204));
	}

	if (info.reset == RESET) {
		if (m204tmp.dipswitch) {
			m204tmp.index = (m204tmp.index + 1) % m204tmp.max;
		}
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memset(&m204tmp, 0x00, sizeof(m204tmp));

		{
			static const WORD ds[] = { 0x00 };

			tmp_fix_204(LENGTH(ds), 0, &ds[0]);
		}
	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_204(void) {
	prg_fix_204();
	chr_fix_204();
	mirroring_fix_204();
}
void extcl_cpu_wr_mem_204(WORD address, UNUSED(BYTE value)) {
	m204.reg = address;
	prg_fix_204();
	chr_fix_204();
	mirroring_fix_204();
}
BYTE extcl_cpu_rd_mem_204(WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		switch (m204.reg & 0xFF0F) {
			case 0xF004:
				return (prgrom_size() <= S64K ? m204tmp.dipswitch[m204tmp.index] & 0x00FF : prgrom_rd(address));
			case 0xF008:
				return ((m204tmp.dipswitch[m204tmp.index] & 0xFF00) >> 8);
			default:
				return (prgrom_rd(address));
		}
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_204(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m204.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_204(void) {
	if (m204.reg & 0x20) {
		memmap_auto_32k(MMCPU(0x8000), (m204.reg >> 1));
	} else {
		memmap_auto_16k(MMCPU(0x8000), m204.reg);
		memmap_auto_16k(MMCPU(0xC000), m204.reg);
	}
}
INLINE static void chr_fix_204(void) {
	memmap_auto_8k(MMPPU(0x0000), m204.reg & (m204.reg & 0x20 ? 0x0E : 0x0F));
}
INLINE static void mirroring_fix_204(void) {
	if (m204.reg & 0x10) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}

INLINE static void tmp_fix_204(BYTE max, BYTE index, const WORD *ds) {
	m204tmp.ds_used = TRUE;
	m204tmp.max = max;
	m204tmp.index = index;
	m204tmp.dipswitch = ds;
}
