/*
 * mapper_183.h
 *
 *  Created on: 9/ott/2011
 *      Author: fhorse
 */

#ifndef MAPPER_183_H_
#define MAPPER_183_H_

#include "common.h"

struct _m183 {
	BYTE enabled;
	BYTE prescaler;
	BYTE count;
	BYTE delay;
	BYTE chr_rom_bank[8];
} m183;

void map_init_183(void);
void extcl_cpu_wr_mem_183(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_183(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_183(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_183(void);

#endif /* MAPPER_183_H_ */
