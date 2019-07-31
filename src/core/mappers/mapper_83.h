/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#ifndef MAPPER_83_H_
#define MAPPER_83_H_

#include "common.h"

enum { MAP83_REG0, MAP83_DGP };

struct _m83 {
	BYTE is2kbank;
	BYTE isnot2kbank;
	BYTE mode;
	BYTE bank;
	BYTE dip;
	BYTE low[4];
	BYTE reg[11];

	struct _m83_irq {
		BYTE active;
		WORD count;
	} irq;
} m83;

void map_init_83(void);
void extcl_cpu_wr_mem_83(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_83(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_83(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_83(void);

#endif /* MAPPER_83_H_ */
