/*
 * mapper_121.h
 *
 *  Created on: 01/ott/2011
 *      Author: fhorse
 */

#ifndef MAPPER_121_H_
#define MAPPER_121_H_

#include "common.h"

struct _m121 {
	WORD bck[2];
	BYTE reg[3];
} m121;

void map_init_121(void);
void extcl_cpu_wr_mem_121(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_121(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_121(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_121_H_ */
