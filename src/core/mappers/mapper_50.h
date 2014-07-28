/*
 * mapper_50.h
 *
 *  Created on: 21/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER_50_H_
#define MAPPER_50_H_

#include "common.h"

struct _m50 {
	BYTE enabled;
	WORD count;
	BYTE delay;
} m50;

void map_init_50(void);
void extcl_cpu_wr_mem_50(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_50(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_50(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_50(void);

#endif /* MAPPER_50_H_ */
