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

struct _bmc831128c {
	BYTE reg;
} bmc831128c;
struct _bmc831128ctmp {
	BYTE *prg_6000;
} bmc831128ctmp;

void map_init_831128C(void) {
	map_init_VRC7(VRC7B);
	extcl_apu_tick = NULL;

	EXTCL_CPU_WR_MEM(831128C);
	EXTCL_CPU_RD_MEM(831128C);
	EXTCL_SAVE_MAPPER(831128C);
	mapper.internal_struct[1] = (BYTE *)&bmc831128c;
	mapper.internal_struct_size[1] = sizeof(bmc831128c);

	memset(&bmc831128c, 0x00, sizeof(bmc831128c));

	{
		BYTE value = info.prg.rom.max.banks_16k >> 1;

		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}

	bmc831128ctmp.prg_6000 = prg_pnt(bmc831128c.reg << 13);
}
void extcl_cpu_wr_mem_831128C(UNUSED(WORD address), UNUSED(BYTE value)) {
	WORD bank = address & 0xF000;

	// ignore writes except $Axxx and $Cxxx
	if ((bank != 0xA000) && (bank != 0xC000)) {
		return;
	}

	{
		BYTE reg = address & 0x0F, outer = (address & 0x4000) >> 10;

		switch (reg) {
			case 0x0:
			case 0x1:
			case 0x2:
			case 0x3:
			case 0x4:
			case 0x5:
			case 0x6:
			case 0x7:
				bank = (outer << 4) | value;
				_control_bank(bank, info.chr.rom.max.banks_1k)
				chr.bank_1k[reg] = chr_pnt(bank << 10);
				break;
			case 0x8:
				bmc831128c.reg = value;
				bank = outer + (value & (outer | 0x0F));
				_control_bank(bank, info.prg.rom.max.banks_8k)
				bmc831128ctmp.prg_6000 = prg_pnt(bank << 13);
				break;
			case 0x9:
			case 0xA:
			// Register $B is not functional,
			//case 0xB:
				bank = outer + (value & (outer | 0x0F));
				_control_bank(bank, info.prg.rom.max.banks_8k)
				map_prg_rom_8k(1, reg - 9, bank);
				break;
			case 0xC:
				extcl_cpu_wr_mem_VRC7(0xE000, value);
				break;
			case 0xD:
				extcl_cpu_wr_mem_VRC7(0xF000, value);
				break;
			case 0xE:
				extcl_cpu_wr_mem_VRC7(0xF008, value);
				break;
			case 0xF:
				extcl_cpu_wr_mem_VRC7(0xE008, value);
				break;
		}

		bank = outer + (0xFE & (outer | 0x0F));
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 2, bank);

		bank = outer + (0xFF & (outer | 0x0F));
		_control_bank(bank, info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 3, bank);

		map_prg_rom_8k_update();
	}
}
BYTE extcl_cpu_rd_mem_831128C(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (bmc831128c.reg == 1) {
			return (openbus);
		}
		return (bmc831128ctmp.prg_6000[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_831128C(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, bmc831128c.reg);
	extcl_save_mapper_VRC7(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		bmc831128ctmp.prg_6000 = prg_pnt(bmc831128c.reg << 13);
	}

	return (EXIT_OK);
}
