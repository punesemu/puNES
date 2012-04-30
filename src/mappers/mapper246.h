/*
 * mapper246.h
 *
 *  Created on: 24/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER246_H_
#define MAPPER246_H_

#include "common.h"

void mapInit_246(void);
void extclCpuWrMem_246(WORD address, BYTE value);
BYTE extclCpuRdMem_246(WORD address, BYTE openbus, BYTE before);

#endif /* MAPPER246_H_ */
