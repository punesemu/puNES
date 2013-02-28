/*
 * mapperTxc.h
 *
 *  Created on: 30/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERTXC_H_
#define MAPPERTXC_H_

#include "common.h"

enum { TXCTW = 2, T22211A, T22211B, T22211C };

struct _t22211x {
	BYTE reg[4];
} t22211x;

void map_init_Txc(BYTE model);

void extcl_cpu_wr_mem_Txc_tw(WORD address, BYTE value);

void extcl_cpu_wr_mem_Txc_t22211x(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_Txc_t22211x(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_Txc_t22211x(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPERTXC_H_ */
