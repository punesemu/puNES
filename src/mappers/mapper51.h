/*
 * mapper51.h
 *
 *  Created on: 21/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER51_H_
#define MAPPER51_H_

#include "common.h"

struct _m51 {
	BYTE mode;
	WORD bank;
	BYTE prg_6000;
} m51;

void map_init_51(void);
void extcl_cpu_wr_mem_51(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_51(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_51(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER51_H_ */
