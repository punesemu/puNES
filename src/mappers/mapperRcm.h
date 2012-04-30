/*
 * mapperRcm.h
 *
 *  Created on: 23/mar/2012
 *      Author: fhorse
 */

#ifndef MAPPERRCM_H_
#define MAPPERRCM_H_

#include "common.h"

enum {
	GS2015 = 2
};

void mapInit_Rcm(BYTE type);

void extclCpuWrMem_GS2015(WORD address, BYTE value);

#endif /* MAPPERRCM_H_ */
