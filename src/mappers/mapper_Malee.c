/*
 * mapper_Malee.c
 *
 *  Created on: 11/lug/2014
 *      Author: fhorse
 */

#include <stdio.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"

void map_init_malee(void) {
	EXTCL_CPU_RD_MEM(malee);

	map_prg_rom_8k(4, 0, 0);

	mirroring_V();
}
BYTE extcl_cpu_rd_mem_malee(WORD address, BYTE openbus, BYTE before) {
	if ((address >= 0x6000) && (address <= 0x67FF)) {
		return (map_prg_chip_rd_byte(1, openbus, address, 0x07FF));
	}
	return (openbus);
}
