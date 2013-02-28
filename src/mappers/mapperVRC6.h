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
} _vrc6_square;
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
} _vrc6_saw;
struct _vrc6 {
	BYTE enabled;
	BYTE reload;
	BYTE mode;
	BYTE acknowledge;
	BYTE count;
	BYTE delay;
	WORD prescaler;
	_vrc6_square S3, S4;
	_vrc6_saw saw;
} vrc6;

void map_init_VRC6(BYTE revision);
void extcl_cpu_wr_mem_VRC6(WORD address, BYTE value);
BYTE extcl_save_mapper_VRC6(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_VRC6(void);
void extcl_apu_tick_VRC6(void);

#endif /* MAPPERVRC6_H_ */
