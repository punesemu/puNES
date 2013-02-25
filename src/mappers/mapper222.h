/*
 * mapper222.h
 *
 *  Created on: 24/mar/2011
 *      Author: fhorse
 */

#ifndef MAPPER222_H_
#define MAPPER222_H_

#include "common.h"

struct _m222 {
	BYTE count;
	BYTE delay;
} m222;

void mapInit_222(void);
void extcl_cpu_wr_mem_222(WORD address, BYTE value);
BYTE extcl_save_mapper_222(BYTE mode, BYTE slot, FILE *fp);
void extcl_irq_A12_clock_222(void);
void extcl_cpu_every_cycle_222(void);

#endif /* MAPPER222_H_ */
