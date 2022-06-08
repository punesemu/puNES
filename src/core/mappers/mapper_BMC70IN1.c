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
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

struct _bmc70in1 {
	WORD reg[3];
} bmc70in1;
struct _bmc70in1tmp {
	BYTE reset;
} bmc70in1tmp;

void map_init_BMC70IN1(void) {
	EXTCL_CPU_WR_MEM(BMC70IN1);
	EXTCL_CPU_RD_MEM(BMC70IN1);
	EXTCL_SAVE_MAPPER(BMC70IN1);
	mapper.internal_struct[0] = (BYTE *)&bmc70in1;
	mapper.internal_struct_size[0] = sizeof(bmc70in1);

	memset(&bmc70in1, 0x00, sizeof(bmc70in1));

	map_chr_bank_1k_reset();

	if (info.mapper.submapper == DEFAULT) {
		// 800-in-1 [p1][U][!].unf
		info.mapper.submapper = (prg_size() == (1024 * 512)) && (info.crc32.prg == 0x0BB4FD7A) ? BMC70IN1B : BMC70IN1;
	}

	if (info.reset == RESET) {
		bmc70in1tmp.reset = (bmc70in1tmp.reset + 1) & 0x0F;
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		if (info.crc32.prg == 0xF92EFDE7) { // 150-in-1.nes (mapper 390)
			bmc70in1tmp.reset = 0x0B;
		} else if (info.mapper.submapper == BMC70IN1) {
			bmc70in1tmp.reset = 0x0D;
		} else {
			bmc70in1tmp.reset = 0x06;
		}
	}

	info.mapper.extend_rd = TRUE;

	extcl_cpu_wr_mem_BMC70IN1(0x0000, 0);
}
void extcl_cpu_wr_mem_BMC70IN1(WORD address, BYTE value) {
	if (address & 0x4000) {
		bmc70in1.reg[0] = address & 0x30;
		bmc70in1.reg[1] = address & 0x07;
	} else {
		if (address & 0x20) {
			mirroring_H();
		} else {
			mirroring_V();
		}

		if (info.mapper.submapper == BMC70IN1B) {
			bmc70in1.reg[2] = (address & 0x03) << 3;
		} else {
			DBWORD bank;

			value = address & 0x07;
			control_bank(info.chr.rom.max.banks_8k)
			bank = value << 13;
			chr.bank_1k[0] = chr_pnt(bank);
			chr.bank_1k[1] = chr_pnt(bank | 0x0400);
			chr.bank_1k[2] = chr_pnt(bank | 0x0800);
			chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
			chr.bank_1k[4] = chr_pnt(bank | 0x1000);
			chr.bank_1k[5] = chr_pnt(bank | 0x1400);
			chr.bank_1k[6] = chr_pnt(bank | 0x1800);
			chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
		}
	}

	switch (bmc70in1.reg[0]) {
		case 0x00:
		case 0x10:
			value = bmc70in1.reg[2] | bmc70in1.reg[1];
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);

			value = bmc70in1.reg[2] | 0x07;
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, value);
			break;
		case 0x20:
			value = (bmc70in1.reg[2] | bmc70in1.reg[1]) >> 1;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			break;
		case 0x30:
			value = bmc70in1.reg[2] | bmc70in1.reg[1];
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k(2, 2, value);
			break;
	}
	map_prg_rom_8k_update();
}
BYTE extcl_cpu_rd_mem_BMC70IN1(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x8000) && (bmc70in1.reg[0] == 0x10)) {
		address = (address & 0xFFF0) | bmc70in1tmp.reset;
		return (prg_rom_rd(address));
	}
	return (openbus);
}
BYTE extcl_save_mapper_BMC70IN1(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, bmc70in1.reg);

	return (EXIT_OK);
}
