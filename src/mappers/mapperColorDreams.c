/*
 * mapperColorDreams.c
 *
 *  Created on: 11/lug/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax, chrRom8kMax;

void map_init_ColorDreams(void) {
	prgRom32kMax = (info.prg_rom_16k_count >> 1) - 1;
	chrRom8kMax = info.chr_rom_8k_count - 1;

	EXTCL_CPU_WR_MEM(ColorDreams);

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}
}
void extcl_cpu_wr_mem_ColorDreams(WORD address, BYTE value) {
	BYTE save = value;
	DBWORD chrBank;

	/* bus conflict */
	if (info.mapper_type != CDNOCONFLCT) {
		save = value &= prg_rom_rd(address);
	}

	control_bank_with_AND(0x03, prgRom32kMax)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	value = (save & 0xF0) >> 4;
	control_bank(chrRom8kMax)
	chrBank = value << 13;
	chr.bank_1k[0] = &chr.data[chrBank];
	chr.bank_1k[1] = &chr.data[chrBank | 0x0400];
	chr.bank_1k[2] = &chr.data[chrBank | 0x0800];
	chr.bank_1k[3] = &chr.data[chrBank | 0x0C00];
	chr.bank_1k[4] = &chr.data[chrBank | 0x1000];
	chr.bank_1k[5] = &chr.data[chrBank | 0x1400];
	chr.bank_1k[6] = &chr.data[chrBank | 0x1800];
	chr.bank_1k[7] = &chr.data[chrBank | 0x1C00];
}
