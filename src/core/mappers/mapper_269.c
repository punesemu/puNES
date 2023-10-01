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

void prg_swap_mmc3_269(WORD address, WORD value);
void chr_swap_mmc3_269(WORD address, WORD value);

struct _m269 {
	BYTE write;
	BYTE reg;
	WORD prg[2];
	WORD chr[2];
} m269;

void map_init_269(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
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

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	}

	memset(&m269, 0x00, sizeof(m269));

	init_MMC3(info.reset);
	MMC3_prg_swap = prg_swap_mmc3_269;
	MMC3_chr_swap = chr_swap_mmc3_269;

	m269.prg[1] = 0x3F;
	m269.chr[1] = 0xFF;

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (!chrrom_size()) {
			vram_set_ram_size(0, 0);
			vram_set_nvram_size(0, 0);

			chrrom_set_size(prgrom_size());
			chrrom_init();

			for (size_t i = 0; i < prgrom_size(); i++) {
				BYTE value = prgrom_byte(i);

				value = ((value & 0x01) << 6) | ((value & 0x02) << 3) |
					((value & 0x04) << 0) | ((value & 0x08) >> 3) |
					((value & 0x10) >> 3) | ((value & 0x20) >> 2) |
					((value & 0x40) >> 1) | ((value & 0x80) << 0);
				chrrom_byte(i) = value;
			}
		}
	}

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	nes[0].irqA12.delay = 1;
}
void extcl_cpu_wr_mem_269(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		if (!(m269.reg & 0x80)) {
			switch (m269.write++ & 0x03) {
				case 0:
					m269.chr[0] = (m269.chr[0] & 0xFF00) | value;
					MMC3_chr_fix();
					return;
				case 1:
					m269.prg[0] = (m269.prg[0] & 0xFF00) | value;
					MMC3_prg_fix();
					return;
				case 2:
					m269.chr[0] = (m269.chr[0] & 0xF0FF) | ((value & 0xF0) << 4);
					m269.chr[1] = 0xFF >> (~value & 0x0F);
					MMC3_chr_fix();
					return;
				case 3:
					m269.chr[0] = (m269.chr[0] & 0xEFFF) | ((value & 0x40) << 6);
					m269.prg[0] = (m269.prg[0] & 0xFEFF) | ((value & 0x40) << 2);
					m269.prg[1] = ~value & 0x3F;
					m269.reg = value;
					MMC3_prg_fix();
					MMC3_chr_fix();
					return;
			}
		}
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_save_mapper_269(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m269.write);
	save_slot_ele(mode, slot, m269.reg);
	save_slot_ele(mode, slot, m269.prg);
	save_slot_ele(mode, slot, m269.chr);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}

void prg_swap_mmc3_269(WORD address, WORD value) {
	prg_swap_MMC3_base(address, (m269.prg[0] | (value & m269.prg[1])));
}
void chr_swap_mmc3_269(WORD address, WORD value) {
	chr_swap_MMC3_base(address, (m269.chr[0] | (value & m269.chr[1])));
}
