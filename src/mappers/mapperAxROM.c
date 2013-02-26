/*
 * mapperAxROM.c
 *
 *  Created on: 01/mar/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

WORD prgRom32kMax;

void map_init_AxROM(void) {
	prgRom32kMax = (info.prg_rom_16k_count >> 1) - 1;

	EXTCL_CPU_WR_MEM(AxROM);

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}

	if (info.id == BBCARUNL) {
		mirroring_SCR0();
	}
}
void extcl_cpu_wr_mem_AxROM(WORD address, BYTE value) {
	/* bus conflict */
	if (info.mapper_type == AMROM) {
		value &= prg_rom_rd(address);
	}

	if (value & 0x10) {
		mirroring_SCR0();
	} else {
		mirroring_SCR1();
	}

	control_bank_with_AND(0x0F, prgRom32kMax)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}
