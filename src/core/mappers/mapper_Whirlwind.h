/*
 * mapper_Whirlwind.h
 *
 *  Created on: 15/set/2011
 *      Author: fhorse
 */

#ifndef MAPPER_WHIRLWIND_H_
#define MAPPER_WHIRLWIND_H_

#include "common.h"

struct _whirlwind {
	uint32_t prg_ram;
} whirlwind;

void map_init_Whirlwind(void);
void extcl_cpu_wr_mem_Whirlwind(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_Whirlwind(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_Whirlwind(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_WHIRLWIND_H_ */
