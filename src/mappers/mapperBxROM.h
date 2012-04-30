/*
 * mapperBxROM.h
 *
 *  Created on: 16/lug/2011
 *      Author: fhorse
 */

#ifndef MAPPERBXROM_H_
#define MAPPERBXROM_H_

#include "common.h"

enum { AVENINA001 = 2, BXROMUNL = 3 };

void mapInit_BxROM(void);

void extclCpuWrMem_BxROM(WORD address, BYTE value);

void extclCpuWrMem_BxROM_UNL(WORD address, BYTE value);

void extclCpuWrMem_AveNina001(WORD address, BYTE value);

#endif /* MAPPERBXROM_H_ */
