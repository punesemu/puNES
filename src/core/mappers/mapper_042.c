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
#include "cpu.h"

INLINE static void prg_fix_042_s1(void);
INLINE static void chr_fix_042_s1(void);
INLINE static void wram_fix_042_s1(void);

void prg_fix_n118_042_s2(void);

INLINE static void wram_fix_042_s2(void);
INLINE static void mirroring_fix_042_s2(void);

INLINE static void prg_fix_042_s3(void);
INLINE static void wram_fix_042_s3(void);
INLINE static void mirroring_fix_042_s3(void);

struct _m042 {
	WORD prg;
	WORD chr;
	BYTE mirroring;
	struct _m042_s3_irq {
		BYTE reg;
		uint16_t count;
	} irq;
} m042;

void map_init_042(void) {
	if ((info.mapper.submapper < 1) || (info.mapper.submapper > 3)) {
		if (chrrom_size()) {
			// Ai Senshi Nicole
			info.mapper.submapper = 1;
		} else if (prgrom_size() > S128K) {
			// Green Beret
			info.mapper.submapper = 2;
		} else {
			// Mario Baby
			info.mapper.submapper = 3;
		}
	}

	switch(info.mapper.submapper) {
		default:
		case 1:
			EXTCL_AFTER_MAPPER_INIT(042_s1);
			EXTCL_CPU_WR_MEM(042_s1);
			EXTCL_SAVE_MAPPER(042_s1);
			mapper.internal_struct[0] = (BYTE *)&m042;
			mapper.internal_struct_size[0] = sizeof(m042);

			if (info.reset >= HARD) {
				memset(&m042, 0x00, sizeof(m042));

				m042.prg = 0xFF;
			}
			break;
		case 2:
			EXTCL_AFTER_MAPPER_INIT(042_s2);
			EXTCL_CPU_WR_MEM(042_s2);
			EXTCL_SAVE_MAPPER(042_s2);
			mapper.internal_struct[0] = (BYTE *)&m042;
			mapper.internal_struct_size[0] = sizeof(m042);
			mapper.internal_struct[1] = (BYTE *)&n118;
			mapper.internal_struct_size[1] = sizeof(n118);

			if (info.reset >= HARD) {
				memset(&m042, 0x00, sizeof(m042));
			}

			init_N118(info.reset);
			N118_prg_fix = prg_fix_n118_042_s2;

			info.mapper.extend_wr = TRUE;
			return;
		case 3:
			EXTCL_AFTER_MAPPER_INIT(042_s3);
			EXTCL_CPU_WR_MEM(042_s3);
			EXTCL_SAVE_MAPPER(042_s3);
			EXTCL_CPU_EVERY_CYCLE(042_s3);
			mapper.internal_struct[0] = (BYTE *)&m042;
			mapper.internal_struct_size[0] = sizeof(m042);

			if (info.reset >= HARD) {
				memset(&m042, 0x00, sizeof(m042));
			}
			return;
	}
}

// submapper 1 -----------------------------------------------------------------

void extcl_after_mapper_init_042_s1(void) {
	prg_fix_042_s1();
	chr_fix_042_s1();
	wram_fix_042_s1();
}
void extcl_cpu_wr_mem_042_s1(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x8000:
			m042.chr = value;
			chr_fix_042_s1();
			return;
		case 0xE000:
			m042.prg = value;
			wram_fix_042_s1();
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_042_s1(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m042.prg);
	save_slot_ele(mode, slot, m042.chr);
	return (EXIT_OK);
}

INLINE static void prg_fix_042_s1(void) {
	memmap_auto_32k(0, MMCPU(0x8000),  0xFF);
}
INLINE static void chr_fix_042_s1(void) {
	memmap_auto_8k(0, MMPPU(0x0000), m042.chr);
}
INLINE static void wram_fix_042_s1(void) {
	memmap_prgrom_8k(0, MMCPU(0x6000),  m042.prg);
}

// submapper 2 -----------------------------------------------------------------

void extcl_after_mapper_init_042_s2(void) {
	extcl_after_mapper_init_N118();
	mirroring_fix_042_s2();
}
void extcl_cpu_wr_mem_042_s2(BYTE nidx, WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x4000:
			if (address == 0x4025) {
				m042.mirroring = value;
				mirroring_fix_042_s2();
			}
			return;
		default:
			extcl_cpu_wr_mem_N118(nidx, address, value);
			return;
	}
}
BYTE extcl_save_mapper_042_s2(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m042.mirroring);
	return (extcl_save_mapper_N118(mode, slot, fp));
}

void prg_fix_n118_042_s2(void) {
	memmap_prgrom_32k(0, MMCPU(0x8000), (prgrom_banks(S16K) & 0x0F ? 4 : 7));
	wram_fix_042_s2();
}

INLINE static void wram_fix_042_s2(void) {
	memmap_prgrom_8k(0, MMCPU(0x6000),  ((n118.reg[5] & 0x1E) >> 1));
}
INLINE static void mirroring_fix_042_s2(void) {
	if (m042.mirroring & 0x08) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}

// submapper 3 -----------------------------------------------------------------

void extcl_after_mapper_init_042_s3(void) {
	prg_fix_042_s3();
	wram_fix_042_s3();
	mirroring_fix_042_s3();
}
void extcl_cpu_wr_mem_042_s3(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xE003) {
		case 0xE000:
			m042.prg = value;
			wram_fix_042_s3();
			return;
		case 0xE001:
			m042.mirroring = value;
			mirroring_fix_042_s3();
			return;
		case 0xE002:
			m042.irq.reg = value;
			return;
		default:
			return;
	}
}
void extcl_cpu_every_cycle_042_s3(BYTE nidx) {
	if (m042.irq.reg & 0x02) {
		if ((++m042.irq.count & 0x6000) == 0x6000) {
			nes[nidx].c.irq.high |= EXT_IRQ;
		} else {
			nes[nidx].c.irq.high &= ~EXT_IRQ;
		}
	} else {
		nes[nidx].c.irq.high &= ~EXT_IRQ;
		m042.irq.count = 0;
	}
}
BYTE extcl_save_mapper_042_s3(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m042.prg);
	save_slot_ele(mode, slot, m042.mirroring);
	save_slot_ele(mode, slot, m042.irq.reg);
	save_slot_ele(mode, slot, m042.irq.count);
	return (EXIT_OK);
}

INLINE static void prg_fix_042_s3(void) {
	memmap_prgrom_32k(0, MMCPU(0x8000),  0xFF);
}
INLINE static void wram_fix_042_s3(void) {
	memmap_prgrom_8k(0, MMCPU(0x6000),  m042.prg);
}
INLINE static void mirroring_fix_042_s3(void) {
	if (m042.mirroring & 0x08) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
