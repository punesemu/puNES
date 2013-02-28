/*
 * mapper45.h
 *
 *  Created on: 19/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER45_H_
#define MAPPER45_H_

#include "common.h"

struct _m45 {
	BYTE reg[4];
	BYTE index;
	WORD prg_map[4];
	WORD chr_map[8];
} m45;

void map_init_45(void);
void extcl_cpu_wr_mem_45(WORD address, BYTE value);
BYTE extcl_save_mapper_45(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER45_H_ */
