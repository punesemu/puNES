/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#ifndef MAPPER_TXC_H_
#define MAPPER_TXC_H_

#include "common.h"

typedef struct _txc {
	BYTE increase;
	BYTE output;
	BYTE invert;
	BYTE staging;
	BYTE accumulator;
	BYTE inverter;
	BYTE A;
	BYTE B;
	BYTE X;
	BYTE Y;
} _txc;

extern _txc txc;

void extcl_after_mapper_init_TXC(void);
void extcl_cpu_wr_mem_TXC(BYTE nidx, WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_TXC(BYTE nidx, WORD address, BYTE openbus);
BYTE extcl_save_mapper_TXC(BYTE mode, BYTE slot, FILE *fp);

void init_TXC(BYTE reset);
void prg_fix_TXC_base(void);
void chr_fix_TXC_base(void);
void wram_fix_TXC_base(void);
void mirroring_fix_TXC_base(void);

extern void (*TXC_prg_fix)(void);
extern void (*TXC_chr_fix)(void);
extern void (*TXC_wram_fix)(void);
extern void (*TXC_mirroring_fix)(void);

#endif /* MAPPER_TXC_H_ */
