/*
 * mapperTxROM.h
 *
 *  Created on: 28/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERTXROM_H_
#define MAPPERTXROM_H_

#include "common.h"

enum {
	TKSROM = 2,
	TLSROM = 3,
	TQROM = 4
};

struct _txrom {
	BYTE delay;
	uint32_t chr[8][2];
	BYTE chrRam[0x2000];
} txrom;

void mapInit_TxROM(BYTE model);

void extclCpuWrMem_TKSROM(WORD address, BYTE value);

void extclCpuWrMem_TQROM(WORD address, BYTE value);
void extclWrChr_TQROM(WORD address, BYTE value);

BYTE extclSaveMapper_TxROM(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPERTXROM_H_ */
