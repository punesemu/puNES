/*
 * clock.h
 *
 *  Created on: 10/mag/2010
 *      Author: fhorse
 */

#ifndef CLOCK_H_
#define CLOCK_H_

typedef struct {
	WORD type;             /* il tipo di formato */
	BYTE fps;              /* il framerate */
	double baseHz;          /* il clock di base (Hz) */
	BYTE baseDivide;       /* il divisore del clock di base */
	double masterHz;        /* master clock (Hz) = ((baseClock / baseClockDivide) * 6) */
	WORD masterHB;         /* numero di master cycles necessari per una scanline */
	WORD totalLines;       /* numero totale delle linee disegnate */
	WORD visibleLines;     /* numero delle scanlines visibili */
	BYTE vintLines;        /* durata (in scanlines) del VINT */
	double ppuHz;           /* PPU clock (masterHz / ppuDivide) */
	BYTE ppuDivide;        /* il divisore del master clock per ottenere il PPU clock */
	BYTE ppuFor1CPU;       /* numero di cicli PPU che servono per avere un ciclo CPU */
	SDBWORD ppuCylesPermitWrite; /* i cicli CPU dall'avvio/reset prima di
                                  * poter scrivere nei registri della PPU */
	BYTE ppuOpenbusFrames; /* frames per il decadimento di un bit openbus del PPU */
	BYTE cpuDivide;        /* CPU clock e' 12 volte piu' lento del master clock */
	double cpuHz;           /* CPU clock (masterHz / cpuDivide) */
	float cpuCyclesFrame;  /* numero di cicli CPU per ogni frame (CPU Clock / fps) */
	double msFrame;        /* i ms dedicati ad ogni frame */
} _machine;

static const _machine machinedb[] = {
	{
		NTSC,
		60,
		39375000,
		11,
		((39375000.0 / 11) * 6),
		1364,
		262,
		224,
		20,
		(((39375000.0 / 11) * 6) / 4),
		4,
		3,
		29658,
		600 / (1000 / 60),
		12,
		(((39375000.0 / 11) * 6) / 12),
		((((39375000.0 / 11) * 6) / 12) / 60),
		17.5f
	},
	{
		PAL,
		50,
		35468950,
		8,
		((35468950.0 / 8) * 6),
		1705,
		312,
		240,
		70,
		(((35468950.0 / 8) * 6) / 5),
		5,
		3,
		33132,
		600 / (1000 / 50),
		16,
		(((35468950.0 / 8) * 6) / 16),
		((((35468950.0 / 8) * 6) / 16) / 50),
		20.0f
	},
	{
		DENDY,
		50,
		35468950,
		8,
		((35468950.0 / 8) * 6),
		1705,
		312,
		240,
		20,
		(((35468950.0 / 8) * 6) / 5),
		5,
		3,
		33132,
		600 / (1000 / 50),
		15,
		(((35468950.0 / 8) * 6) / 15),
		((((35468950.0 / 8) * 6) / 15) / 50),
		20.0f
	}
};

_machine machine;

#endif /* CLOCK_H_ */
