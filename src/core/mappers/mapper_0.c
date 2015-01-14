/*
 * mapper_0.c
 *
 *  Created on: 11/mag/2010
 *      Author: fhorse
 */

#include <string.h>
#include "mappers.h"
#include "mem_map.h"

void map_init_0(void) {
	EXTCL_CPU_WR_MEM(0);
	EXTCL_CPU_RD_MEM(0);
}
void extcl_cpu_wr_mem_0(WORD address, BYTE value) {
#if defined (DEBUG)
	//fprintf(stderr, "Attempt to write %02X to %04X\n", value, address);
#endif
}
BYTE extcl_cpu_rd_mem_0(WORD address, BYTE openbus, BYTE before) {
	if (address < 0x6000) {
		return (before);
	}
	return (openbus);
}
