/*
 * mapperNtdec.h
 *
 *  Created on: 26/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERNTDEC_H_
#define MAPPERNTDEC_H_

#include "common.h"

enum {
	ASDER = 2,
	FHERO = 3
};

struct _asder {
	BYTE address;
	BYTE reg[8];
} asder;

void mapInit_Ntdec(BYTE model);

void extclCpuWrMem_Ntdec_asder(WORD address, BYTE value);
BYTE extclSaveMapper_Ntdec_asder(BYTE mode, BYTE slot, FILE *fp);

void extclCpuWrMem_Ntdec_fhero(WORD address, BYTE value);

#endif /* MAPPERNTDEC_H_ */
