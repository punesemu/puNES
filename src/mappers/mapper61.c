/*
 * mapper61.c
 *
 *  Created on: 22/apr/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

WORD prgRom32kMax, prgRom16kMax;

void map_init_61(void) {
	prgRom32kMax = (info.prg_rom_16k_count >> 1) - 1;
	prgRom16kMax = info.prg_rom_16k_count - 1;

	EXTCL_CPU_WR_MEM(61);

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}
}
void extcl_cpu_wr_mem_61(WORD address, BYTE value) {
	if (address & 0x0080) {
		mirroring_H();
	} else  {
		mirroring_V();
	}

	if (address & 0x0010) {
		value = ((address << 1) & 0x1E) | ((address >> 5) & 0x01);
		control_bank(prgRom16kMax)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k(2, 2, value);
	} else {
		value = address & 0x0F;
		control_bank(prgRom32kMax)
		map_prg_rom_8k(4, 0, value);
	}
	map_prg_rom_8k_update();
}
