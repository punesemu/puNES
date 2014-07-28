/*
 * mapper_VRC3.h
 *
 *  Created on: 11/set/2011
 *      Author: fhorse
 */

#ifndef MAPPER_VRC3_H_
#define MAPPER_VRC3_H_

#include "common.h"

struct _vrc3 {
	BYTE enabled;
	WORD reload;
	BYTE mode;
	BYTE acknowledge;
	WORD mask;
	WORD count;
} vrc3;

void map_init_VRC3(void);
void extcl_cpu_wr_mem_VRC3(WORD address, BYTE value);
BYTE extcl_save_mapper_VRC3(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_VRC3(void);

#endif /* MAPPER_VRC3_H_ */
