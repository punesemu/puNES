/*
 * mapperVRC3.h
 *
 *  Created on: 11/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERVRC3_H_
#define MAPPERVRC3_H_

#include "common.h"

struct _vrc3 {
	BYTE enabled;
	WORD reload;
	BYTE mode;
	BYTE acknowledge;
	WORD mask;
	WORD count;
} vrc3;

void mapInit_VRC3(void);
void extclCpuWrMem_VRC3(WORD address, BYTE value);
BYTE extclSaveMapper_VRC3(BYTE mode, BYTE slot, FILE *fp);
void extclCPUEveryCycle_VRC3(void);

#endif /* MAPPERVRC3_H_ */
