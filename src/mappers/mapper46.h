/*
 * mapper46.h
 *
 *  Created on: 20/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER46_H_
#define MAPPER46_H_

#include "common.h"

struct _m46 {
	BYTE prg;
	BYTE chr;
} m46;

void map_init_46(void);
void extcl_cpu_wr_mem_46(WORD address, BYTE value);
BYTE extcl_save_mapper_46(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER46_H_ */
