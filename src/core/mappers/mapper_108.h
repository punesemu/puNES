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

#ifndef MAPPER_108_H_
#define MAPPER_108_H_

#include "common.h"

enum mm14_types { M108_1 = 1, M108_2, M108_3, M108_4 };

void map_init_108(void);
void extcl_after_mapper_init_108(void);
void extcl_cpu_wr_mem_108(WORD address, BYTE value);
BYTE extcl_save_mapper_108(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_108_H_ */