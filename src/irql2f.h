/*
 * irqf2f.h
 *
 *  Created on: 02/set/2011
 *      Author: fhorse
 */

#ifndef IRQL2F_H_
#define IRQL2F_H_

#include "common.h"
#include "cpu.h"
#include "ppu.h"

enum {
	IRQL2FINFRAME = 0x40,
	IRQL2FPENDING = 0x80
};

typedef struct {
	BYTE present;
	BYTE enable;
	BYTE counter;
	BYTE scanline;
	WORD frameX;
	BYTE delay;
	BYTE inFrame;
	BYTE pending;
} _irql2f;

_irql2f irql2f;

void irql2f_tick(void);

#endif /* IRQL2F_H_ */
