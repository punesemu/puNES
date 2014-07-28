/*
 * mapper_215.h
 *
 *  Created on: 20/mar/2012
 *      Author: fhorse
 */

#ifndef MAPPER_215_H_
#define MAPPER_215_H_

#include "common.h"

enum { M215_MK3E };

struct _m215 {
	BYTE reg[4];
	WORD prg_8k_bank[4];
} m215;

void map_init_215(void);
void extcl_cpu_wr_mem_215(WORD address, BYTE value);
BYTE extcl_save_mapper_215(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_215_H_ */
