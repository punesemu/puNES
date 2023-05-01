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

#ifndef MAPPER_FME7_H_
#define MAPPER_FME7_H_

#include "common.h"

typedef struct _square_fme7 {
	BYTE disable;
	BYTE step;
	WORD frequency;
	WORD timer;
	WORD volume;
	SWORD output;
} _square_fme7;
typedef struct _fme7 {
	BYTE reg;
	WORD prg[4];
	WORD chr[8];
	BYTE mirroring;
	struct _fme7_snd {
		BYTE reg;
		_square_fme7 square[3];
	} snd;
	struct _fme7_irq {
		BYTE control;
		WORD count;
	} irq;

/* ------------------------------------------------------- */
/* questi valori non e' necessario salvarli nei savestates */
/* ------------------------------------------------------- */
/* */ BYTE clocked;                                     /* */
/* ------------------------------------------------------- */
} _fme7;

extern _fme7 fme7;

void extcl_after_mapper_init_FME7(void);
void extcl_cpu_wr_mem_FME7(WORD address, BYTE value);
BYTE extcl_save_mapper_FME7(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_FME7(void);
void extcl_apu_tick_FME7(void);

void init_NSF_FME7(void);
void init_FME7(void);
void prg_fix_FME7_base(void);
void prg_swap_FME7_base(WORD address, WORD value);
void chr_fix_FME7_base(void);
void chr_swap_FME7_base(WORD address, WORD value);
void wram_fix_FME7_base(void);
void wram_swap_FME7_base(WORD value);
void mirroring_fix_FME7_base(void);

extern void (*FME7_prg_fix)(void);
extern void (*FME7_prg_swap)(WORD address, WORD value);
extern void (*FME7_chr_fix)(void);
extern void (*FME7_chr_swap)(WORD address, WORD value);
extern void (*FME7_wram_fix)(void);
extern void (*FME7_wram_swap)(WORD value);
extern void (*FME7_mirroring_fix)(void);

#endif /* MAPPER_FME7_H_ */
