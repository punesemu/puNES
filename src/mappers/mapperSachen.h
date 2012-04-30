/*
 * mapperSachen.h
 *
 *  Created on: 27/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERSACHEN_H_
#define MAPPERSACHEN_H_

#include "common.h"

enum {
	SA0036 = 2,
	SA0037 = 3,
	SA8259A = 4,
	SA8259B = 5,
	SA8259C = 6,
	SA8259D = 7,
	TCA01 = 8,
	TCU01 = 9,
	TCU02 = 10,
	SA72007 = 11,
	SA72008 = 12,
	SA74374A = 13,
	SA74374B = 14,
};

struct _sa8259 {
	BYTE ctrl;
	BYTE reg[8];
} sa8259;
struct _tcu02 {
	BYTE reg;
} tcu02;
struct _sa74374x {
	BYTE reg;
	BYTE chrRom8kBank;
} sa74374x;

static const char pokeriiichr[2][40] = {
	"5066c2d12ff2ac45ef395d3a4353e897fce19f78",
	"c6bf926ed14c21f1a5b64fbccf3288005ff54be5"
};

void mapInit_Sachen(BYTE model);

void extclCpuWrMem_Sachen_sa0036(WORD address, BYTE value);

void extclCpuWrMem_Sachen_sa0037(WORD address, BYTE value);

void extclCpuWrMem_Sachen_sa8259x(WORD address, BYTE value);
BYTE extclSaveMapper_Sachen_sa8259x(BYTE mode, BYTE slot, FILE *fp);

void extclCpuWrMem_Sachen_tca01(WORD address, BYTE value);
BYTE extclCpuRdMem_Sachen_tca01(WORD address, BYTE openbus, BYTE before);

void extclCpuWrMem_Sachen_tcu01(WORD address, BYTE value);

void extclCpuWrMem_Sachen_tcu02(WORD address, BYTE value);
BYTE extclCpuRdMem_Sachen_tcu02(WORD address, BYTE openbus, BYTE before);
BYTE extclSaveMapper_Sachen_tcu02(BYTE mode, BYTE slot, FILE *fp);

void extclCpuWrMem_Sachen_sa72007(WORD address, BYTE value);

void extclCpuWrMem_Sachen_sa72008(WORD address, BYTE value);

void extclCpuWrMem_Sachen_sa74374a(WORD address, BYTE value);
void extclCpuWrMem_Sachen_sa74374b(WORD address, BYTE value);
BYTE extclSaveMapper_Sachen_sa74374x(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPERSACHEN_H_ */
