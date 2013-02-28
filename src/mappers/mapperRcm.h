/*
 * mapperRcm.h
 *
 *  Created on: 23/mar/2012
 *      Author: fhorse
 */

#ifndef MAPPERRCM_H_
#define MAPPERRCM_H_

#include "common.h"

enum { GS2015 = 2 };

void map_init_Rcm(BYTE type);

void extcl_cpu_wr_mem_GS2015(WORD address, BYTE value);

#endif /* MAPPERRCM_H_ */
