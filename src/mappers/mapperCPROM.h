/*
 * mapperCPROM.h
 *
 *  Created on: 14/lug/2011
 *      Author: fhorse
 */

#ifndef MAPPERCPROM_H_
#define MAPPERCPROM_H_

#include "common.h"

void mapInit_CPROM(void);
void extclCpuWrMem_CPROM(WORD address, BYTE value);

#endif /* MAPPERCPROM_H_ */
