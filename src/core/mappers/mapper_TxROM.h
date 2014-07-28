/*
 * mapper_TxROM.h
 *
 *  Created on: 28/set/2011
 *      Author: fhorse
 */

#ifndef MAPPER_TXROM_H_
#define MAPPER_TXROM_H_

#include "common.h"

enum { TKSROM, TLSROM, TQROM };

struct _txrom {
	BYTE delay;
	uint32_t chr[8][2];
} txrom;

void map_init_TxROM(BYTE model);

void extcl_cpu_wr_mem_TKSROM(WORD address, BYTE value);

void extcl_cpu_wr_mem_TQROM(WORD address, BYTE value);
void extcl_wr_chr_TQROM(WORD address, BYTE value);

BYTE extcl_save_mapper_TxROM(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_TXROM_H_ */
