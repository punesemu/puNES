/*
 * mapper205.h
 *
 *  Created on: 20/mar/2012
 *      Author: fhorse
 */

#ifndef MAPPER205_H_
#define MAPPER205_H_

#include "common.h"

struct _m205 {
	BYTE reg;
	WORD prg_map[4];
	WORD chr_map[8];
} m205;

void map_init_205(void);
void extcl_cpu_wr_mem_205(WORD address, BYTE value);
BYTE extcl_save_mapper_205(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER205_H_ */
