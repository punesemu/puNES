/*
 * mapper_Rcm.h
 *
 *  Created on: 23/mar/2012
 *      Author: fhorse
 */

#ifndef MAPPER_RCM_H_
#define MAPPER_RCM_H_

#include "common.h"

enum { GS2015 };

void map_init_Rcm(BYTE type);

void extcl_cpu_wr_mem_GS2015(WORD address, BYTE value);

#endif /* MAPPER_RCM_H_ */
