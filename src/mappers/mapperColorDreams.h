/*
 * mapperColorDreams.h
 *
 *  Created on: 11/lug/2011
 *      Author: fhorse
 */

#ifndef MAPPERCOLORDREAMS_H_
#define MAPPERCOLORDREAMS_H_

#include "common.h"

enum {
	CDNOCONFLCT = 2,
	BADKINGNEPT = 3,
};

void mapInit_ColorDreams(void);
void extclCpuWrMem_ColorDreams(WORD address, BYTE value);

#endif /* MAPPERCOLORDREAMS_H_ */
