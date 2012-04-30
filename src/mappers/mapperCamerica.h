/*
 * mapperCamerica.h
 *
 *  Created on: 12/lug/2011
 *      Author: fhorse
 */

#ifndef MAPPERCAMERICA_H_
#define MAPPERCAMERICA_H_

#include "common.h"

enum {
	BF9093     = 2,
	BF9096     = 3,
	BF9097     = 4,
	GOLDENFIVE = 5
};

void mapInit_Camerica(void);
void extclCpuWrMem_Camerica_BF9093(WORD address, BYTE value);
void extclCpuWrMem_Camerica_BF9096(WORD address, BYTE value);
void extclCpuWrMem_Camerica_BF9097(WORD address, BYTE value);
void extclCpuWrMem_Camerica_GoldenFive(WORD address, BYTE value);

#endif /* MAPPERCAMERICA_H_ */
