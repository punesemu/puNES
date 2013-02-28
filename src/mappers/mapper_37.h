/*
 * mapper_37.h
 *
 *  Created on: 25/mar/2012
 *      Author: fhorse
 */

#ifndef MAPPER_37_H_
#define MAPPER_37_H_

#include "common.h"

struct _m37 {
	BYTE reg;
	WORD prg_map[4];
	WORD chr_map[8];
} m37;

void map_init_37(void);
void extcl_cpu_wr_mem_37(WORD address, BYTE value);
BYTE extcl_save_mapper_37(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_37_H_ */
