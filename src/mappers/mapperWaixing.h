/*
 * mapperWaixing.h
 *
 *  Created on: 11/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERWAIXING_H_
#define MAPPERWAIXING_H_

#include "common.h"

enum {
	WPSX = 2,
	WTA,
	WTB,
	WTC,
	WTD,
	WTE,
	WTG,
	WTH,
	SH2,
	BADSUGOROQUEST
};

struct _waixing {
	WORD prgmap[4];
	WORD chrmap[8];
	BYTE chrRam[0x2000];
	BYTE reg;
	WORD ctrl[8];
} waixing;

void mapInit_Waixing(BYTE model);

void extcl_cpu_wr_mem_Waixing_PSx(WORD address, BYTE value);

void extcl_cpu_wr_mem_Waixing_TypeACDE(WORD address, BYTE value);
BYTE extcl_save_mapper_Waixing_TypeACDE(BYTE mode, BYTE slot, FILE *fp);
void extcl_wr_chr_Waixing_TypeACDE(WORD address, BYTE value);

void extcl_cpu_wr_mem_Waixing_TypeB(WORD address, BYTE value);
BYTE extcl_save_mapper_Waixing_TypeB(BYTE mode, BYTE slot, FILE *fp);
void extcl_wr_chr_Waixing_TypeB(WORD address, BYTE value);

void extcl_cpu_wr_mem_Waixing_TypeG(WORD address, BYTE value);
BYTE extcl_save_mapper_Waixing_TypeG(BYTE mode, BYTE slot, FILE *fp);
void extcl_wr_chr_Waixing_TypeG(WORD address, BYTE value);

void extcl_cpu_wr_mem_Waixing_TypeH(WORD address, BYTE value);
BYTE extcl_save_mapper_Waixing_TypeH(BYTE mode, BYTE slot, FILE *fp);

void extcl_cpu_wr_mem_Waixing_SH2(WORD address, BYTE value);
BYTE extcl_save_mapper_Waixing_SH2(BYTE mode, BYTE slot, FILE *fp);
void extcl_update_r2006_Waixing_SH2(WORD old_r2006);
void extcl_after_rd_chr_Waixing_SH2(WORD address);
void extcl_wr_chr_Waixing_SH2(WORD address, BYTE value);

#endif /* MAPPERWAIXING_H_ */
