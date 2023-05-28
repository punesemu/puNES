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

	if ((info.mapper.submapper == 0) || (info.mapper.submapper > 4)) {
		if (mapper.write_vram) {
			info.mapper.submapper = mapper.mirroring != MIRRORING_VERTICAL ? M108_1 : M108_3;
		} else {
			info.mapper.submapper = chr_size() > (1024 * 16) ? M108_2 : M108_4;
		}
	}

	switch (info.mapper.submapper) {
		default:
		case M108_1:
			m108tmp.start = 0xF000;
			break;
		case M108_2:
			m108tmp.start = 0xE000;
			break;
		case M108_3:
			m108tmp.start = 0x8000;
			break;
		case M108_4:
			m108tmp.start = 0x8000;
			break;
	}
}
void extcl_after_mapper_init_108(void) {
	prg_fix_108();
	chr_fix_108();
	wram_fix_108();
}
void extcl_cpu_wr_mem_108(WORD address, BYTE value) {
	if (address >= m108tmp.start) {
		m108.reg = value;
		prg_fix_108();
		chr_fix_108();
		wram_fix_108();
	}
}
BYTE extcl_save_mapper_108(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m108.reg);

	if (mode == SAVE_SLOT_READ) {
		prg_fix_108();
	}

	return (EXIT_OK);
}

INLINE static void prg_fix_108(void) {
	memmap_prgrom_32k(MMCPU(0x8000), 0xFF);
}
INLINE static void chr_fix_108(void) {
	if (!mapper.write_vram) {
		DBWORD bank = m108.reg;

		_control_bank(bank, info.chr.rom.max.banks_8k);
		bank <<= 13;
		chr.bank_1k[0] = chr_pnt(bank);
		chr.bank_1k[1] = chr_pnt(bank | 0x0400);
		chr.bank_1k[2] = chr_pnt(bank | 0x0800);
		chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
		chr.bank_1k[4] = chr_pnt(bank | 0x1000);
		chr.bank_1k[5] = chr_pnt(bank | 0x1400);
		chr.bank_1k[6] = chr_pnt(bank | 0x1800);
		chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
	}
}
INLINE static void wram_fix_108(void) {
	memmap_prgrom_8k(MMCPU(0x6000), info.mapper.submapper == 4 ? 0xFF : m108.reg);
}
