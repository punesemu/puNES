/*
 * mapper_114.h
 *
 *  Created on: 8/ott/2011
 *      Author: fhorse
 */

#ifndef MAPPER_114_H_
#define MAPPER_114_H_

#include "common.h"

struct _m114 {
	BYTE prg_rom_switch;
	BYTE mmc3_ctrl_change;
	WORD prg_rom_bank[4];
} m114;

void map_init_114(void);
void extcl_cpu_wr_mem_114(WORD address, BYTE value);
BYTE extcl_save_mapper_114(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_114_H_ */
