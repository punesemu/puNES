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

#ifndef MAPPER_IREM_H_
#define MAPPER_IREM_H_

#include "common.h"

enum {
	G101,
	G101A,
	G101B,
	H3000,
	LROG017,
	TAMS1,
	MAJORLEAGUE
};

void map_init_Irem(BYTE model);

void extcl_cpu_wr_mem_Irem_G101(WORD address, BYTE value);
BYTE extcl_save_mapper_Irem_G101(BYTE mode, BYTE slot, FILE *fp);

void extcl_cpu_wr_mem_Irem_H3000(WORD address, BYTE value);
BYTE extcl_save_mapper_Irem_H3000(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_Irem_H3000(void);

void extcl_cpu_wr_mem_Irem_LROG017(WORD address, BYTE value);
BYTE extcl_save_mapper_Irem_LROG017(BYTE mode, BYTE slot, FILE *fp);
void extcl_wr_chr_Irem_LROG017(WORD address, BYTE value);

void extcl_cpu_wr_mem_Irem_TAMS1(WORD address, BYTE value);

#endif /* MAPPER_IREM_H_ */
