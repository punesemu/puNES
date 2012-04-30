/*
 * mapperAve.h
 *
 *  Created on: 19/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERAVE_H_
#define MAPPERAVE_H_

#include "common.h"

enum { NINA06 = 2, D1012 = 3, PUZZLEUNL = 4 };

struct _ave_d1012 {
	BYTE reg[3];
} ave_d1012;

void mapInit_Ave(BYTE model);

void extclCpuWrMem_Ave_NINA06(WORD address, BYTE value);

void extclCpuWrMem_Ave_D1012(WORD address, BYTE value);
BYTE extclCpuRdMem_Ave_D1012(WORD address, BYTE openbus, BYTE before);
BYTE extclSaveMapper_Ave_D1012(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPERAVE_H_ */
