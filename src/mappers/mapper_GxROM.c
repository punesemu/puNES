/*
 * mapper_GxROM.c
 *
 *  Created on: 16/lug/2011
 *      Author: fhorse
 */

#include "mem_map.h"
#include "mappers.h"

void map_init_GxROM(void) {
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
	control_bank_with_AND(0x03, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	value = save;
	control_bank_with_AND(0x03, info.chr.rom.max.banks_8k)
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
