/*
 * mapper_GameGenie.h
 *
 *  Created on: 16/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER_GAMEGENIE_H_
#define MAPPER_GAMEGENIE_H_

#include "common.h"

enum { GAMEGENIE_MAPPER = 0x1001 };

void map_init_GameGenie(void);
void extcl_cpu_wr_mem_GameGenie(WORD address, BYTE value);

#endif /* MAPPER_GAMEGENIE_H_ */
