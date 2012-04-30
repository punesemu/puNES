/*
 * mapperTengen.h
 *
 *  Created on: 17/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERTENGEN_H_
#define MAPPERTENGEN_H_

#include "common.h"

enum {
	TRAMBO = 3,
	T800037 = 4,
	NOCNTPLUS = 5
};

struct _tengenRambo {
	BYTE prgMode;
	BYTE chrMode;
	BYTE regIndex;
	BYTE chr[8];
	BYTE prg[4];
	BYTE irqMode;
	BYTE irqDelay;
	BYTE irqPrescaler;
} tRambo;

void mapInit_Tengen(BYTE model);

void extclCpuWrMem_Tengen_Rambo(WORD address, BYTE value);
BYTE extclSaveMapper_Tengen_Rambo(BYTE mode, BYTE slot, FILE *fp);
void extclPPU000to255_Tengen_Rambo(void);
void extclPPU256to319_Tengen_Rambo(void);
void extclPPU320to34x_Tengen_Rambo(void);
void extclIrqA12Clock_Tengen_Rambo(void);
void extclCPUEveryCycle_Tengen_Rambo(void);

#endif /* MAPPERTENGEN_H_ */
