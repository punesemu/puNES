/*
 * mapper226.h
 *
 *  Created on: 08/feb/2012
 *      Author: fhorse
 */

#ifndef MAPPER226_H_
#define MAPPER226_H_

#include "common.h"

struct _m226 {
	BYTE reg[2];
} m226;

void mapInit_226(void);
void extclCpuWrMem_226(WORD address, BYTE value);
BYTE extclSaveMapper_226(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER226_H_ */
