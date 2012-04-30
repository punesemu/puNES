/*
 * mapper46.h
 *
 *  Created on: 20/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER46_H_
#define MAPPER46_H_

#include "common.h"

struct _m46 {
	BYTE prg;
	BYTE chr;
} m46;

void mapInit_46(void);
void extclCpuWrMem_46(WORD address, BYTE value);
BYTE extclSaveMapper_46(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER46_H_ */
