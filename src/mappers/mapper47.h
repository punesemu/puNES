/*
 * mapper47.h
 *
 *  Created on: 21/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER47_H_
#define MAPPER47_H_

#include "common.h"

struct _m47 {
	BYTE reg;
	WORD prg_map[4];
	WORD chr_map[8];
} m47;

void map_init_47(void);
void extcl_cpu_wr_mem_47(WORD address, BYTE value);
BYTE extcl_save_mapper_47(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER47_H_ */
