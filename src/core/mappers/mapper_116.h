/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#ifndef MAPPER_116_H_
#define MAPPER_116_H_

#include "common.h"

enum {
	MAP116_TYPE_A,
	MAP116_TYPE_B,
	MAP116_TYPE_C
};

void map_init_116(void);

void extcl_cpu_wr_mem_116_type_A(WORD address, BYTE value);
BYTE extcl_save_mapper_116_type_A(BYTE mode, BYTE slot, FILE *fp);

void extcl_cpu_wr_mem_116_type_B(WORD address, BYTE value);
BYTE extcl_save_mapper_116_type_B(BYTE mode, BYTE slot, FILE *fp);
void extcl_wr_chr_116_type_B(WORD address, BYTE value);

void extcl_cpu_wr_mem_116_type_C(WORD address, BYTE value);
BYTE extcl_save_mapper_116_type_C(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_116_H_ */
