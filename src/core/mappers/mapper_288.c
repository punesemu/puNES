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

INLINE static void prg_fix_288(void);
INLINE static void chr_fix_288(void);

INLINE static void tmp_fix_288(BYTE max, BYTE index, const WORD *ds);

struct _m288 {
	WORD reg;
} m288;
struct _m288tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const WORD *dipswitch;
} m288tmp;

void map_init_288(void) {
	EXTCL_AFTER_MAPPER_INIT(288);
	EXTCL_CPU_WR_MEM(288);
	EXTCL_CPU_RD_MEM(288);
	EXTCL_SAVE_MAPPER(288);
	mapper.internal_struct[0] = (BYTE *)&m288;
	mapper.internal_struct_size[0] = sizeof(m288);

	if (info.reset >= HARD) {
		memset(&m288, 0x00, sizeof(m288));
	}

	if (info.reset == RESET) {
		if (m288tmp.ds_used) {
			m288tmp.index = (m288tmp.index + 1) % m288tmp.max;
		}
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memset(&m288tmp, 0x00, sizeof(m288tmp));

		if (info.crc32.prg == 0xC6A91589) { // 21-in-1 (GA-003).nes
			static const WORD ds[] = { 0, 8, 2, 1, 3, 5, 4, 6, 9, 10, 11, 13, 14, 7 };

			tmp_fix_288(LENGTH(ds), 0, &ds[0]);
		} else if (info.crc32.prg == 0xEE24B155) { // 64-in-1 (CF-015).nes
			static const WORD ds[] = { 8, 3, 5, 4, 6, 9, 10, 11, 13, 14, 7, 1, 2, 0 };

			tmp_fix_288(LENGTH(ds), 0, &ds[0]);
		} else {
			static WORD ds[] = { 0x00 };

			tmp_fix_288(LENGTH(ds), 0, &ds[0]);
		}
	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_288(void) {
	prg_fix_288();
	chr_fix_288();
}
void extcl_cpu_wr_mem_288(WORD address, UNUSED(BYTE value)) {
	m288.reg = address;
	prg_fix_288();
	chr_fix_288();
}
BYTE extcl_cpu_rd_mem_288(WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		return (m288.reg & 0x0020 ? prgrom_rd((address | m288tmp.dipswitch[m288tmp.index])) : prgrom_rd(address));
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_288(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m288.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_288(void) {
	memmap_auto_32k(MMCPU(0x8000), (m288.reg >> 3));
}
INLINE static void chr_fix_288(void) {
	memmap_auto_8k(MMPPU(0x0000), (m288.reg & 0x07));
}

INLINE static void tmp_fix_288(BYTE max, BYTE index, const WORD *ds) {
	m288tmp.ds_used = TRUE;
	m288tmp.max = max;
	m288tmp.index = index;
	m288tmp.dipswitch = ds;
}
