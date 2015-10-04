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

#ifndef MAPPER_KAISER_H_
#define MAPPER_KAISER_H_

#include "common.h"

enum {
	KS202,
	KS7032,
	KS7058,
	KS7022
};

struct _ks202 {
	BYTE enabled;
	WORD count;
	WORD reload;
	BYTE delay;
	BYTE reg;
	BYTE *prg_ram_rd;
} ks202;
struct _ks7022 {
	BYTE reg;
} ks7022;

void map_init_Kaiser(BYTE model);

void extcl_cpu_wr_mem_Kaiser_ks202(WORD address, BYTE value);
BYTE extcl_save_mapper_Kaiser_ks202(BYTE mode, BYTE slot, FILE *fp);
BYTE extcl_cpu_rd_mem_Kaiser_ks202(WORD address, BYTE openbus, BYTE before);
void extcl_cpu_every_cycle_Kaiser_ks202(void);

void extcl_cpu_wr_mem_Kaiser_ks7058(WORD address, BYTE value);

void extcl_cpu_wr_mem_Kaiser_ks7022(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_Kaiser_ks7022(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_Kaiser_ks7022(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_KAISER_H_ */
