/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

INLINE static void prg_fix_286(void);
INLINE static void chr_fix_286(void);
INLINE static void mirroring_fix_286(void);

struct _m286 {
	BYTE prg[4];
	BYTE chr[4];
	BYTE mirroring;
} m286;

void map_init_286(void) {
	EXTCL_AFTER_MAPPER_INIT(286);
	EXTCL_CPU_WR_MEM(286);
	EXTCL_SAVE_MAPPER(286);
	map_internal_struct_init((BYTE *)&m286, sizeof(m286));

	if (info.reset >= HARD) {
		memset(&m286, 0x00, sizeof(m286));

		m286.prg[0] = 0x0C;
		m286.prg[1] = 0x0D;
		m286.prg[2] = 0x0E;
		m286.prg[3] = 0x0F;
	}
}
void extcl_after_mapper_init_286(void) {
	prg_fix_286();
	chr_fix_286();
	mirroring_fix_286();
}
void extcl_cpu_wr_mem_286(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	switch (address & 0xF000) {
		case 0x8000:
		case 0x9000:
			m286.chr[(address >> 10) & 0x03] = address & 0x1F;
			chr_fix_286();
			return;
		case 0xA000:
		case 0xB000:
			if (address & dipswitch.value) {
				m286.prg[(address >> 10) & 0x03] = address & 0x0F;
				prg_fix_286();
			}
			return;
		case 0xC000:
			m286.mirroring = address & 0x01;
			mirroring_fix_286();
			return;
	}
}
BYTE extcl_save_mapper_286(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m286.prg);
	save_slot_ele(mode, slot, m286.chr);
	save_slot_ele(mode, slot, m286.mirroring);
	return (EXIT_OK);
}

INLINE static void prg_fix_286(void) {
	memmap_auto_8k(0, MMCPU(0x8000), m286.prg[0]);
	memmap_auto_8k(0, MMCPU(0xA000), m286.prg[1]);
	memmap_auto_8k(0, MMCPU(0xC000), m286.prg[2]);
	memmap_auto_8k(0, MMCPU(0xE000), m286.prg[3]);
}
INLINE static void chr_fix_286(void) {
	memmap_auto_2k(0, MMPPU(0x0000), m286.chr[0]);
	memmap_auto_2k(0, MMPPU(0x0800), m286.chr[1]);
	memmap_auto_2k(0, MMPPU(0x1000), m286.chr[2]);
	memmap_auto_2k(0, MMPPU(0x1800), m286.chr[3]);
}
INLINE static void mirroring_fix_286(void) {
	if (m286.mirroring) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
