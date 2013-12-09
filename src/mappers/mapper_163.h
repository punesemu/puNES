/*
 * mapper_163.h
 *
 *  Created on: 06/dic/2012
 *      Author: fhorse
 */

#ifndef MAPPER_163_H_
#define MAPPER_163_H_

#include "common.h"

struct _m163 {
	BYTE prg;
	BYTE chr;
	BYTE reg;
	BYTE security;
	BYTE trigger;
	BYTE chr_mode;
} m163;

void map_init_163(void);
void extcl_cpu_wr_mem_163(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_163(WORD address, BYTE openbus, BYTE before);
void extcl_ppu_update_screen_y_163(void);
BYTE extcl_save_mapper_163(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_163_H_ */
