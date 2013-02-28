/*
 * mapper_235.h
 *
 *  Created on: 11/feb/2012
 *      Author: fhorse
 */

#ifndef MAPPER_235_H_
#define MAPPER_235_H_

#include "common.h"

struct _m235 {
	BYTE openbus;
} m235;

void map_init_235(void);
void extcl_cpu_wr_mem_235(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_235(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_235(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_235_H_ */
