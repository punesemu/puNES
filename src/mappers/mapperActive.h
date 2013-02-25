/*
 * mapperActive.h
 *
 *  Created on: 02/feb/2012
 *      Author: fhorse
 */

#ifndef MAPPERACTIVE_H_
#define MAPPERACTIVE_H_

#include "common.h"

struct _active {
	BYTE openbus;
	BYTE prgRam[4];
} active;

void mapInit_Active(void);
void extcl_cpu_wr_mem_Active(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_Active(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_Active(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPERACTIVE_H_ */
