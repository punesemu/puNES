/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

#ifndef MAPPER_091_H_
#define MAPPER_091_H_

#include "common.h"

void map_init_091(void);
void extcl_after_mapper_init_091(void);
void extcl_cpu_wr_mem_091(BYTE nidx, WORD address, BYTE value);
BYTE extcl_save_mapper_091(BYTE mode, BYTE slot, FILE *fp);
void extcl_ppu_256_to_319_091(BYTE nidx);
void extcl_cpu_every_cycle_091(BYTE nidx);

#endif /* MAPPER_091_H_ */
