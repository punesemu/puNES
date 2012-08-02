/*
 * mapperVRC6.h
 *
 *  Created on: 22/gen/2012
 *      Author: fhorse
 */

#ifndef MAPPERVRC6_H_
#define MAPPERVRC6_H_

#include "common.h"

enum { VRC6A, VRC6B };

typedef struct {
	BYTE enabled;
	BYTE duty;
	BYTE step;
	BYTE volume;
	BYTE mode;
	WORD timer;
	WORD frequency;
	SWORD output;

/* ------------------------------------------------------- */
/* questi valori non e' necessario salvarli nei savestates */
/* ------------------------------------------------------- */
/* */ BYTE clocked;                                     /* */
/* ------------------------------------------------------- */
} _vrc6Square;
typedef struct {
	BYTE enabled;
	BYTE accumulator;
	BYTE step;
	BYTE internal;
	WORD timer;
	WORD frequency;
	SWORD output;

/* ------------------------------------------------------- */
/* questi valori non e' necessario salvarli nei savestates */
/* ------------------------------------------------------- */
/* */ BYTE clocked;                                     /* */
/* ------------------------------------------------------- */
} _vrc6Saw;
struct _vrc6 {
	BYTE enabled;
	BYTE reload;
	BYTE mode;
	BYTE acknowledge;
	BYTE count;
	BYTE delay;
	WORD prescaler;
	_vrc6Square S3, S4;
	_vrc6Saw saw;
} vrc6;

void mapInit_VRC6(BYTE revision);
void extclCpuWrMem_VRC6(WORD address, BYTE value);
BYTE extclSaveMapper_VRC6(BYTE mode, BYTE slot, FILE *fp);
void extclCPUEveryCycle_VRC6(void);
void extclApuTick_VRC6(void);

#endif /* MAPPERVRC6_H_ */
