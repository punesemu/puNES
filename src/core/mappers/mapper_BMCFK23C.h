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

#ifndef MAPPER_BMCFK23C_H_
#define MAPPER_BMCFK23C_H_

#include "common.h"

enum bmcfk23c_types { LP8002KB = 0, BMCFK23C = 1, FS005 = 2, JX9003B = 3, HST162 = 4 };

void map_init_BMCFK23C(void);
void extcl_after_mapper_init_BMCFK23C(void);
void extcl_cpu_wr_mem_BMCFK23C(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_BMCFK23C(WORD address, BYTE openbus, BYTE before);
void extcl_wr_chr_BMCFK23C(WORD address, BYTE value);
BYTE extcl_save_mapper_BMCFK23C(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_BMCFK23C_H_ */
