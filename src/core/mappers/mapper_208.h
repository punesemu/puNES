/*
 * mapper_208.h
 *
 *  Created on: 01/ott/2011
 *      Author: fhorse
 */

#ifndef MAPPER_208_H_
#define MAPPER_208_H_

#include "common.h"

struct _m208 {
	BYTE ctrl;
	BYTE reg[4];
} m208;

void map_init_208(void);
void extcl_cpu_wr_mem_208(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_208(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_208(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_208_H_ */
