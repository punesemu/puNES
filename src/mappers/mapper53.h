/*
 * mapper53.h
 *
 *  Created on: 21/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER53_H_
#define MAPPER53_H_

#include "common.h"

struct _m53 {
	BYTE reg[2];
	BYTE prg_6000;
} m53;

void map_init_53(void);
void extcl_cpu_wr_mem_53(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_53(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_53(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER53_H_ */
