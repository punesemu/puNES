/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

static void INLINE bmc64in1norepeat_update_chr(void);

#define bmc64in1norepeat_prg_8k()\
	((bmc64in1norepeat.reg[1] & 0x1F) << 1) | ((bmc64in1norepeat.reg[1] >> 6) & 0x01);

void map_init_BMC64IN1NOREPEAT(void) {
	EXTCL_CPU_WR_MEM(BMC64IN1NOREPEAT);
	EXTCL_SAVE_MAPPER(BMC64IN1NOREPEAT);
	mapper.internal_struct[0] = (BYTE *) &bmc64in1norepeat;
	mapper.internal_struct_size[0] = sizeof(bmc64in1norepeat);

	memset(&bmc64in1norepeat, 0x00, sizeof(bmc64in1norepeat));
	bmc64in1norepeat.reg[0] = 0x80;
	bmc64in1norepeat.reg[1] = 0x43;

	extcl_cpu_wr_mem_BMC64IN1NOREPEAT(0x5003, bmc64in1norepeat.reg[3]);

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_BMC64IN1NOREPEAT(WORD address, BYTE value) {
	if (address >= 0x8000) {
		bmc64in1norepeat.reg[3] = value;
	} else if ((address >= 0x5000) && (address <= 0x5003)) {
		bmc64in1norepeat.reg[address & 0x03] = value;
	} else {
		return;
	}

	if (bmc64in1norepeat.reg[0] & 0x80) {
		if (bmc64in1norepeat.reg[1] & 0x80) {
			value = bmc64in1norepeat.reg[1] & 0x1F;
			control_bank(info.prg.rom[0].max.banks_32k)
			map_prg_rom_8k(4, 0, value);
		} else {
			value = bmc64in1norepeat_prg_8k();
			control_bank(info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k(2, 2, value);
		}
	} else {
		value = bmc64in1norepeat_prg_8k();
		control_bank(info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 2, value);
	}
	map_prg_rom_8k_update();

	bmc64in1norepeat_update_chr();

	if (bmc64in1norepeat.reg[0] & 0x20) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
BYTE extcl_save_mapper_BMC64IN1NOREPEAT(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, bmc64in1norepeat.reg);

	return (EXIT_OK);
}

static void INLINE bmc64in1norepeat_update_chr(void) {
	WORD value = (bmc64in1norepeat.reg[2] << 2) | ((bmc64in1norepeat.reg[0] >> 1) & 0x03);
	DBWORD bank;

	control_bank(info.chr.rom[0].max.banks_8k)
	bank = value << 13;
	chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
	chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
	chr.bank_1k[2] = chr_chip_byte_pnt(0, bank | 0x0800);
	chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0C00);
	chr.bank_1k[4] = chr_chip_byte_pnt(0, bank | 0x1000);
	chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x1400);
	chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x1800);
	chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x1C00);
}
