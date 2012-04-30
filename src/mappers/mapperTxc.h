/*
 * mapperTxc.h
 *
 *  Created on: 30/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERTXC_H_
#define MAPPERTXC_H_

#include "common.h"

enum {
	TXCTW = 2,
	T22211A = 3,
	T22211B = 4,
	T22211C = 5
};

struct _t22211x {
	BYTE reg[4];
} t22211x;

void mapInit_Txc(BYTE model);

void extclCpuWrMem_Txc_tw(WORD address, BYTE value);

void extclCpuWrMem_Txc_t22211x(WORD address, BYTE value);
BYTE extclCpuRdMem_Txc_t22211x(WORD address, BYTE openbus, BYTE before);
BYTE extclSaveMapper_Txc_t22211x(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPERTXC_H_ */
