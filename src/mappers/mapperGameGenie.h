/*
 * mapperGameGenie.h
 *
 *  Created on: 16/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPERGAMEGENIE_H_
#define MAPPERGAMEGENIE_H_

#include "common.h"

enum { GAMEGENIE_MAPPER = 0x1001 };

void mapInit_GameGenie(void);
void extclCpuWrMem_GameGenie(WORD address, BYTE value);

#endif /* MAPPERGAMEGENIE_H_ */
