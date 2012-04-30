/*
 * mapper50.h
 *
 *  Created on: 21/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER50_H_
#define MAPPER50_H_

#include "common.h"

struct _m50 {
	BYTE enabled;
	WORD count;
	BYTE delay;
} m50;

void mapInit_50(void);
void extclCpuWrMem_50(WORD address, BYTE value);
BYTE extclCpuRdMem_50(WORD address, BYTE openbus, BYTE before);
BYTE extclSaveMapper_50(BYTE mode, BYTE slot, FILE *fp);
void extclCPUEveryCycle_50(void);

#endif /* MAPPER50_H_ */
