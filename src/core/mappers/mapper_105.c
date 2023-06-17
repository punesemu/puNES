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
#include "cpu.h"
#include "save_slot.h"

void prg_fix_mmc1_105(void);
void prg_swap_mmc1_105(WORD address, WORD value);
void chr_swap_mmc1_105(WORD address, WORD value);

INLINE static void tmp_fix_105(BYTE max, BYTE index, const BYTE *ds);

struct _m105 {
	uint32_t count;
} m105;
struct _m105tmp {
	uint32_t counter_must_reach;
	// dipswitch
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const BYTE *dipswitch;
} m105tmp;

void map_init_105(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC1);
	EXTCL_CPU_WR_MEM(MMC1);
	EXTCL_SAVE_MAPPER(105);
	EXTCL_CPU_EVERY_CYCLE(105);
	mapper.internal_struct[0] = (BYTE *)&m105;
	mapper.internal_struct_size[0] = sizeof(m105);
	mapper.internal_struct[1] = (BYTE *)&mmc1;
	mapper.internal_struct_size[1] = sizeof(mmc1);

	memset(&m105, 0x00, sizeof(m105));
	memset(&m105tmp, 0x00, sizeof(m105tmp));

	init_MMC1(MMC1B, HARD);
	MMC1_prg_fix = prg_fix_mmc1_105;
	MMC1_prg_swap = prg_swap_mmc1_105;
	MMC1_chr_swap = chr_swap_mmc1_105;

	if (info.reset == RESET) {
		if (m105tmp.ds_used) {
			m105tmp.index = (m105tmp.index + 1) % m105tmp.max;
		}
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if ((info.crc32.prg == 0x0B0E128F) || // Nintendo World Championships 1990 (U) [!].nes
			(info.crc32.prg == 0x00CA6F07)) {
			static BYTE ds[] = {
				0, 1,  2,  3,  4,  5,  6,  7,
				8, 9, 10, 11, 12, 13, 14, 15
			};

			tmp_fix_105(LENGTH(ds), 0, &ds[0]);
		}
		m105tmp.counter_must_reach = m105tmp.dipswitch ? (m105tmp.dipswitch[m105tmp.index] << 25) | 0x20000000 : 0;
	}
}
BYTE extcl_save_mapper_105(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m105.count);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_105(void) {
	if (mmc1.reg[1] & 0x10) {
		m105.count = 0;
		irq.high &= ~EXT_IRQ;
	} else if (++m105.count == m105tmp.counter_must_reach) {
		irq.high |= EXT_IRQ;
	}
}

void prg_fix_mmc1_105(void) {
	if (mmc1.reg[1] & 0x08) {
		prg_fix_MMC1_base();
		return;
	}
	memmap_auto_32k(MMCPU(0x8000), ((mmc1.reg[1] & 0x06) >> 1));
}
void prg_swap_mmc1_105(WORD address, WORD value) {
	prg_swap_MMC1_base(address, (0x08 | (value & 0x07)));
}
void chr_swap_mmc1_105(WORD address, WORD value) {
	chr_swap_MMC1_base(address, (value & 0x01));
}

INLINE static void tmp_fix_105(BYTE max, BYTE index, const BYTE *ds) {
	m105tmp.ds_used = TRUE;
	m105tmp.max = max;
	m105tmp.index = index;
	m105tmp.dipswitch = ds;
}
