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

#ifndef MAPPER_60_H_
#define MAPPER_60_H_

#include "common.h"

enum { MAP60, MAP60_VT5201 };

void map_init_60(void);
void extcl_cpu_wr_mem_60(WORD address, BYTE value);
BYTE extcl_save_mapper_60(BYTE mode, BYTE slot, FILE *fp);

void map_init_60_vt5201(void);
void extcl_cpu_wr_mem_60_vt5201(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_60_vt5201(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_60_vt5201(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_60_H_ */
