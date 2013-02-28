/*
 * mapper120.h
 *
 *  Created on: 29/set/2011
 *      Author: fhorse
 */

#ifndef MAPPER120_H_
#define MAPPER120_H_

#include "common.h"

struct _m120 {
	BYTE *prg_ram_rd;
} m120;

void map_init_120(void);
void extcl_cpu_wr_mem_120(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_120(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_120(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER120_H_ */
