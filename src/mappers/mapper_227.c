/*
 * mapper_227.c
 *
 *  Created on: 09/feb/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

WORD prg_rom_16k_max;

void map_init_227(void) {
	prg_rom_16k_max = info.prg.rom.banks_16k - 1;

	EXTCL_CPU_WR_MEM(227);

	extcl_cpu_wr_mem_227(0x8000, 0x00);
}
void extcl_cpu_wr_mem_227(WORD address, BYTE value) {
	BYTE bank = ((address >> 4) & 0x10) | ((address >> 3) & 0x0F);

	if (address & 0x0001) {
		value = bank;
		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	} else {
		value = (bank << 1) | ((address >> 2) & 0x01);
		control_bank(prg_rom_16k_max)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k(2, 2, value);
	}

	if (!(address & 0x0080)) {
		value = ((address & 0x0200) ? 0x07 : 0x00) | ((bank << 1) & 0x38);
		control_bank(prg_rom_16k_max)
		map_prg_rom_8k(2, 2, value);
	}
	map_prg_rom_8k_update();

	if (address & 0x0002) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
