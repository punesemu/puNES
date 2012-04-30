/*
 * mapperFuturemedia.h
 *
 *  Created on: 28/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERFUTUREMEDIA_H_
#define MAPPERFUTUREMEDIA_H_

#include "common.h"

struct _futuremedia {
	BYTE delay;
} futuremedia;

void mapInit_Futuremedia(void);
void extclCpuWrMem_Futuremedia(WORD address, BYTE value);
BYTE extclSaveMapper_Futuremedia(BYTE mode, BYTE slot, FILE *fp);
void extclCPUEveryCycle_Futuremedia(void);
void extclIrqA12Clock_Futuremedia(void);

#endif /* MAPPERFUTUREMEDIA_H_ */
