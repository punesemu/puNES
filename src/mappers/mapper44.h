/*
 * mapper44.h
 *
 *  Created on: 19/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER44_H_
#define MAPPER44_H_

#include "common.h"

struct _m44 {
	BYTE reg;
	WORD prgmap[4];
	WORD chrmap[8];
} m44;

void mapInit_44(void);
void extclCpuWrMem_44(WORD address, BYTE value);
BYTE extclSaveMapper_44(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER44_H_ */
