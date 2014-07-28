/*
 * mapper_105.h
 *
 *  Created on: 08/gen/2014
 *      Author: fhorse
 */

#ifndef MAPPER_105_H_
#define MAPPER_105_H_

#include "common.h"

struct m105 {
	BYTE reg;
	BYTE pos;
	BYTE ctrl;
	BYTE reset;
	struct _prg_m105 {
		BYTE mode;
		BYTE locked;
		BYTE upper;
		BYTE reg[2];
	} prg;
	struct _irq_m105 {
		BYTE reg;
		uint32_t count;
	} irq;
} m105;

void map_init_105(void);
void extcl_cpu_wr_mem_105(WORD address, BYTE value);
BYTE extcl_save_mapper_105(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_105(void);

#endif /* MAPPER_105_H_ */
