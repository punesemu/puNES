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

#ifndef MAPPER_MMC1_H_
#define MAPPER_MMC1_H_

#include "common.h"

enum MMC1_types {
	SNROM,
	SUROM,
	SOROM,
	MMC1_SUBMAPPER3_DEPRECATED3,
	SXROM = 4,
	SEROM = 5,
	MMC1_2ME,
	MMC1A = 10,
	MMC1B,
	BAD_YOSHI_U = 20,
	MOWPC10
};

typedef struct _mmc1 {
	WORD reg[4];
	BYTE accumulator;
	BYTE shift;
	BYTE reset;
} _mmc1;
typedef struct _mmc1tmp {
	BYTE type;
} _mmc1tmp;

extern _mmc1 mmc1;
extern _mmc1tmp mmc1tmp;

void extcl_after_mapper_init_MMC1(void);
void extcl_cpu_wr_mem_MMC1(WORD address, BYTE value);
BYTE extcl_save_mapper_MMC1(BYTE mode, BYTE slot, FILE *fp);

void init_MMC1(BYTE type);
void prg_fix_MMC1(void);
void prg_swap_MMC1(WORD address, WORD value);
void chr_fix_MMC1(void);
void chr_swap_MMC1(WORD address, WORD value);
void wram_fix_MMC1(void);
void wram_swap_MMC1(WORD value);
void mirroring_fix_MMC1(void);

WORD prg_bank_MMC1(int index);
WORD chr_bank_MMC1(int index);

extern void (*MMC1_prg_fix)(void);
extern void (*MMC1_prg_swap)(WORD address, WORD value);
extern void (*MMC1_chr_fix)(void);
extern void (*MMC1_chr_swap)(WORD address, WORD value);
extern void (*MMC1_wram_fix)(void);
extern void (*MMC1_wram_swap)(WORD value);
extern void (*MMC1_mirroring_fix)(void);

#endif /* MAPPER_MMC1_H_ */
