/*
 * mapperIrem.h
 *
 *  Created on: 13/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERIREM_H_
#define MAPPERIREM_H_

#include "common.h"

enum {
	G101 = 2,
	G101A = 3,
	G101B = 4,
	H3000 = 5,
	LROG017 = 6,
	TAMS1 = 7,
	MAJORLEAGUE = 8
};

struct _iremG101 {
	BYTE prgMode;
	BYTE prgReg;
} iremG101;
struct _iremH3000 {
	BYTE enable;
	WORD count;
	WORD reload;
	BYTE delay;
} iremH3000;
struct _iremLROG017 {
	BYTE chrRam[0x1800];
} iremLROG017;

void mapInit_Irem(BYTE model);

void extclCpuWrMem_Irem_G101(WORD address, BYTE value);
BYTE extclSaveMapper_Irem_G101(BYTE mode, BYTE slot, FILE *fp);

void extclCpuWrMem_Irem_H3000(WORD address, BYTE value);
BYTE extclSaveMapper_Irem_H3000(BYTE mode, BYTE slot, FILE *fp);
void extclCPUEveryCycle_Irem_H3000(void);

void extclCpuWrMem_Irem_LROG017(WORD address, BYTE value);
BYTE extclSaveMapper_Irem_LROG017(BYTE mode, BYTE slot, FILE *fp);
void extclWrChr_Irem_LROG017(WORD address, BYTE value);

void extclCpuWrMem_Irem_TAMS1(WORD address, BYTE value);

#endif /* MAPPERIREM_H_ */
