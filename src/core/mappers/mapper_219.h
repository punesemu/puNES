/*
 * mapper_219.h
 *
 *  Created on: 20/mar/2012
 *      Author: fhorse
 */

#ifndef MAPPER_219_H_
#define MAPPER_219_H_

#include "common.h"

struct _m219 {
	BYTE reg[3];
} m219;

void map_init_219(void);
void extcl_cpu_wr_mem_219(WORD address, BYTE value);
BYTE extcl_save_mapper_219(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_219_H_ */
