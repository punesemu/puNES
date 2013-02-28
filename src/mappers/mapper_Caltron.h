/*
 * mapper_Caltron.h
 *
 *  Created on: 16/set/2011
 *      Author: fhorse
 */

#ifndef MAPPER_CALTRON_H_
#define MAPPER_CALTRON_H_

#include "common.h"

struct _caltron {
	BYTE reg;
} caltron;

void map_init_Caltron(void);
void extcl_cpu_wr_mem_Caltron(WORD address, BYTE value);
BYTE extcl_save_mapper_Caltron(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_CALTRON_H_ */
