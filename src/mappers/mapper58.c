/*
 * mapper58.c
 *
 *  Created on: 22/apr/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

WORD prgRom16kMax, chrRom8kMax;

void map_init_58(void) {
	prgRom16kMax = info.prg_rom_16k_count - 1;
	chrRom8kMax = info.chr_rom_8k_count - 1;

	EXTCL_CPU_WR_MEM(58);

	if (info.reset >= HARD) {
		extcl_cpu_wr_mem_58(0x8000, 0x00);
	}
}
void extcl_cpu_wr_mem_58(WORD address, BYTE value) {
	DBWORD bank;
	BYTE tmp = address & 0x07;

	if (address & 0x0080) {
		mirroring_H();
	} else  {
		mirroring_V();
	}

	value = tmp & ~((~address >> 6) & 0x01);
	control_bank(prgRom16kMax)
	map_prg_rom_8k(2, 0, value);

	value = tmp | ((~address >> 6) & 0x01);
	control_bank(prgRom16kMax)
	map_prg_rom_8k(2, 2, value);

	map_prg_rom_8k_update();

	value = (address >> 3) & 0x07;
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
