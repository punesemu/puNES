/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

INLINE static void prg_fix_080(void);
INLINE static void chr_fix_080(void);
INLINE static void wram_fix_080(void);
INLINE static void mirroring_fix_080(void);

struct _m080 {
	BYTE prg[3];
	BYTE chr[6];
	BYTE mirroring;
} m080;

void map_init_080(void) {
	EXTCL_AFTER_MAPPER_INIT(080);
	EXTCL_CPU_WR_MEM(080);
	EXTCL_SAVE_MAPPER(080);
	map_internal_struct_init((BYTE *)&m080, sizeof(m080));

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memmap_wram_region_init(0, S128B);
		if ((info.format != NES_2_0) && info.mapper.battery) {
			wram_set_ram_size(0);
			wram_set_nvram_size(S128B);
		}
	}

	if (info.reset >= HARD) {
		memset(&m080, 0x00, sizeof(m080));

		m080.prg[1] = 0x01;
		m080.prg[2] = 0xFE;
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_080(void) {
	prg_fix_080();
	chr_fix_080();
	wram_fix_080();
	mirroring_fix_080();
}
void extcl_cpu_wr_mem_080(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0x7EF0) && (address <= 0x7EFF)) {
		switch (address) {
			case 0x7EF0:
			case 0x7EF1:
				m080.chr[address & 0x01] = value;
				chr_fix_080();
				if (info.mapper.id == 207) {
					mirroring_fix_080();
				}
				return;
			case 0x7EF2:
			case 0x7EF3:
			case 0x7EF4:
			case 0x7EF5:
				m080.chr[address & 0x07] = value;
				chr_fix_080();
				return;
			case 0x7EF6:
				m080.mirroring = value;
				mirroring_fix_080();
				return;
			case 0x7EFA:
			case 0x7EFB:
			case 0x7EFC:
			case 0x7EFD:
			case 0x7EFE:
			case 0x7EFF:
				m080.prg[(address - 0x7EFA) >> 1] = value;
				prg_fix_080();
				return;
		}
	}
}
BYTE extcl_save_mapper_080(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m080.prg);
	save_slot_ele(mode, slot, m080.chr);
	save_slot_ele(mode, slot, m080.mirroring);
	return (EXIT_OK);
}

INLINE static void prg_fix_080(void) {
	memmap_auto_8k(0, MMCPU(0x8000), m080.prg[0]);
	memmap_auto_8k(0, MMCPU(0xA000), m080.prg[1]);
	memmap_auto_8k(0, MMCPU(0xC000), m080.prg[2]);
	memmap_auto_8k(0, MMCPU(0xE000), 0xFF);
}
INLINE static void chr_fix_080(void) {
	memmap_auto_2k(0, MMPPU(0x0000), (m080.chr[0] >> 1));
	memmap_auto_2k(0, MMPPU(0x0800), (m080.chr[1] >> 1));
	memmap_auto_1k(0, MMPPU(0x1000), m080.chr[2]);
	memmap_auto_1k(0, MMPPU(0x1400), m080.chr[3]);
	memmap_auto_1k(0, MMPPU(0x1800), m080.chr[4]);
	memmap_auto_1k(0, MMPPU(0x1C00), m080.chr[5]);
}
INLINE static void wram_fix_080(void) {
	memmap_disable_8k(0, MMCPU(0x6000));
	memmap_auto_128b(0, MMCPU(0x7F00), 0);
	memmap_auto_128b(0, MMCPU(0x7F80), 0);
}
INLINE static void mirroring_fix_080(void) {
	if (info.mapper.id == 207) {
		memmap_auto_1k(0, MMPPU(0x2000), (m080.chr[0] >> 7));
		memmap_auto_1k(0, MMPPU(0x2400), (m080.chr[0] >> 7));
		memmap_auto_1k(0, MMPPU(0x2800), (m080.chr[1] >> 7));
		memmap_auto_1k(0, MMPPU(0x2C00), (m080.chr[1] >> 7));

		memmap_auto_1k(0, MMPPU(0x3000), (m080.chr[0] >> 7));
		memmap_auto_1k(0, MMPPU(0x3400), (m080.chr[0] >> 7));
		memmap_auto_1k(0, MMPPU(0x3800), (m080.chr[1] >> 7));
		memmap_auto_1k(0, MMPPU(0x3C00), (m080.chr[1] >> 7));
	} else if (m080.mirroring & 0x01) {
		mirroring_V(0);
	} else {
		mirroring_H(0);
	}
}
