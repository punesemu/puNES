/*
 * mapper235.h
 *
 *  Created on: 11/feb/2012
 *      Author: fhorse
 */

#ifndef MAPPER235_H_
#define MAPPER235_H_

#include "common.h"

struct _m235 {
	BYTE openbus;
} m235;

void mapInit_235(void);
void extclCpuWrMem_235(WORD address, BYTE value);
BYTE extclCpuRdMem_235(WORD address, BYTE openbus, BYTE before);
BYTE extclSaveMapper_235(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER235_H_ */
