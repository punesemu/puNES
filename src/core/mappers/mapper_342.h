/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#ifndef MAPPER_342_H_
#define MAPPER_342_H_

#include "common.h"

void map_init_342(void);
void extcl_after_mapper_init_342(void);
void extcl_mapper_quit_342(void);
void extcl_cpu_wr_mem_342(BYTE nidx, WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_342(BYTE nidx, WORD address, BYTE openbus);
BYTE extcl_save_mapper_342(BYTE mode, BYTE slot, FILE *fp);
void extcl_battery_io_342(BYTE mode, FILE *fp);
void extcl_cpu_every_cycle_342(BYTE nidx);
void extcl_ppu_000_to_34x_342(BYTE nidx);
void extcl_after_rd_chr_342(BYTE nidx, WORD address);
void extcl_update_r2006_342(BYTE nidx, WORD new_r2006, WORD old_r2006);

#endif /* MAPPER_342_H_ */
