/*
 * mapperVs.h
 *
 *  Created on: 22/set/2010
 *      Author: fhorse
 */

#ifndef MAPPERVS_H_
#define MAPPERVS_H_

#include "common.h"

void mapInit_Vs(void);
void extclCpuWrMem_Vs(WORD address, BYTE value);
void extclCPUWr4016_Vs(BYTE value);

#endif /* MAPPERVS_H_ */
