/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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
#include "save_slot.h"

void map_init_MMC2and4(void) {
	EXTCL_CPU_WR_MEM(MMC2and4);
	EXTCL_SAVE_MAPPER(MMC2and4);
	EXTCL_AFTER_RD_CHR(MMC2and4);
	mapper.internal_struct[0] = (BYTE *) &mmc2and4;
	mapper.internal_struct_size[0] = sizeof(mmc2and4);

	if (info.reset >= HARD) {
		memset(&mmc2and4, 0x00, sizeof(mmc2and4));
		mmc2and4.latch1 = 2;

		/* MMC2 */
		if (info.mapper.id == 9) {
			mapper.rom_map_to[1] = info.prg.rom[0].banks_8k - 3;
		}
	}
}
void extcl_cpu_wr_mem_MMC2and4(WORD address, BYTE value) {
	DBWORD tmp;

	address &= 0xF000;

	switch (address) {
		case 0xA000:
			if (info.mapper.id == 9) {
				/* MMC2 */
				control_bank_with_AND(0x0F, info.prg.rom[0].max.banks_8k)
				map_prg_rom_8k(1, 0, value);
			} else {
				/* MMC4 */
				control_bank_with_AND(0x0F, info.prg.rom[0].max.banks_16k)
				map_prg_rom_8k(2, 0, value);
			}
			map_prg_rom_8k_update();
			return;
		case 0xB000:
			control_bank_with_AND(0x1F, info.chr.rom[0].max.banks_4k)
			mmc2and4.regs[0] = value;
			break;
		case 0xC000:
			control_bank_with_AND(0x1F, info.chr.rom[0].max.banks_4k)
			mmc2and4.regs[1] = value;
			break;
		case 0xD000:
			control_bank_with_AND(0x1F, info.chr.rom[0].max.banks_4k)
			mmc2and4.regs[2] = value;
			break;
		case 0xE000:
			control_bank_with_AND(0x1F, info.chr.rom[0].max.banks_4k)
			mmc2and4.regs[3] = value;
			break;
		case 0xF000:
			if (value & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			return;
	}
	tmp = mmc2and4.regs[mmc2and4.latch0] << 12;
	chr.bank_1k[0] = chr_chip_byte_pnt(0, tmp);
	chr.bank_1k[1] = chr_chip_byte_pnt(0, tmp | 0x0400);
	chr.bank_1k[2] = chr_chip_byte_pnt(0, tmp | 0x0800);
	chr.bank_1k[3] = chr_chip_byte_pnt(0, tmp | 0x0C00);
	tmp = mmc2and4.regs[mmc2and4.latch1] << 12;
	chr.bank_1k[4] = chr_chip_byte_pnt(0, tmp);
	chr.bank_1k[5] = chr_chip_byte_pnt(0, tmp | 0x0400);
	chr.bank_1k[6] = chr_chip_byte_pnt(0, tmp | 0x0800);
	chr.bank_1k[7] = chr_chip_byte_pnt(0, tmp | 0x0C00);
}
BYTE extcl_save_mapper_MMC2and4(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, mmc2and4.regs);
	save_slot_ele(mode, slot, mmc2and4.latch0);
	save_slot_ele(mode, slot, mmc2and4.latch1);

	return (EXIT_OK);
}
void extcl_after_rd_chr_MMC2and4(WORD address) {
	WORD bank, latch = address & 0xFFF0;
	DBWORD value;

	switch (latch) {
		case 0x0FD0:
			mmc2and4.latch0 = 0;
			value = mmc2and4.regs[mmc2and4.latch0];
			bank = 0;
			break;
		case 0x0FE0:
			mmc2and4.latch0 = 1;
			value = mmc2and4.regs[mmc2and4.latch0];
			bank = 0;
			break;
		case 0x1FD0:
			mmc2and4.latch1 = 2;
			value = mmc2and4.regs[mmc2and4.latch1];
			bank = 4;
			break;
		case 0x1FE0:
			mmc2and4.latch1 = 3;
			value = mmc2and4.regs[mmc2and4.latch1];
			bank = 4;
			break;
		default:
			return;
	}
	value <<= 12;
	chr.bank_1k[0 | bank] = chr_chip_byte_pnt(0, value);
	chr.bank_1k[1 | bank] = chr_chip_byte_pnt(0, value | 0x0400);
	chr.bank_1k[2 | bank] = chr_chip_byte_pnt(0, value | 0x0800);
	chr.bank_1k[3 | bank] = chr_chip_byte_pnt(0, value | 0x0C00);
}
