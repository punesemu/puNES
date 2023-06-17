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
#include "irqA12.h"
#include "save_slot.h"

void prg_swap_mmc3_012(WORD address, WORD value);
void chr_swap_mmc3_012(WORD address, WORD value);

INLINE static void tmp_fix_012(BYTE max, BYTE index, const WORD *ds);

struct _m012 {
	BYTE reg;
} m012;
struct _m012tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const WORD *dipswitch;
} m012tmp;

void map_init_012(void) {
	if (info.mapper.submapper == 1) {
		map_init_006();
	} else {
		EXTCL_AFTER_MAPPER_INIT(MMC3);
		EXTCL_CPU_WR_MEM(012);
		EXTCL_CPU_RD_MEM(012);
		EXTCL_SAVE_MAPPER(012);
		EXTCL_CPU_EVERY_CYCLE(MMC3);
		EXTCL_PPU_000_TO_34X(MMC3);
		EXTCL_PPU_000_TO_255(MMC3);
		EXTCL_PPU_256_TO_319(MMC3);
		EXTCL_PPU_320_TO_34X(MMC3);
		EXTCL_UPDATE_R2006(MMC3);
		mapper.internal_struct[0] = (BYTE *)&m012;
		mapper.internal_struct_size[0] = sizeof(m012);
		mapper.internal_struct[1] = (BYTE *)&mmc3;
		mapper.internal_struct_size[1] = sizeof(mmc3);

		memset(&irqA12, 0x00, sizeof(irqA12));

		if (info.reset >= HARD) {
			memset(&m012, 0x00, sizeof(m012));
		}

		init_MMC3();
		MMC3_prg_swap = prg_swap_mmc3_012;
		MMC3_chr_swap = chr_swap_mmc3_012;

		if (info.reset == RESET) {
			if (m012tmp.ds_used) {
				m012tmp.index = (m012tmp.index + 1) % m012tmp.max;
			}
		} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
			memset(&m012tmp, 0x00, sizeof(m012tmp));

			{
				//static const BYTE ds[] = { 0, 1 };
				static const WORD ds[] = { 0 };

				tmp_fix_012(LENGTH(ds), 0, &ds[0]);
			}
		}

		info.mapper.extend_wr = TRUE;

		irqA12.present = TRUE;
		irqA12_delay = 1;
	}
}
void extcl_cpu_wr_mem_012(WORD address, BYTE value) {
	if ((address >= 0x4000) && (address < 0x4FFF)) {
		if (address & 0x0100) {
			m012.reg = value;
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_012(WORD address, UNUSED(BYTE openbus)) {
	if ((address >= 0x4000) && (address < 0x4FFF)) {
		return (address & 0x0100 ? m012tmp.dipswitch[m012tmp.index] : wram_rd(address));
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_012(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m012.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_012(WORD address, WORD value) {
	prg_swap_MMC3_base(address, (value & 0x3F));
}
void chr_swap_mmc3_012(WORD address, WORD value) {
	WORD base = (m012.reg << (((address >> 10) >= 4) ? 4 : 8)) & 0x0100;

	chr_swap_MMC3_base(address, (base | (value & 0xFF)));
}

INLINE static void tmp_fix_012(BYTE max, BYTE index, const WORD *ds) {
	m012tmp.ds_used = TRUE;
	m012tmp.max = max;
	m012tmp.index = index;
	m012tmp.dipswitch = ds;
}