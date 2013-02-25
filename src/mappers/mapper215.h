/*
 * mapper215.h
 *
 *  Created on: 20/mar/2012
 *      Author: fhorse
 */

#ifndef MAPPER215_H_
#define MAPPER215_H_

#include "common.h"

enum { M215MK3E = 2 };

struct _m215 {
	BYTE reg[4];
	WORD prg8kBank[4];
} m215;

void map_init_215(void);
void extcl_cpu_wr_mem_215(WORD address, BYTE value);
BYTE extcl_save_mapper_215(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER215_H_ */
