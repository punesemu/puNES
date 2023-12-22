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

INLINE static void prg_fix_354(void);
INLINE static void chr_fix_354(void);
INLINE static void wram_fix_354(void);
INLINE static void mirroring_fix_354(void);

struct _m354 {
	WORD reg[2];
} m354;

void map_init_354(void) {
	EXTCL_AFTER_MAPPER_INIT(354);
	EXTCL_CPU_WR_MEM(354);
	EXTCL_SAVE_MAPPER(354);
	map_internal_struct_init((BYTE *)&m354, sizeof(m354));

	memset(&m354, 0x00, sizeof(m354));
}
void extcl_after_mapper_init_354(void) {
	prg_fix_354();
	chr_fix_354();
	wram_fix_354();
	mirroring_fix_354();
}
void extcl_cpu_wr_mem_354(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if (address >= (info.mapper.submapper == 0 ? 0xF000 : 0xE000)) {
		m354.reg[0] = address;
		m354.reg[1] = value;
		prg_fix_354();
		chr_fix_354();
		wram_fix_354();
		mirroring_fix_354();
	}
}
BYTE extcl_save_mapper_354(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m354.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_354(void) {
	DBWORD bank = ((m354.reg[0] & 0x1000) >> 5) | ((m354.reg[0] & 0x10) << 2) | (m354.reg[1] & 0x3F);

	switch (m354.reg[0] & 0x07) {
		default:
		case 0:
		case 4:
			memmap_auto_32k(0, MMCPU(0x8000), (bank >> 1));
			break;
		case 1:
			memmap_auto_16k(0, MMCPU(0x8000), bank);
			memmap_auto_16k(0, MMCPU(0xC000), (bank | 0x07));
			break;
		case 2:
		case 6:
			memmap_auto_8k(0, MMCPU(0x8000), ((bank << 1) | (m354.reg[1] >> 7)));
			memmap_auto_8k(0, MMCPU(0xA000), ((bank << 1) | (m354.reg[1] >> 7)));
			memmap_auto_8k(0, MMCPU(0xC000), ((bank << 1) | (m354.reg[1] >> 7)));
			memmap_auto_8k(0, MMCPU(0xE000), ((bank << 1) | (m354.reg[1] >> 7)));
			break;
		case 3:
		case 7:
			memmap_auto_16k(0, MMCPU(0x8000), bank);
			memmap_auto_16k(0, MMCPU(0xC000), bank);
			break;
		case 5:
			memmap_auto_32k(0, MMCPU(0x8000), ((bank >> 1) | 0x03));
			break;
	}
}
INLINE static void chr_fix_354(void) {
	BYTE enabled = !(m354.reg[0] & 0x08);

	memmap_auto_wp_8k(0, MMPPU(0x0000), 0, TRUE, enabled);
}
INLINE static void wram_fix_354(void) {
	DBWORD bank = ((m354.reg[0] & 0x1000) >> 5) | ((m354.reg[0] & 0x10) << 2) | (m354.reg[1] & 0x3F);

	if ((m354.reg[0] & 0x0007) == 5) {
		memmap_prgrom_8k(0, MMCPU(0x6000), ((bank << 1) | (m354.reg[1] >> 7)));
	} else {
		memmap_disable_8k(0, MMCPU(0x6000));
	}
}
INLINE static void mirroring_fix_354(void) {
	if (m354.reg[1] & 0x40) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
