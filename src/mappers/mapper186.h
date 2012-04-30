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
void extclCpuWrMem_186(WORD address, BYTE value);
BYTE extclCpuRdMem_186(WORD address, BYTE openbus, BYTE before);
BYTE extclSaveMapper_186(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER186_H_ */
