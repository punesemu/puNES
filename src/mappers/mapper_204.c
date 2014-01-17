/*
 * mapper_204.c
 *
 *  Created on: 21/mar/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

WORD prg_rom_16k_max, chr_rom_8k_max;

void map_init_204(void) {
	prg_rom_16k_max = info.prg.rom.banks_16k - 1;
	chr_rom_8k_max = (info.chr.rom.banks_4k >> 1) - 1;

	EXTCL_CPU_WR_MEM(204);

	if (info.reset >= HARD) {
		extcl_cpu_wr_mem_204(0x8000, 0);
	}
}
void extcl_cpu_wr_mem_204(WORD address, BYTE value) {
	WORD save = (address >> 1) & (address >> 2) & 0x0001;
	DBWORD bank;

	value = address & ~save;
	control_bank(prg_rom_16k_max)
	map_prg_rom_8k(2, 0, value);
	value = address | save;
	control_bank(prg_rom_16k_max)
	map_prg_rom_8k(2, 2, value);
	map_prg_rom_8k_update();

	value = address & ~save;

	if (value > chr_rom_8k_max) {
		value &= (chr_rom_8k_max + 1);
		if (value > chr_rom_8k_max) {
			value &= chr_rom_8k_max;
		}
	}
	bank = value << 13;
	chr.bank_1k[0] = &chr.data[bank];
	chr.bank_1k[1] = &chr.data[bank | 0x0400];
	chr.bank_1k[2] = &chr.data[bank | 0x0800];
	chr.bank_1k[3] = &chr.data[bank | 0x0C00];
	chr.bank_1k[4] = &chr.data[bank | 0x1000];
	chr.bank_1k[5] = &chr.data[bank | 0x1400];
	chr.bank_1k[6] = &chr.data[bank | 0x1800];
	chr.bank_1k[7] = &chr.data[bank | 0x1C00];

	if (address & 0x0010) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
