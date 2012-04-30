/*
 * mapper62.h
 *
 *  Created on: 22/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER62_H_
#define MAPPER62_H_

#include "common.h"

enum { SUPER700IN1 = 2 };

void mapInit_62(void);
void extclCpuWrMem_62(WORD address, BYTE value);

#endif /* MAPPER62_H_ */
