/*
 * mapper_62.h
 *
 *  Created on: 22/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER_62_H_
#define MAPPER_62_H_

#include "common.h"

enum { SUPER700IN1 };

void map_init_62(void);
void extcl_cpu_wr_mem_62(WORD address, BYTE value);

#endif /* MAPPER_62_H_ */
