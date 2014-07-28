/*
 * mapper_57.h
 *
 *  Created on: 22/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER_57_H_
#define MAPPER_57_H_

#include "common.h"

struct _m57 {
	BYTE reg[2];
} m57;

void map_init_57(void);
void extcl_cpu_wr_mem_57(WORD address, BYTE value);
BYTE extcl_save_mapper_57(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_57_H_ */
