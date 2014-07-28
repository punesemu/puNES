/*
 * mapper_222.h
 *
 *  Created on: 24/mar/2011
 *      Author: fhorse
 */

#ifndef MAPPER_222_H_
#define MAPPER_222_H_

#include "common.h"

struct _m222 {
	BYTE count;
	BYTE delay;
} m222;

void map_init_222(void);
void extcl_cpu_wr_mem_222(WORD address, BYTE value);
BYTE extcl_save_mapper_222(BYTE mode, BYTE slot, FILE *fp);
void extcl_irq_A12_clock_222(void);
void extcl_cpu_every_cycle_222(void);

#endif /* MAPPER_222_H_ */
