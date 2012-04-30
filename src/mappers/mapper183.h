/*
 * mapper183.h
 *
 *  Created on: 9/ott/2011
 *      Author: fhorse
 */

#ifndef MAPPER183_H_
#define MAPPER183_H_

#include "common.h"

struct _m183 {
	BYTE enabled;
	BYTE prescaler;
	BYTE count;
	BYTE delay;
	BYTE chrRomBank[8];
} m183;

void mapInit_183(void);
void extclCpuWrMem_183(WORD address, BYTE value);
BYTE extclCpuRdMem_183(WORD address, BYTE openbus, BYTE before);
BYTE extclSaveMapper_183(BYTE mode, BYTE slot, FILE *fp);
void extclCPUEveryCycle_183(void);

#endif /* MAPPER183_H_ */
