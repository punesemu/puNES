/*
 * mapper60.h
 *
 *  Created on: 22/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER60_H_
#define MAPPER60_H_

#include "common.h"

enum { M60 = 2, M60VT5201 };

struct _m60 {
	BYTE index;
} m60;

void mapInit_60(void);
void extclCpuWrMem_60(WORD address, BYTE value);
BYTE extclSaveMapper_60(BYTE mode, BYTE slot, FILE *fp);

void mapInit_60_vt5201(void);
void extclCpuWrMem_60_vt5201(WORD address, BYTE value);

#endif /* MAPPER60_H_ */
