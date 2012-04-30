/*
 * mapper120.h
 *
 *  Created on: 29/set/2011
 *      Author: fhorse
 */

#ifndef MAPPER120_H_
#define MAPPER120_H_

#include "common.h"

struct _m120 {
	BYTE *prgRamRd;
} m120;

void mapInit_120(void);
void extclCpuWrMem_120(WORD address, BYTE value);
BYTE extclCpuRdMem_120(WORD address, BYTE openbus, BYTE before);
BYTE extclSaveMapper_120(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER120_H_ */
