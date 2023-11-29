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

INLINE static void prg_fix_034_s1(void);
INLINE static void chr_fix_034_s1(void);

INLINE static void prg_fix_034_s2(void);

INLINE static void prg_fix_034_nstcl(void);
INLINE static void chr_fix_034_nstcl(void);

struct _m034 {
	BYTE reg[3];
} m034;

void map_init_034(void) {
	BYTE type = 0;
	
	if (miscrom_size() && miscrom_pnt()) {
		type = 3;
	} else if ((info.mapper.submapper == 1) || ((info.mapper.submapper != 2) && chrrom_size())) {
		info.mapper.submapper = 1;
		type = 1;
	} else if ((info.mapper.submapper == 2) || ((info.mapper.submapper != 1) && !chrrom_size())) {
		info.mapper.submapper = 2;
		type = 2;
	}

	switch(type) {
		default:
		case 1: // NINA-01
			EXTCL_AFTER_MAPPER_INIT(034_s1);
			EXTCL_CPU_WR_MEM(034_s1);
			EXTCL_SAVE_MAPPER(034_s1);
			map_internal_struct_init((BYTE *)&m034, sizeof(m034));

			if (info.reset >= HARD) {
				memset(&m034, 0x00, sizeof(m034));
			}

			info.mapper.extend_wr = TRUE;
			return;
		case 2: // BNROM
			EXTCL_AFTER_MAPPER_INIT(034_s2);
			EXTCL_CPU_WR_MEM(034_s2);
			EXTCL_SAVE_MAPPER(034_s2);
			map_internal_struct_init((BYTE *)&m034, sizeof(m034));

			if (info.reset >= HARD) {
				memset(&m034, 0x00, sizeof(m034));
			}
			return;
		case 3: // Nesticle
			EXTCL_AFTER_MAPPER_INIT(034_nstcl);
			EXTCL_CPU_WR_MEM(034_nstcl);
			EXTCL_SAVE_MAPPER(034_nstcl);
			map_internal_struct_init((BYTE *)&m034, sizeof(m034));

			if (info.reset >= HARD) {
				memset(&m034, 0x00, sizeof(m034));
			}

			info.mapper.extend_wr = TRUE;
			return;
	}
}

// submapper 1 ----------------------------------------------------------------

void extcl_after_mapper_init_034_s1(void) {
	prg_fix_034_s1();
	chr_fix_034_s1();
}
void extcl_cpu_wr_mem_034_s1(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0x7FFD) && (address <= 0x7FFF)) {
		m034.reg[address - 0x7FFD] = value;
		prg_fix_034_s1();
		chr_fix_034_s1();
	}
}
BYTE extcl_save_mapper_034_s1(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m034.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_034_s1(void) {
	memmap_auto_32k(0, MMCPU(0x8000), m034.reg[0]);
}
INLINE static void chr_fix_034_s1(void) {
	memmap_auto_4k(0, MMPPU(0x0000), m034.reg[1]);
	memmap_auto_4k(0, MMPPU(0x1000), m034.reg[2]);
}

// submapper 2 -----------------------------------------------------------------

void extcl_after_mapper_init_034_s2(void) {
	prg_fix_034_s2();
}
void extcl_cpu_wr_mem_034_s2(BYTE nidx, UNUSED(WORD address), BYTE value) {
	// bus conflict
	m034.reg[0] = value & prgrom_rd(nidx, address);
	prg_fix_034_s2();
}
BYTE extcl_save_mapper_034_s2(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m034.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_034_s2(void) {
	memmap_auto_32k(0, MMCPU(0x8000), m034.reg[0]);
}

// nesticle --------------------------------------------------------------------

void extcl_after_mapper_init_034_nstcl(void) {
	prg_fix_034_nstcl();
	chr_fix_034_nstcl();
}
void extcl_cpu_wr_mem_034_nstcl(UNUSED(BYTE nidx), UNUSED(WORD address), BYTE value) {
	if ((address >= 0x7FFD) && (address <= 0x7FFF)) {
		m034.reg[address - 0x7FFD] = value;
		prg_fix_034_nstcl();
		chr_fix_034_nstcl();
		return;
	} else if (address >= 0x8000) {
		m034.reg[0] = value;
		prg_fix_034_nstcl();
		return;
	}
}
BYTE extcl_save_mapper_034_nstcl(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m034.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_034_nstcl(void) {
	memmap_auto_32k(0, MMCPU(0x8000), m034.reg[0]);
}
INLINE static void chr_fix_034_nstcl(void) {
	memmap_auto_4k(0, MMPPU(0x0000), m034.reg[1]);
	memmap_auto_4k(0, MMPPU(0x1000), m034.reg[2]);
}
