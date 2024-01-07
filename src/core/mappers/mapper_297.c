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

void prg_swap_mmc1_297(WORD address, WORD value);
void chr_swap_mmc1_297(WORD address, WORD value);

INLINE static void prg_fix_297(void);
INLINE static void chr_fix_297(void);
INLINE static void mirroring_fix_297(void);

struct _m297 {
	BYTE reg[2];
} m297;

void map_init_297(void) {
	EXTCL_AFTER_MAPPER_INIT(297);
	EXTCL_CPU_WR_MEM(297);
	EXTCL_SAVE_MAPPER(297);
	map_internal_struct_init((BYTE *)&m297, sizeof(m297));
	map_internal_struct_init((BYTE *)&mmc1, sizeof(mmc1));

	if (info.reset >= HARD) {
		memset(&m297, 0x00, sizeof(m297));
	}

	init_MMC1(MMC1A, info.reset);
	MMC1_prg_swap = prg_swap_mmc1_297;
	MMC1_chr_swap = chr_swap_mmc1_297;

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_297(void) {
	prg_fix_297();
	chr_fix_297();
	mirroring_fix_297();
}
void extcl_cpu_wr_mem_297(BYTE nidx, WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x4000:
		case 0x5000:
			if (address & 0x0100) {
				m297.reg[0] = value;
				prg_fix_297();
				chr_fix_297();
				mirroring_fix_297();
			}
			break;
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			if (m297.reg[0] & 0x01) {
				extcl_cpu_wr_mem_MMC1(nidx, address, value);
			} else {
				m297.reg[1] = value;
				prg_fix_297();
				chr_fix_297();
				mirroring_fix_297();
			}
			break;
	}
}
BYTE extcl_save_mapper_297(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m297.reg);
	return (extcl_save_mapper_MMC1(mode, slot, fp));
}

void prg_swap_mmc1_297(WORD address, WORD value) {
	prg_swap_MMC1_base(address, (0x08 | (value & 0x07)));
}
void chr_swap_mmc1_297(WORD address, WORD value) {
	chr_swap_MMC1_base(address, (0x20 | (value & 0x1F)));
}

INLINE static void prg_fix_297(void) {
	if (m297.reg[0] & 0x01) {
		MMC1_prg_fix();
	} else {
		memmap_auto_16k(0, MMCPU(0x8000), (((m297.reg[0] & 0x02) << 1) | ((m297.reg[1] & 0x30) >> 4)));
		memmap_auto_16k(0, MMCPU(0xC000), (((m297.reg[0] & 0x02) << 1) | 0x03));
	}
}
INLINE static void chr_fix_297(void) {
	if (m297.reg[0] & 0x01) {
		MMC1_chr_fix();
	} else {
		memmap_auto_8k(0, MMPPU(0x0000), (m297.reg[1] & 0x0F));
	}
}
INLINE static void mirroring_fix_297(void) {
	if (m297.reg[0] & 0x01) {
		MMC1_mirroring_fix();
	} else {
		mirroring_V(0);
	}
}
