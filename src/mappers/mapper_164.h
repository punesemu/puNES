/*
 * mapper_164.h
 *
 *  Created on: 06/dic/2012
 *      Author: fhorse
 */

#ifndef MAPPER_164_H_
#define MAPPER_164_H_

#include "common.h"

struct _m164 {
	BYTE prg;
} m164;

void map_init_164(void);
void extcl_cpu_wr_mem_164(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_164(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_164(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_164_H_ */
