/*
 * mapper_28.h
 *
 *  Created on: 29/dic/2013
 *      Author: fhorse
 */

#ifndef MAPPER_28_H_
#define MAPPER_28_H_

#include "common.h"

struct _m28 {
	BYTE index;
	BYTE mirroring;
	BYTE prg[3];
} m28;

void map_init_28(void);
void extcl_cpu_wr_mem_28(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_28(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_28(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_28_H_ */
