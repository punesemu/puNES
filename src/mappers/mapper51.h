/*
 * mapper51.h
 *
 *  Created on: 21/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER51_H_
#define MAPPER51_H_

#include "common.h"

struct _m51 {
	BYTE mode;
	WORD bank;
	BYTE prg6000;
} m51;

void mapInit_51(void);
void extclCpuWrMem_51(WORD address, BYTE value);
BYTE extclCpuRdMem_51(WORD address, BYTE openbus, BYTE before);
BYTE extclSaveMapper_51(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER51_H_ */
