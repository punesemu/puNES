/*
 * mapper241.h
 *
 *  Created on: 24/mar/2012
 *      Author: fhorse
 */

#ifndef MAPPER241_H_
#define MAPPER241_H_

#include "common.h"

void mapInit_241(void);
void extclCpuWrMem_241(WORD address, BYTE value);
BYTE extclCpuRdMem_241(WORD address, BYTE openbus, BYTE before);

#endif /* MAPPER241_H_ */
