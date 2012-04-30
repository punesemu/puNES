/*
 * mapper208.h
 *
 *  Created on: 01/ott/2011
 *      Author: fhorse
 */

#ifndef MAPPER208_H_
#define MAPPER208_H_

#include "common.h"

struct _m208 {
	BYTE ctrl;
	BYTE reg[4];
} m208;

void mapInit_208(void);
void extclCpuWrMem_208(WORD address, BYTE value);
BYTE extclCpuRdMem_208(WORD address, BYTE openbus, BYTE before);
BYTE extclSaveMapper_208(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER208_H_ */
