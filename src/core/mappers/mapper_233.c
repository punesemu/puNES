/*
 * mapper_233.c
 *
 *  Created on: 10/feb/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "info.h"
#include "mem_map.h"

void map_init_233(void) {
	EXTCL_CPU_WR_MEM(233);

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}
}
void extcl_cpu_wr_mem_233(WORD address, BYTE value) {
	BYTE save = value;

	value &= 0x1F;

	if (save & 0x20) {
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k(2, 2, value);
	} else {
		value >>= 1;
		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}
	map_prg_rom_8k_update();

	switch ((save & 0xC0) >> 6) {
		case 0:
			mirroring_SCR0x3_SCR1x1();
			break;
		case 1:
			mirroring_V();
			break;
		case 2:
			mirroring_H();
			break;
		case 3:
			mirroring_SCR1();
			break;
	}
}
