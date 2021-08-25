/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef CLOCK_H_
#define CLOCK_H_

#include "common.h"

typedef struct _machine {
	WORD type;                      /* il tipo di formato */
	BYTE fps;                       /* il framerate */
	double base_hz;                 /* il clock di base (Hz) */
	BYTE base_divide;               /* il divisore del clock di base */
	double master_hz;               /* master clock (Hz) = ((base_hz / base_divide) * 6) */
	WORD master_hb;                 /* numero di master cycles necessari per una scanline */
	WORD total_lines;               /* numero totale delle linee disegnate */
	WORD visible_lines;             /* numero delle scanlines visibili */
	BYTE vint_lines;                /* durata (in scanlines) del VINT */
	double ppu_hz;                  /* PPU clock (master_hz / ppu_divide) */
	BYTE ppu_divide;                /* il divisore del master clock per ottenere il PPU clock */
	BYTE ppu_for_1_cycle_cpu;       /* numero di cicli PPU che servono per avere un ciclo CPU */
	SDBWORD ppu_cyles_permit_write; /* i cicli CPU dall'avvio/reset prima di
	                                 * poter scrivere nei registri della PPU */
	BYTE ppu_openbus_frames;        /* frames per il decadimento di un bit openbus del PPU */
	BYTE cpu_divide;                /* CPU clock e' 12 volte piu' lento del master clock */
	double cpu_hz;                  /* CPU clock (master_hz / cpu_divide) */
	float cpu_cycles_frame;         /* numero di cicli CPU per ogni frame (CPU Clock / fps) */
	double ms_frame;                /* i ms dedicati ad ogni frame */
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

extern _machine machine;

#endif /* CLOCK_H_ */
