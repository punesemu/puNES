/*
 * mapperTaito.h
 *
 *  Created on: 17/lug/2011
 *      Author: fhorse
 */

#ifndef MAPPERTAITO_H_
#define MAPPERTAITO_H_

#include "common.h"

enum {
	TC0190FMC = 2,
	TC0190FMCPAL16R4 = 3,
	X1005A = 4,
	X1005B = 5,
	X1017 = 6,
	BADINEFLINJ = 100,
	X1005NOBAT = 101
};

struct _taitoX1005 {
	BYTE ram[0x80];
	BYTE enable;
} taitoX1005;
struct _taitoX1017 {
	BYTE chr[6];
	BYTE control;
} taitoX1017;

void mapInit_Taito(BYTE model);

void extclCpuWrMem_Taito_TC0190FMC(WORD address, BYTE value);

void extclCpuWrMem_Taito_TC0190FMCPAL16R4(WORD address, BYTE value);
void extclPPU000to34x_Taito_TC0190FMCPAL16R4(void);
void extclPPU000to255_Taito_TC0190FMCPAL16R4(void);
void extclPPU256to319_Taito_TC0190FMCPAL16R4(void);
void extclPPU320to34x_Taito_TC0190FMCPAL16R4(void);

void extclCpuWrMem_Taito_X1005(WORD address, BYTE value);
BYTE extclCpuRdMem_Taito_X1005(WORD address, BYTE openbus, BYTE before);
BYTE extclSaveMapper_Taito_X1005(BYTE mode, BYTE slot, FILE *fp);
void extclBatteryIO_Taito_X1005(BYTE mode, FILE *fp);

void extclCpuWrMem_Taito_X1017(WORD address, BYTE value);
BYTE extclSaveMapper_Taito_X1017(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPERTAITO_H_ */
