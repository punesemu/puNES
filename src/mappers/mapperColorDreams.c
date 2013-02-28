/*
 * mapperColorDreams.c
 *
 *  Created on: 11/lug/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

WORD prg_rom_32k_max, chr_rom_8k_max;

void map_init_ColorDreams(void) {
	prg_rom_32k_max = (info.prg_rom_16k_count >> 1) - 1;
	chr_rom_8k_max = info.chr_rom_8k_count - 1;

	EXTCL_CPU_WR_MEM(ColorDreams);

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}
}
void extcl_cpu_wr_mem_ColorDreams(WORD address, BYTE value) {
	BYTE save = value;
	DBWORD chr_bank;

	/* bus conflict */
	if (info.mapper_type != CD_NO_CONFLCT) {
		save = value &= prg_rom_rd(address);
	}

	control_bank_with_AND(0x03, prg_rom_32k_max)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	value = (save & 0xF0) >> 4;
	control_bank(chr_rom_8k_max)
	chr_bank = value << 13;
	chr.bank_1k[0] = &chr.data[chr_bank];
	chr.bank_1k[1] = &chr.data[chr_bank | 0x0400];
	chr.bank_1k[2] = &chr.data[chr_bank | 0x0800];
	chr.bank_1k[3] = &chr.data[chr_bank | 0x0C00];
	chr.bank_1k[4] = &chr.data[chr_bank | 0x1000];
	chr.bank_1k[5] = &chr.data[chr_bank | 0x1400];
	chr.bank_1k[6] = &chr.data[chr_bank | 0x1800];
	chr.bank_1k[7] = &chr.data[chr_bank | 0x1C00];
}
