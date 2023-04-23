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

void prg_swap_334(WORD address, WORD value);
void chr_swap_334(WORD address, WORD value);

INLINE static void tmp_fix_334(BYTE max, BYTE index, const BYTE *ds);

struct _m334 {
	BYTE reg;
} m334;
struct _m334tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const BYTE *dipswitch;
} m334tmp;

void map_init_334(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(334);
	EXTCL_CPU_RD_MEM(334);
	EXTCL_SAVE_MAPPER(334);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m334;
	mapper.internal_struct_size[0] = sizeof(m334);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m334, 0x00, sizeof(m334));

	init_MMC3();
	MMC3_prg_swap = prg_swap_334;
	MMC3_chr_swap = chr_swap_334;

	if (info.reset == RESET) {
		if (m334tmp.ds_used) {
			m334tmp.index = (m334tmp.index + 1) % m334tmp.max;
		}
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		memset(&m334tmp, 0x00, sizeof(m334tmp));
		if (info.crc32.prg == 0x7EC6DF24) { // 5-in-1 (Multi)[Unknown][1993 Copyrights].nes
			static const BYTE ds[] = { 1, 0 };

			tmp_fix_334(LENGTH(ds), 0, &ds[0]);
		}
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_334(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (cpu.prg_ram_wr_active && !(address & 0x0001)) {
			m334.reg = value;
			MMC3_prg_fix(mmc3.bank_to_update);
			MMC3_chr_fix(mmc3.bank_to_update);
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_cpu_rd_mem_334(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (m334tmp.ds_used && cpu.prg_ram_rd_active && (address & 0x0002)) {
			return ((openbus & 0xFE) | (m334tmp.dipswitch[m334tmp.index] & 0x01));
		}
	}
	return (openbus);
}
BYTE extcl_save_mapper_334(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m334.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_334(WORD address, WORD value) {
	const WORD slot = (address >> 13) & 0x03;

	value = ((m334.reg & ~1) << 1) | slot;
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, slot, value);
	map_prg_rom_8k_update();
}
void chr_swap_334(WORD address, WORD value) {
	WORD base = 0;
	WORD mask = 0xFF;

	value = base | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}

INLINE static void tmp_fix_334(BYTE max, BYTE index, const BYTE *ds) {
	m334tmp.ds_used = TRUE;
	m334tmp.max = max;
	m334tmp.index = index;
	m334tmp.dipswitch = ds;
}
