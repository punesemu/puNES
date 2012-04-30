/*
 * mapperHen.h
 *
 *  Created on: 6/ott/2011
 *      Author: fhorse
 */

#ifndef MAPPERHEN_H_
#define MAPPERHEN_H_

#include "common.h"

enum {
	HEN177 = 2,
	HENXJZB = 3,
	HENFANKONG = 4
};

void mapInit_Hen(BYTE model);

void extclCpuWrMem_Hen_177(WORD address, BYTE value);

void extclCpuWrMem_Hen_xjzb(WORD address, BYTE value);

#endif /* MAPPERHEN_H_ */
