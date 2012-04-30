/*
 * mapper230.h
 *
 *  Created on: 05/feb/2012
 *      Author: fhorse
 */

#ifndef MAPPER230_H_
#define MAPPER230_H_

#include "common.h"

struct _m230 {
	BYTE mode;
} m230;

void mapInit_230(void);
void extclCpuWrMem_230(WORD address, BYTE value);
BYTE extclSaveMapper_230(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER230_H_ */
