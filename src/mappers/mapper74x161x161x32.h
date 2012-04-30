/*
 * mapper74x161x161x32.h
 *
 *  Created on: 19/set/2011
 *      Author: fhorse
 */

#ifndef MAPPER74X161X161X32_H_
#define MAPPER74X161X161X32_H_

#include "common.h"

enum {
	IC74X161X161X32A = 2,
	IC74X161X161X32B = 3,
};

void mapInit_74x161x161x32(BYTE model);
void extclCpuWrMem_74x161x161x32(WORD address, BYTE value);

#endif /* MAPPER74X161X161X32_H_ */
