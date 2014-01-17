/*
 * mapper_AxROM.c
 *
 *  Created on: 01/mar/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

WORD prg_rom_32k_max;

void map_init_AxROM(void) {
	prg_rom_32k_max = (info.prg.rom.banks_16k >> 1) - 1;

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
	if (info.mapper.from_db == AMROM) {
		value &= prg_rom_rd(address);
	}

	if (value & 0x10) {
		mirroring_SCR0();
	} else {
		mirroring_SCR1();
	}

	control_bank_with_AND(0x0F, prg_rom_32k_max)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}
