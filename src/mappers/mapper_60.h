/*
 * mapper_60.h
 *
 *  Created on: 22/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER_60_H_
#define MAPPER_60_H_

#include "common.h"

enum { MAP60 = 2, MAP60_VT5201 };

struct _m60 {
	BYTE index;
} m60;

void map_init_60(void);
void extcl_cpu_wr_mem_60(WORD address, BYTE value);
BYTE extcl_save_mapper_60(BYTE mode, BYTE slot, FILE *fp);

void map_init_60_vt5201(void);
void extcl_cpu_wr_mem_60_vt5201(WORD address, BYTE value);

#endif /* MAPPER_60_H_ */
