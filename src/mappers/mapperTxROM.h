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

void extcl_cpu_wr_mem_TKSROM(WORD address, BYTE value);

void extcl_cpu_wr_mem_TQROM(WORD address, BYTE value);
void extcl_wr_chr_TQROM(WORD address, BYTE value);

BYTE extcl_save_mapper_TxROM(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPERTXROM_H_ */
