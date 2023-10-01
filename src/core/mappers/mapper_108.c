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

#include "mappers.h"
#include "save_slot.h"

INLINE static void prg_fix_108(void);
INLINE static void chr_fix_108(void);
INLINE static void wram_fix_108(void);

struct _m108 {
	BYTE reg;
} m108;
struct _m108tmp {
	WORD start;
} m108tmp;

void map_init_108(void) {
	EXTCL_AFTER_MAPPER_INIT(108);
	EXTCL_CPU_WR_MEM(108);
	EXTCL_SAVE_MAPPER(108);
	mapper.internal_struct[0] = (BYTE *)&m108;
	mapper.internal_struct_size[0] = sizeof(m108);

	if (info.reset >= HARD) {
		m108.reg = 0;
	}

	if (!info.mapper.submapper || (info.mapper.submapper > 4)) {
		info.mapper.submapper = !chrrom_size()
			? info.mapper.mirroring != MIRRORING_VERTICAL ? 1 : 3
			: prgrom_size() <= S32K ? 4 : 2;
	}

	switch (info.mapper.submapper) {
		default:
		case 1:
			m108tmp.start = 0xF000;
			break;
		case 2:
			m108tmp.start = 0xE000;
			break;
		case 3:
			m108tmp.start = 0x8000;
			break;
		case 4:
			m108tmp.start = 0x8000;
			break;
	}
}
void extcl_after_mapper_init_108(void) {
	prg_fix_108();
	chr_fix_108();
	wram_fix_108();
}
void extcl_cpu_wr_mem_108(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if (address >= m108tmp.start) {
		m108.reg = value;
		prg_fix_108();
		chr_fix_108();
		wram_fix_108();
	}
}
BYTE extcl_save_mapper_108(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m108.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_108(void) {
	memmap_prgrom_32k(0, MMCPU(0x8000), 0xFF);
}
INLINE static void chr_fix_108(void) {
	if (chrrom_size()) {
		memmap_auto_8k(0, MMPPU(0x0000), m108.reg);
	}
}
INLINE static void wram_fix_108(void) {
	memmap_prgrom_8k(0, MMCPU(0x6000), info.mapper.submapper == 4 ? 0xFF : m108.reg);
}
