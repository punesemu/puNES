/*
 * mapperGXROM.h
 *
 *  Created on: 16/lug/2011
 *      Author: fhorse
 */

#ifndef MAPPERGXROM_H_
#define MAPPERGXROM_H_

#include "common.h"

void mapInit_GxROM(void);
void extclCpuWrMem_GxROM(WORD address, BYTE value);

#endif /* MAPPERGXROM_H_ */
