/*
 * mapper_UxROM.c
 *
 *  Created on: 18/mag/2010
 *      Author: fhorse
 */

#include "mem_map.h"
#include "mappers.h"

WORD prg_rom_16k_max;

void map_init_UxROM(BYTE model) {
	prg_rom_16k_max = info.prg_rom_16k_count - 1;

	switch (model) {
		case UNLROM:
			EXTCL_CPU_WR_MEM(UnlROM);
			break;
		case UXROM:
			EXTCL_CPU_WR_MEM(UxROM);
			break;
		case UNL1XROM:
			EXTCL_CPU_WR_MEM(Unl1xROM);
			break;
		case UNROM180:
			EXTCL_CPU_WR_MEM(UNROM_180);
			break;
	}
}

void extcl_cpu_wr_mem_UxROM(WORD address, BYTE value) {
	/* bus conflict */
	value &= prg_rom_rd(address);

	control_bank_with_AND(0x0F, prg_rom_16k_max)
	map_prg_rom_8k(2, 0, value);
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_Unl1xROM(WORD address, BYTE value) {
	/* bus conflict */
	value = (value & prg_rom_rd(address)) >> 2;

	control_bank_with_AND(0x0F, prg_rom_16k_max)
	map_prg_rom_8k(2, 0, value);
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_UNROM_180(WORD address, BYTE value) {
	control_bank(prg_rom_16k_max)
	map_prg_rom_8k(2, 2, value);
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_UnlROM(WORD address, BYTE value) {
	control_bank_with_AND(0x0F, prg_rom_16k_max)
	map_prg_rom_8k(2, 0, value);
	map_prg_rom_8k_update();
}
