/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

#ifndef MAPPER_VRC6_H_
#define MAPPER_VRC6_H_

#include "common.h"

typedef struct _vrc6_square {
	BYTE enabled;
	BYTE duty;
	BYTE step;
	BYTE volume;
	BYTE mode;
	WORD timer;
	WORD frequency;
	SWORD output;
} _vrc6_square;
typedef struct _vrc6_saw {
	BYTE enabled;
	BYTE accumulator;
	BYTE step;
	BYTE internal;
	WORD timer;
	WORD frequency;
	SWORD output;
} _vrc6_saw;
typedef struct _vrc6 {
	BYTE reg;
	WORD prg[2];
	WORD chr[8];
	_vrc6_square S3, S4;
	_vrc6_saw saw;
	struct _vrc6_irq {
		BYTE enabled;
		BYTE reload;
		BYTE mode;
		BYTE acknowledge;
		BYTE count;
		BYTE delay;
		WORD prescaler;
	} irq;

	/* ------------------------------------------------------- */
	/* questi valori non e' necessario salvarli nei savestates */
	/* ------------------------------------------------------- */
	/* */ BYTE clocked;                                     /* */
	/* ------------------------------------------------------- */
} _vrc6;

extern _vrc6 vrc6;

void extcl_after_mapper_init_VRC6(void);
void extcl_cpu_wr_mem_VRC6(BYTE cidx, WORD address, BYTE value);
BYTE extcl_save_mapper_VRC6(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_VRC6(BYTE cidx);
void extcl_apu_tick_VRC6(void);

void init_NSF_VRC6(WORD A0, WORD A1);
void init_VRC6(WORD A0, WORD A1, BYTE reset);
void prg_fix_VRC6_base(void);
void prg_swap_VRC6_base(WORD address, WORD value);
void chr_fix_VRC6_base(void);
void chr_swap_VRC6_base(WORD address, WORD value);
void wram_fix_VRC6_base(void);
void mirroring_fix_VRC6_base(void);
void nmt_swap_VRC6_base(WORD address, WORD value);

extern void (*VRC6_prg_fix)(void);
extern void (*VRC6_prg_swap)(WORD address, WORD value);
extern void (*VRC6_chr_fix)(void);
extern void (*VRC6_chr_swap)(WORD address, WORD value);
extern void (*VRC6_wram_fix)(void);
extern void (*VRC6_mirroring_fix)(void);
extern void (*VRC6_nmt_swap)(WORD address, WORD value);

#endif /* MAPPER_VRC6_H_ */
