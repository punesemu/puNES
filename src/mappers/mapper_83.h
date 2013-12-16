/*
 * mapper_83.h
 *
 *  Created on: 15/dic/2013
 *      Author: fhorse
 */

#ifndef MAPPER_83_H_
#define MAPPER_83_H_

#include "common.h"

enum { MAP83_REG0 = 2, MAP83_DGP };

struct _m83 {
	BYTE is2kbank;
	BYTE isnot2kbank;
	BYTE mode;
	BYTE bank;
	BYTE dip;
	BYTE low[4];
	BYTE reg[11];

	struct _m83_irq {
		BYTE active;
		WORD count;
	} irq;
} m83;

void map_init_83(void);
void extcl_cpu_wr_mem_83(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_83(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_83(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_83(void);

#endif /* MAPPER_83_H_ */
