/*
 * mapper53.h
 *
 *  Created on: 21/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER53_H_
#define MAPPER53_H_

#include "common.h"

struct _m53 {
	BYTE reg[2];
	BYTE prg6000;
} m53;

void mapInit_53(void);
void extclCpuWrMem_53(WORD address, BYTE value);
BYTE extclCpuRdMem_53(WORD address, BYTE openbus, BYTE before);
BYTE extclSaveMapper_53(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER53_H_ */
