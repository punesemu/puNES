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

void prg_swap_mmc3_262(WORD address, WORD value);
void chr_swap_mmc3_262(WORD address, WORD value);
void mirroring_fix_mmc3_262(void);

INLINE static void tmp_fix_262(BYTE max, BYTE index, const BYTE *ds);

struct _m262 {
	BYTE reg;
} m262;
struct _m262tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const BYTE *dipswitch;
} m262tmp;

void map_init_262(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(262);
	EXTCL_CPU_RD_MEM(262);
	EXTCL_SAVE_MAPPER(262);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m262;
	mapper.internal_struct_size[0] = sizeof(m262);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	if (info.reset >= HARD) {
		memset(&m262, 0x00, sizeof(m262));
	}

	init_MMC3();
	MMC3_prg_swap = prg_swap_mmc3_262;
	MMC3_chr_swap = chr_swap_mmc3_262;
	MMC3_mirroring_fix = mirroring_fix_mmc3_262;

	if (info.reset == RESET) {
		if (m262tmp.ds_used) {
			m262tmp.index = (m262tmp.index + 1) % m262tmp.max;
		}
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memset(&m262tmp, 0x00, sizeof(m262tmp));

		{
			static BYTE ds[] = { 0x00 };

			tmp_fix_262(LENGTH(ds), 0, &ds[0]);
		}
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_262(WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x4FFF)) {
		if (address & 0x0100) {
			m262.reg = value;
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_262(WORD address, BYTE openbus) {
	if ((address >= 0x4000) && (address <= 0x4FFF)) {
		if (address & 0x0100) {
			return (m262tmp.dipswitch[m262tmp.index]);
		}
	}
	return (openbus);
}
BYTE extcl_save_mapper_262(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m262.reg);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_262(WORD address, WORD value) {
	prg_swap_MMC3_base(address, (value & 0x3F));
}
void chr_swap_mmc3_262(WORD address, WORD value) {
	const BYTE slot = (address >> 10) & 0x07;

	if ((m262.reg & 0x40) && vram_size()) {
		memmap_vram_1k(MMPPU(address), slot);
	} else {
		static const BYTE shift[] = { 3, 2, 0, 1 };
		const BYTE index = (slot & 0x06) >> 1;
		WORD base = 0;

		base = ((m262.reg >> shift[index]) & 0x01) * 0x100;
		chr_swap_MMC3_base(address, (base | value));
	}
}
void mirroring_fix_mmc3_262(void) {
	mirroring_FSCR();
}

INLINE static void tmp_fix_262(BYTE max, BYTE index, const BYTE *ds) {
	m262tmp.ds_used = TRUE;
	m262tmp.max = max;
	m262tmp.index = index;
	m262tmp.dipswitch = ds;
}
