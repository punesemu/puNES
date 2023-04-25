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
#include "info.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

void chr_swap_Rexdbz(WORD address, WORD value);

INLINE static void tmp_fix_Rexdbz(BYTE max, BYTE index, const BYTE *ds);

struct _rexdbz {
	BYTE reg;
} rexdbz;
struct _rexdbztmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const BYTE *dipswitch;
} rexdbztmp;

void map_init_Rexdbz(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(Rexdbz);
	EXTCL_CPU_RD_MEM(Rexdbz);
	EXTCL_SAVE_MAPPER(Rexdbz);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&rexdbz;
	mapper.internal_struct_size[0] = sizeof(rexdbz);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&rexdbz, 0x00, sizeof(rexdbz));

	init_MMC3();
	MMC3_chr_swap = chr_swap_Rexdbz;

	if (info.reset == RESET) {
		if (rexdbztmp.ds_used) {
			rexdbztmp.index = (rexdbztmp.index + 1) % rexdbztmp.max;
		}
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		static const BYTE ds[] = { 0, 1 };

		memset(&rexdbztmp, 0x00, sizeof(rexdbztmp));
		tmp_fix_Rexdbz(LENGTH(ds), 0, &ds[0]);
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_Rexdbz(WORD address, BYTE value) {
	if ((address >= 0x4000) && (address < 0x4FFF)) {
		if (address & 0x0100) {
			rexdbz.reg = value;
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_Rexdbz(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x4000) && (address < 0x4FFF)) {
		if (address & 0x0100) {
			return (rexdbztmp.ds_used ? rexdbztmp.dipswitch[rexdbztmp.index] : openbus);
		}
	}
	return (openbus);
}
BYTE extcl_save_mapper_Rexdbz(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, rexdbz.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void chr_swap_Rexdbz(WORD address, WORD value) {
	const BYTE slot = address >> 10;
	WORD base = (rexdbz.reg << ((slot >= 4) ? 4 : 8)) & 0x0100;

	value = base | value;
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[slot] = chr_pnt(value << 10);
}

INLINE static void tmp_fix_Rexdbz(BYTE max, BYTE index, const BYTE *ds) {
	rexdbztmp.ds_used = TRUE;
	rexdbztmp.max = max;
	rexdbztmp.index = index;
	rexdbztmp.dipswitch = ds;
}