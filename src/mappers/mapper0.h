/*
 * mapper0.h
 *
 *  Created on: 09/mag/2010
 *      Author: fhorse
 */

#ifndef MAPPER0_H_
#define MAPPER0_H_

#include "common.h"

enum  { UNKHORIZONTAL, UNKVERTICAL };

void mapInit_0(void);
void extclCpuWrMem_0(WORD address, BYTE value);
BYTE extclCpuRdMem_0(WORD address, BYTE openbus, BYTE before);

#endif /* MAPPER0_H_ */
