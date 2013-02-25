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

void extcl_cpu_wr_mem_Irem_G101(WORD address, BYTE value);
BYTE extcl_save_mapper_Irem_G101(BYTE mode, BYTE slot, FILE *fp);

void extcl_cpu_wr_mem_Irem_H3000(WORD address, BYTE value);
BYTE extcl_save_mapper_Irem_H3000(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_Irem_H3000(void);

void extcl_cpu_wr_mem_Irem_LROG017(WORD address, BYTE value);
BYTE extcl_save_mapper_Irem_LROG017(BYTE mode, BYTE slot, FILE *fp);
void extcl_wr_chr_Irem_LROG017(WORD address, BYTE value);

void extcl_cpu_wr_mem_Irem_TAMS1(WORD address, BYTE value);

#endif /* MAPPERIREM_H_ */
