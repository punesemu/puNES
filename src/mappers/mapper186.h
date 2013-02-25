/*
 * mapper186.h
 *
 *  Created on: 10/ott/2011
 *      Author: fhorse
 */

#ifndef MAPPER186_H_
#define MAPPER186_H_

#include "common.h"

struct _m186 {
	BYTE *prgRamBank2;
} m186;

void mapInit_186(void);
void extcl_cpu_wr_mem_186(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_186(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_186(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER186_H_ */
