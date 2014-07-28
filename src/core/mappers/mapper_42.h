/*
 * mapper_42.h
 *
 *  Created on: 02/gen/2014
 *      Author: fhorse
 */

#ifndef MAPPER_42_H_
#define MAPPER_42_H_

#include "common.h"

struct _m42 {
	WORD rom_map_to;
	BYTE *prg_8k_6000;
	struct _m42_irq {
		BYTE active;
		uint32_t count;
	} irq;
} m42;

void map_init_42(void);
void extcl_cpu_wr_mem_42(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_42(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_42(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_42(void);

#endif /* MAPPER_42_H_ */
