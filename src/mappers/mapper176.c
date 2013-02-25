/*
 * mapper176.c
 *
 *  Created on: 6/ott/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax, chrRom8kMax;

void map_init_176(void) {
	prgRom32kMax = (info.prg_rom_16k_count >> 1) - 1;
	chrRom8kMax = info.chr_rom_8k_count - 1;

	EXTCL_CPU_WR_MEM(176);

	info.mapper_extend_wr = TRUE;
}
void extcl_cpu_wr_mem_176(WORD address, BYTE value) {
	switch (address) {
		case 0x5FF1:
			value >>= 1;
			control_bank(prgRom32kMax)
			map_prg_rom_8k(4, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x5FF2: {
			DBWORD bank;

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
			return;
		}
	}
}
