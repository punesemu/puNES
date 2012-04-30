/*
 * mapper0.c
 *
 *  Created on: 11/mag/2010
 *      Author: fhorse
 */

#include <stdio.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"

void mapInit_0(void) {
	EXTCLCPUWRMEM(0);
	EXTCLCPURDMEM(0);
}
void extclCpuWrMem_0(WORD address, BYTE value) {
#ifdef DEBUG
	fprintf(stderr, "Attempt to write %02X to %04X\n", value, address);
#endif
}
BYTE extclCpuRdMem_0(WORD address, BYTE openbus, BYTE before) {
	if (address < 0x6000) {
		return (before);
	}
	return (openbus);
}
