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

void extclCpuWrMem_Waixing_PSx(WORD address, BYTE value);

void extclCpuWrMem_Waixing_TypeACDE(WORD address, BYTE value);
BYTE extclSaveMapper_Waixing_TypeACDE(BYTE mode, BYTE slot, FILE *fp);
void extclWrChr_Waixing_TypeACDE(WORD address, BYTE value);

void extclCpuWrMem_Waixing_TypeB(WORD address, BYTE value);
BYTE extclSaveMapper_Waixing_TypeB(BYTE mode, BYTE slot, FILE *fp);
void extclWrChr_Waixing_TypeB(WORD address, BYTE value);

void extclCpuWrMem_Waixing_TypeG(WORD address, BYTE value);
BYTE extclSaveMapper_Waixing_TypeG(BYTE mode, BYTE slot, FILE *fp);
void extclWrChr_Waixing_TypeG(WORD address, BYTE value);

void extclCpuWrMem_Waixing_TypeH(WORD address, BYTE value);
BYTE extclSaveMapper_Waixing_TypeH(BYTE mode, BYTE slot, FILE *fp);

void extclCpuWrMem_Waixing_SH2(WORD address, BYTE value);
BYTE extclSaveMapper_Waixing_SH2(BYTE mode, BYTE slot, FILE *fp);
void extcl2006Update_Waixing_SH2(WORD r2006Old);
void extclRdChrAfter_Waixing_SH2(WORD address);
void extclWrChr_Waixing_SH2(WORD address, BYTE value);

#endif /* MAPPERWAIXING_H_ */
