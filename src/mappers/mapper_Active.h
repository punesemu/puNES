/*
 * mapper_Active.h
 *
 *  Created on: 02/feb/2012
 *      Author: fhorse
 */

#ifndef MAPPER_ACTIVE_H_
#define MAPPER_ACTIVE_H_

#include "common.h"

struct _active {
	BYTE openbus;
	BYTE prg_ram[4];
} active;

void map_init_Active(void);
void extcl_cpu_wr_mem_Active(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_Active(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_Active(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_ACTIVE_H_ */
