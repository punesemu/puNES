/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_404(void);
INLINE static void chr_fix_404(void);

struct _m404 {
	BYTE reg;
} m404;

void map_init_404(void) {
	info.mapper.submapper = MAP404;
	map_init_MMC1();

	EXTCL_AFTER_MAPPER_INIT(404);
	EXTCL_CPU_WR_MEM(404);
	EXTCL_SAVE_MAPPER(404);
	mapper.internal_struct[0] = (BYTE *)&m404;
	mapper.internal_struct_size[0] = sizeof(m404);
	mapper.internal_struct[1] = (BYTE *)&mmc1;
	mapper.internal_struct_size[1] = sizeof(mmc1);

	memset(&m404, 0x00, sizeof(m404));

	mmc1.prg_mask = 0x0F;
	mmc1.prg_upper = 0;
	mmc1.chr_upper = 0;

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_404(void) {
	prg_fix_404();
	chr_fix_404();
}
void extcl_cpu_wr_mem_404(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x6000:
		case 0x7000:
			if (!(m404.reg & 0x80)) {
				m404.reg = value;
				mmc1.prg_mask = 0x0F >> ((m404.reg & 0x40) >> 6);
				mmc1.prg_upper = ((m404.reg & 0x0F) << 3) & ~mmc1.prg_mask;
				mmc1.chr_upper = ((m404.reg & 0x0F) << 5);
				prg_fix_404();
				chr_fix_404();
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
			extcl_cpu_wr_mem_MMC1(address, value);
			break;
	}
}
BYTE extcl_save_mapper_404(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m404.reg);
	extcl_save_mapper_MMC1(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void prg_fix_404(void) {
	WORD bank;

	bank = mmc1.prg_upper | (mmc1.prg0 & mmc1.prg_mask);
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, bank);

	bank = mmc1.prg_upper | mmc1.prg_mask;
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, bank);

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_404(void) {
	DBWORD bank;

	bank = mmc1.chr_upper & 0x1F;
	_control_bank(bank, info.chr.rom.max.banks_8k)
	bank <<= 13;
	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
}
