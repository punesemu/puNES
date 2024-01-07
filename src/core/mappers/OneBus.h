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

#ifndef ONEBUSV_H_
#define ONEBUSV_H_

#include "common.h"

typedef struct _onebus {
	void *gpio[4];
	WORD relative_8k;
	struct _onebus_regs {
		BYTE cpu[0x100];
		BYTE ppu[0x100];
		BYTE apu[0x40];
	} reg;
	struct _onebus_pcm {
		SWORD address;
		SWORD size;
		SWORD latch;
		SWORD clock;
		BYTE enable;
		BYTE irq;
	} pcm;
} _onebus;

extern _onebus onebus;

void extcl_after_mapper_init_OneBus(void);
void extcl_mapper_quit_OneBus(void);
void extcl_cpu_wr_mem_OneBus(BYTE nidx, WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_OneBus(BYTE nidx, WORD address, BYTE openbus);
BYTE extcl_save_mapper_OneBus(BYTE mode, BYTE slot, FILE *fp);
BYTE extcl_wr_ppu_reg_OneBus(BYTE nidx, WORD address, BYTE *value);
BYTE extcl_wr_apu_OneBus(BYTE nidx, WORD address, BYTE *value);
BYTE extcl_rd_apu_OneBus(BYTE nidx, WORD address, BYTE openbus);
BYTE extcl_rd_chr_OneBus(BYTE nidx, WORD address);
void extcl_cpu_every_cycle_OneBus(BYTE nidx);
void extcl_irq_A12_clock_OneBus(BYTE nidx);
void extcl_ppu_000_to_34x_OneBus(BYTE nidx);

void init_OneBus(BYTE reset);
void prg_fix_8k_OneBus_base(WORD mmask, WORD mblock);
void prg_swap_8k_OneBus_base(WORD address, WORD value);
void prg_fix_16k_OneBus_base(WORD bank0, WORD bank1, WORD mmask, WORD mblock);
void prg_swap_16k_OneBus_base(WORD address, WORD value);
void chr_fix_OneBus_base(WORD mmask, WORD mblock);
void chr_swap_OneBus_base(BYTE **banks, BYTE *base, BYTE bit4pp, BYTE extended, WORD EVA, WORD mmask, WORD mblock);
void wram_fix_OneBus_base(WORD mmask, WORD mblock);
void mirroring_fix_OneBus_base(void);

void chr_wrap_OneBus(BYTE **banks, BYTE *base, BYTE bit4pp, BYTE extended, WORD EVA, WORD mmask, WORD mblock);

extern void (*OneBus_prg_fix_8k)(WORD mmask, WORD mblock);
extern void (*OneBus_prg_swap_8k)(WORD address, WORD value);
extern void (*OneBus_prg_fix_16k)(WORD bank0, WORD bank1, WORD mmask, WORD mblock);
extern void (*OneBus_prg_swap_16k)(WORD address, WORD value);
extern void (*OneBus_chr_fix)(WORD mmask, WORD mblock);
extern void (*OneBus_chr_swap)(BYTE **banks, BYTE *base, BYTE bit4pp, BYTE extended, WORD EVA, WORD mmask, WORD mblock);
extern void (*OneBus_wram_fix)(WORD mmask, WORD mblock);
extern void (*OneBus_mirroring_fix)(void);

#endif /* ONEBUSV_H_ */
