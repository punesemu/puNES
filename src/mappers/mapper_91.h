/*
 * mapper_91.h
 *
 *  Created on: 03/gen/2014
 *      Author: fhorse
 */

#ifndef MAPPER_91_H_
#define MAPPER_91_H_

#include "common.h"

struct _m91 {
	struct _m91_irq {
		BYTE active;
		BYTE count;
	} irq;
} m91;

void map_init_91(void);
void extcl_cpu_wr_mem_91(WORD address, BYTE value);
BYTE extcl_save_mapper_91(BYTE mode, BYTE slot, FILE *fp);
void extcl_ppu_256_to_319_91(void);

#endif /* MAPPER_91_H_ */
