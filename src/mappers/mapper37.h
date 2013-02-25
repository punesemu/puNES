/*
 * mapper37.h
 *
 *  Created on: 25/mar/2012
 *      Author: fhorse
 */

#ifndef MAPPER37_H_
#define MAPPER37_H_

#include "common.h"

struct _m37 {
	BYTE reg;
	WORD prgmap[4];
	WORD chrmap[8];
} m37;

void map_init_37(void);
void extcl_cpu_wr_mem_37(WORD address, BYTE value);
BYTE extcl_save_mapper_37(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER37_H_ */
