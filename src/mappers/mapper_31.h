/*
 * mapper_31.h
 *
 *  Created on: 29/apr/2014
 *      Author: fhorse
 */

#ifndef MAPPER_31_H_
#define MAPPER_31_H_

#include "common.h"

struct _m31 {
	WORD regs[8];
	BYTE *rom_4k[8];
} m31;

void map_init_31(void);
void extcl_cpu_wr_mem_31(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_31(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_31(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_31_H_ */
