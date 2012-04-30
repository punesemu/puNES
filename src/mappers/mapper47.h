/*
 * mapper47.h
 *
 *  Created on: 21/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER47_H_
#define MAPPER47_H_

#include "common.h"

struct _m47 {
	BYTE reg;
	WORD prgmap[4];
	WORD chrmap[8];
} m47;

void mapInit_47(void);
void extclCpuWrMem_47(WORD address, BYTE value);
BYTE extclSaveMapper_47(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER47_H_ */
