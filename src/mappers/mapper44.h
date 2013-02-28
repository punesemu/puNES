/*
 * mapper44.h
 *
 *  Created on: 19/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER44_H_
#define MAPPER44_H_

#include "common.h"

struct _m44 {
	BYTE reg;
	WORD prg_map[4];
	WORD chr_map[8];
} m44;

void map_init_44(void);
void extcl_cpu_wr_mem_44(WORD address, BYTE value);
BYTE extcl_save_mapper_44(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER44_H_ */
