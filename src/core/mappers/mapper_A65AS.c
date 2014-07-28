/*
 * mapper_A65AS.c
 *
 *  Created on: 20/mag/2014
 *      Author: fhorse
 */

#include "mappers.h"
#include "info.h"
#include "mem_map.h"

void map_init_A65AS(void) {
	EXTCL_CPU_WR_MEM(A65AS);
}
void extcl_cpu_wr_mem_A65AS(WORD address, BYTE value) {
	if (value & 0x80) {
		if (value & 0x20) {
			mirroring_SCR1();
		} else {
			mirroring_SCR0();
		}
	} else {
		if (value & 0x08) {
			mirroring_H();
		} else {
			mirroring_V();
		}
	}

	if (value & 0x40) {
		value >>= 1;
		control_bank_with_AND(0x0F, info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	} else {
		BYTE save = value;

		value = ((save & 0x30) >> 1) | (save & 0x07);
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		value = ((save & 0x30) >> 1) | 0x07;
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 2, value);
	}
	map_prg_rom_8k_update();
}
