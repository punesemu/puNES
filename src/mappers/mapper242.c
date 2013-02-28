/*
 * mapper242.c
 *
 *  Created on: 24/apr/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

WORD prg_rom_32k_max;

void map_init_242(void) {
	prg_rom_32k_max = (info.prg_rom_16k_count >> 1) - 1;

	EXTCL_CPU_WR_MEM(242);

	map_prg_rom_8k(4, 0, 0);
}
void extcl_cpu_wr_mem_242(WORD address, BYTE value) {
	if (address & 0x0002) {
		mirroring_H();
	} else  {
		mirroring_V();
	}

	value = (address & 0x0078) >> 3;
	control_bank(prg_rom_32k_max)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}
