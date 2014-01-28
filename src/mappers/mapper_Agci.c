/*
 * mapper_Agci.c
 *
 *  Created on: 12/lug/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

WORD chr_rom_8k_max;

void map_init_Agci(void) {
	chr_rom_8k_max = info.chr.rom.banks_8k - 1;

	EXTCL_CPU_WR_MEM(Agci);

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}
}
void extcl_cpu_wr_mem_Agci(WORD address, BYTE value) {
	BYTE bus_conflict, save;
	DBWORD bank;

	/* bus conflict */
	bus_conflict = prg_rom_rd(address);
	value = (value & bus_conflict) | (bus_conflict & 0x01);
	save = value;

	control_bank_with_AND(0x03, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	value = (save & 0xF0) >> 4;
	control_bank(chr_rom_8k_max)
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
