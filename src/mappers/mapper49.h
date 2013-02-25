/*
 * mapper49.h
 *
 *  Created on: 21/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER49_H_
#define MAPPER49_H_

#include "common.h"

struct _m49 {
	BYTE reg;
	WORD prgmap[4];
	WORD chrmap[8];
} m49;

void map_init_49(void);
void extcl_cpu_wr_mem_49(WORD address, BYTE value);
BYTE extcl_save_mapper_49(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER49_H_ */
