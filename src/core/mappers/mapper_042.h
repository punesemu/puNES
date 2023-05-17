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

#ifndef MAPPER_042_H_
#define MAPPER_042_H_

#include "common.h"

void map_init_042(void);

void extcl_after_mapper_init_042_s1(void);
void extcl_cpu_wr_mem_042_s1(WORD address, BYTE value);
BYTE extcl_save_mapper_042_s1(BYTE mode, BYTE slot, FILE *fp);

void extcl_after_mapper_init_042_s2(void);
void extcl_cpu_wr_mem_042_s2(WORD address, BYTE value);
BYTE extcl_save_mapper_042_s2(BYTE mode, BYTE slot, FILE *fp);

void extcl_after_mapper_init_042_s3(void);
void extcl_cpu_wr_mem_042_s3(WORD address, BYTE value);
BYTE extcl_save_mapper_042_s3(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_042_s3(void);

#endif /* MAPPER_042_H_ */
