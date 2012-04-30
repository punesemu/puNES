/*
 * mapper222.h
 *
 *  Created on: 24/mar/2011
 *      Author: fhorse
 */

#ifndef MAPPER222_H_
#define MAPPER222_H_

#include "common.h"

struct _m222 {
	BYTE count;
	BYTE delay;
} m222;

void mapInit_222(void);
void extclCpuWrMem_222(WORD address, BYTE value);
BYTE extclSaveMapper_222(BYTE mode, BYTE slot, FILE *fp);
void extclIrqA12Clock_222(void);
void extclCPUEveryCycle_222(void);

#endif /* MAPPER222_H_ */
