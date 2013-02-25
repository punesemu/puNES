/*
 * mapper116.h
 *
 *  Created on: 24/apr/2012
 *      Author: fhorse
 */

#ifndef MAPPER116_H_
#define MAPPER116_H_

#include "common.h"

enum {
	M116TYPEA = 2,
	M116TYPEB,
	M116TYPEC
};

struct _m166 {
	BYTE mode;

	struct {
		WORD chr[8];
		WORD prg[4];
		BYTE nmt;
		BYTE padding;
	} mode0;
	struct {
		WORD banks[10];
		BYTE ctrl;
		BYTE nmt;
	} mode1;
	struct {
		BYTE reg[4];
		BYTE buffer;
		BYTE shifter;
		BYTE padding[2];
	} mode2;

	WORD prgmap[4];
	WORD chrmap[8];
	BYTE chrRam[0x2000];
} m116;

void map_init_116(void);

void extcl_cpu_wr_mem_116_TypeA(WORD address, BYTE value);
BYTE extcl_save_mapper_116_TypeA(BYTE mode, BYTE slot, FILE *fp);

void extcl_cpu_wr_mem_116_TypeB(WORD address, BYTE value);
BYTE extcl_save_mapper_116_TypeB(BYTE mode, BYTE slot, FILE *fp);
void extcl_wr_chr_116_TypeB(WORD address, BYTE value);

void extcl_cpu_wr_mem_116_TypeC(WORD address, BYTE value);
BYTE extcl_save_mapper_116_TypeC(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER116_H_ */
