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

#ifndef ONEBUS_H_
#define ONEBUS_H_

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

void map_init_OneBus(void);
void extcl_after_mapper_init_OneBus(void);
void extcl_mapper_quit_OneBus(void);
void extcl_cpu_wr_mem_OneBus(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_OneBus(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_OneBus(BYTE mode, BYTE slot, FILE *fp);
BYTE extcl_wr_ppu_reg_OneBus(WORD address, BYTE *value);
BYTE extcl_wr_apu_OneBus(WORD address, BYTE *value);
BYTE extcl_rd_apu_OneBus(WORD address, BYTE openbus, BYTE before);
BYTE extcl_rd_chr_OneBus(WORD address);
void extcl_cpu_every_cycle_OneBus(void);
void extcl_irq_A12_clock_OneBus(void);
void extcl_ppu_000_to_34x_OneBus(void);

void prg_fix_8k_OneBus(WORD mmask, WORD mblock);
void prg_fix_16k_OneBus(WORD bank0, WORD bank1, WORD mmask, WORD mblock);
void chr_fix_OneBus(WORD mmask, WORD mblock);
void mirroring_fix_OneBus(void);

#endif /* ONEBUS_H_ */
