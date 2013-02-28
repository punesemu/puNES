/*
 * mapper241.c
 *
 *  Created on: 24/mar/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"
#include "cpu.h"

WORD prg_rom_32k_max;

void map_init_241(void) {
	prg_rom_32k_max = (info.prg_rom_16k_count >> 1) - 1;

	EXTCL_CPU_WR_MEM(241);
	EXTCL_CPU_RD_MEM(241);

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
	}
}
void extcl_cpu_wr_mem_241(WORD address, BYTE value) {
	control_bank(prg_rom_32k_max)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}
BYTE extcl_cpu_rd_mem_241(WORD address, BYTE openbus, BYTE before) {
	if ((address >= 0x4020) && (address < 0x6000)) {
		return (0x50);
	}

	return (openbus);
}
