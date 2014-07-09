/*
 * mapper_AxROM.c
 *
 *  Created on: 01/mar/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

void map_init_AxROM(void) {
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
	if (info.mapper.submapper == AMROM) {
		value &= prg_rom_rd(address);
	}

	if (value & 0x10) {
		mirroring_SCR0();
	} else {
		mirroring_SCR1();
	}

	control_bank_with_AND(0x0F, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}
