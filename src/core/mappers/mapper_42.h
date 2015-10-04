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

#ifndef MAPPER_42_H_
#define MAPPER_42_H_

#include "common.h"

struct _m42 {
	WORD rom_map_to;
	BYTE *prg_8k_6000;
	struct _m42_irq {
		BYTE active;
		uint32_t count;
	} irq;
} m42;

void map_init_42(void);
void extcl_cpu_wr_mem_42(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_42(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_42(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_42(void);

#endif /* MAPPER_42_H_ */
