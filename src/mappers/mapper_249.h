/*
 * mapper_249.h
 *
 *  Created on: 09/dic/2012
 *      Author: fhorse
 */

#ifndef MAPPER_249_H_
#define MAPPER_249_H_

#include "common.h"

struct _m249 {
	BYTE reg;
	WORD prg_map[4];
	WORD chr_map[8];
} m249;

void map_init_249(void);
void extcl_cpu_wr_mem_249(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_249(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_249(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_249_H_ */
