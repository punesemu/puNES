/*
 * mapper219.h
 *
 *  Created on: 20/mar/2012
 *      Author: fhorse
 */

#ifndef MAPPER219_H_
#define MAPPER219_H_

#include "common.h"

struct _m219 {
	BYTE reg[3];
} m219;

void mapInit_219(void);
void extclCpuWrMem_219(WORD address, BYTE value);
BYTE extclSaveMapper_219(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER219_H_ */
