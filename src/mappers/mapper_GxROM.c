/*
 * mapper_GxROM.c
 *
 *  Created on: 16/lug/2011
 *      Author: fhorse
 */

#include "mem_map.h"
#include "mappers.h"

WORD prg_rom_32k_max, chr_rom_8k_max;

void map_init_GxROM(void) {
	prg_rom_32k_max = (info.prg.rom.banks_16k >> 1) - 1;
	chr_rom_8k_max = (info.chr.rom.banks_4k >> 1) - 1;

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}

	EXTCL_CPU_WR_MEM(GxROM);
}
void extcl_cpu_wr_mem_GxROM(WORD address, BYTE value) {
	/* bus conflict */
	BYTE save = value &= prg_rom_rd(address);
	DBWORD bank;

	value >>= 4;
	control_bank_with_AND(0x03, prg_rom_32k_max)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	value = save;
	control_bank_with_AND(0x03, chr_rom_8k_max)
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
