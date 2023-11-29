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

INLINE static void prg_fix_033(void);
INLINE static void chr_fix_033(void);
INLINE static void mirroring_fix_033(void);

struct _m033 {
	BYTE prg[2];
	BYTE chr[6];
} m033;

void map_init_033(void) {
	EXTCL_AFTER_MAPPER_INIT(033);
	EXTCL_CPU_WR_MEM(033);
	EXTCL_SAVE_MAPPER(033);
	map_internal_struct_init((BYTE *)&m033, sizeof(m033));

	if (info.reset >= HARD) {
		memset(&m033, 0x00, sizeof(m033));

		m033.prg[1] = 0x01;
		m033.chr[1] = 0x01;
		m033.chr[2] = 0x04;
		m033.chr[3] = 0x05;
		m033.chr[4] = 0x06;
		m033.chr[5] = 0x07;
	}
}
void extcl_after_mapper_init_033(void) {
	prg_fix_033();
	chr_fix_033();
	mirroring_fix_033();
}
void extcl_cpu_wr_mem_033(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xE003) {
		case 0x8000:
		case 0x8001:
			m033.prg[address & 0x01] = value;
			prg_fix_033();
			mirroring_fix_033();
			return;
		case 0x8002:
		case 0x8003:
			m033.chr[address & 0x01] = value;
			chr_fix_033();
			return;
		case 0xA000:
		case 0xA001:
		case 0xA002:
		case 0xA003:
			m033.chr[(address & 0x03) + 2] = value;
			chr_fix_033();
			return;
	}
}
BYTE extcl_save_mapper_033(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m033.prg);
	save_slot_ele(mode, slot, m033.chr);
	return (EXIT_OK);
}

INLINE static void prg_fix_033(void) {
	memmap_auto_8k(0, MMCPU(0x8000), m033.prg[0]);
	memmap_auto_8k(0, MMCPU(0xA000), m033.prg[1]);
	memmap_auto_16k(0, MMCPU(0xC000), 0xFF);
}
INLINE static void chr_fix_033(void) {
	memmap_auto_2k(0, MMPPU(0x0000), m033.chr[0]);
	memmap_auto_2k(0, MMPPU(0x0800), m033.chr[1]);
	memmap_auto_1k(0, MMPPU(0x1000), m033.chr[2]);
	memmap_auto_1k(0, MMPPU(0x1400), m033.chr[3]);
	memmap_auto_1k(0, MMPPU(0x1800), m033.chr[4]);
	memmap_auto_1k(0, MMPPU(0x1C00), m033.chr[5]);
}
INLINE static void mirroring_fix_033(void) {
	if (m033.prg[0] & 0x40) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
