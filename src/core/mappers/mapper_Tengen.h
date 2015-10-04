/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

#ifndef MAPPER_TENGEN_H_
#define MAPPER_TENGEN_H_

#include "common.h"

enum { TRAMBO, T800037, NOCNTPLUS };

struct _tengen_rambo {
	BYTE prg_mode;
	BYTE chr_mode;
	BYTE reg_index;
	BYTE chr[8];
	BYTE prg[4];
	BYTE irq_mode;
	BYTE irq_delay;
	BYTE irq_prescaler;
	BYTE irq_force_clock;
} tengen_rambo;

void map_init_Tengen(BYTE model);

void extcl_cpu_wr_mem_Tengen_Rambo(WORD address, BYTE value);
BYTE extcl_save_mapper_Tengen_Rambo(BYTE mode, BYTE slot, FILE *fp);
void extcl_ppu_000_to_34x_Tengen_Rambo(void);
void extcl_update_r2006_Tengen_Rambo(WORD new_r2006, WORD old_r2006);
void extcl_irq_A12_clock_Tengen_Rambo(void);
void extcl_cpu_every_cycle_Tengen_Rambo(void);

#endif /* MAPPER_TENGEN_H_ */
