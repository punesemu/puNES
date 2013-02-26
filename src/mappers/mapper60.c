/*
 * mapper60.c
 *
 *  Created on: 22/apr/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"
#include "save_slot.h"

WORD prgRom16kMax, chrRom8kMax;

void map_init_60(void) {
	prgRom16kMax = info.prg_rom_16k_count - 1;
	chrRom8kMax = info.chr_rom_8k_count - 1;

	EXTCL_CPU_WR_MEM(60);
	EXTCL_SAVE_MAPPER(60);
	mapper.internal_struct[0] = (BYTE *) &m60;
	mapper.internal_struct_size[0] = sizeof(m60);

	if (info.reset >= HARD) {
		m60.index = 0;
	} else {
		BYTE tmp = m60.index;
		m60.index = ++tmp & 0x03;
	}

	{
		BYTE value;
		DBWORD bank;

		value = m60.index;
		control_bank(prgRom16kMax)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k(2, 2, value);

		value = m60.index;
		control_bank(chrRom8kMax)
		bank = value << 13;
		chr.bank_1k[0] = &chr.data[bank];
		chr.bank_1k[1] = &chr.data[bank | 0x0400];
		chr.bank_1k[2] = &chr.data[bank | 0x0800];
		chr.bank_1k[3] = &chr.data[bank | 0x0C00];
		chr.bank_1k[4] = &chr.data[bank | 0x1000];
		chr.bank_1k[5] = &chr.data[bank | 0x1400];
		chr.bank_1k[6] = &chr.data[bank | 0x1800];
		chr.bank_1k[7] = &chr.data[bank | 0x1C00];
	}
}
void extcl_cpu_wr_mem_60(WORD address, BYTE value) {
	return;
}
BYTE extcl_save_mapper_60(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m60.index);

	return (EXIT_OK);
}

void map_init_60_vt5201(void) {
	prgRom16kMax = info.prg_rom_16k_count - 1;
	chrRom8kMax = info.chr_rom_8k_count - 1;

	EXTCL_CPU_WR_MEM(60_vt5201);

	if (info.reset >= HARD) {
		extcl_cpu_wr_mem_60_vt5201(0x8000, 0);
	}
}
void extcl_cpu_wr_mem_60_vt5201(WORD address, BYTE value) {
	DBWORD bank;

	if (address & 0x0008) {
		mirroring_H();
	} else  {
		mirroring_V();
	}

	value = (address >> 4) & ~((~address >> 7) & 0x01);
	control_bank(prgRom16kMax)
	map_prg_rom_8k(2, 0, value);

	value = (address >> 4) | ((~address >> 7) & 0x01);
	control_bank(prgRom16kMax)
	map_prg_rom_8k(2, 2, value);

	map_prg_rom_8k_update();

	value = address & 0xFF;
	control_bank(chrRom8kMax)
	bank = value << 13;
	chr.bank_1k[0] = &chr.data[bank];
	chr.bank_1k[1] = &chr.data[bank | 0x0400];
	chr.bank_1k[2] = &chr.data[bank | 0x0800];
	chr.bank_1k[3] = &chr.data[bank | 0x0C00];
	chr.bank_1k[4] = &chr.data[bank | 0x1000];
	chr.bank_1k[5] = &chr.data[bank | 0x1400];
	chr.bank_1k[6] = &chr.data[bank | 0x1800];
	chr.bank_1k[7] = &chr.data[bank | 0x1C00];
}
