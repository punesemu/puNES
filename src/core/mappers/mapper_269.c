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

void prg_swap_mmc3_269(WORD address, WORD value);
void chr_swap_mmc3_269(WORD address, WORD value);

struct _m269 {
	BYTE write;
	BYTE reg[4];
} m269;

void map_init_269(void) {
	EXTCL_AFTER_MAPPER_INIT(269);
	EXTCL_CPU_WR_MEM(269);
	EXTCL_SAVE_MAPPER(269);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m269;
	mapper.internal_struct_size[0] = sizeof(m269);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m269, 0x00, sizeof(m269));

	init_MMC3();
	MMC3_prg_swap = prg_swap_mmc3_269;
	MMC3_chr_swap = chr_swap_mmc3_269;

	m269.reg[2] = 0x0F;

	if (mapper.write_vram) {
		info.chr.rom.banks_8k = info.prg.rom.banks_8k;
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_269(void) {
	extcl_after_mapper_init_MMC3();

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (mapper.write_vram) {
			size_t i;

			for (i = 0; i < prg_size(); i++) {
				BYTE value = prg_rom()[i];

				value = ((value & 0x01) << 6) | ((value & 0x02) << 3) |
					((value & 0x04) << 0) | ((value & 0x08) >> 3) |
					((value & 0x10) >> 3) | ((value & 0x20) >> 2) |
					((value & 0x40) >> 1) | ((value & 0x80) << 0);
				chr_rom()[i] = value;
			}
		}
	}
}
void extcl_cpu_wr_mem_269(WORD address, BYTE value) {
	if (address == 0x5000) {
		if (!(m269.reg[3] & 0x80)) {
			m269.reg[m269.write++ & 0x03] = value;
			MMC3_prg_fix();
			MMC3_chr_fix();
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
	}
}
BYTE extcl_save_mapper_269(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m269.write);
	save_slot_ele(mode, slot, m269.reg);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

void prg_swap_mmc3_269(WORD address, WORD value) {
	WORD base = ((m269.reg[3] & 0x40) << 2) | m269.reg[1];
	WORD mask = ~m269.reg[3] & 0x3F;

	prg_swap_MMC3_base(address, (base | (value & mask)));
}
void chr_swap_mmc3_269(WORD address, WORD value) {
	WORD base = ((m269.reg[3] & 0x40) << 6) | ((m269.reg[2] & 0xF0) << 4) | m269.reg[0];
	WORD mask = 0xFF >> (~m269.reg[2] & 0x0F);

	chr_swap_MMC3_base(address, (base | (value & mask)));
}
