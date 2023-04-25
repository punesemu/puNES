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

void prg_swap_205(WORD address, WORD value);
void chr_swap_205(WORD address, WORD value);

INLINE static void tmp_fix_205(BYTE max, BYTE index, const BYTE *ds);

struct _m205 {
	BYTE reg;
} m205;
struct _m205tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const BYTE *dipswitch;
} m205tmp;

void map_init_205(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(205);
	EXTCL_SAVE_MAPPER(205);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m205;
	mapper.internal_struct_size[0] = sizeof(m205);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m205, 0x00, sizeof(m205));

	init_MMC3();
	MMC3_prg_swap = prg_swap_205;
	MMC3_chr_swap = chr_swap_205;

	if (info.reset == RESET) {
		if (m205tmp.ds_used) {
			m205tmp.index = (m205tmp.index + 1) % m205tmp.max;
		}
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		memset(&m205tmp, 0x00, sizeof(m205tmp));
		if (info.crc32.prg == 0x5A22BA9F) { // 15-in-1 [p1].nes
			static const BYTE ds[] = { 1, 0 };

			tmp_fix_205(LENGTH(ds), 0, &ds[0]);
		}
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_205(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (cpu.prg_ram_wr_active) {
			m205.reg = value;
			if ((value & 0x01) && m205tmp.ds_used && (m205tmp.dipswitch[m205tmp.index])) {
				m205.reg |= 2;
			}
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_save_mapper_205(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m205.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_205(WORD address, WORD value) {
	WORD base = (m205.reg & 0x03) << 4;
	WORD mask = 0x1F >> ((m205.reg & 0x04) ? 1 : ((m205.reg & 0x02) >> 1));

	value = (base & ~mask) | (value & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_swap_205(WORD address, WORD value) {
	WORD base = (m205.reg & 0x03) << 7;
	WORD mask = 0xFF >> ((m205.reg & 0x04) ? 1 : ((m205.reg & 0x02) >> 1));

	value = (base & ~mask) | (value & mask);
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}

INLINE static void tmp_fix_205(BYTE max, BYTE index, const BYTE *ds) {
	m205tmp.ds_used = TRUE;
	m205tmp.max = max;
	m205tmp.index = index;
	m205tmp.dipswitch = ds;
}
