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

#ifndef MAPPER_394_H_
#define MAPPER_394_H_

#include "common.h"

typedef struct _m394 {
	BYTE reg[4];
} _m394;

extern _m394 m394;

void map_init_394(void);
void extcl_after_mapper_init_394(void);
void extcl_cpu_wr_mem_394(BYTE nidx, WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_394(BYTE nidx, WORD address, BYTE openbus);
BYTE extcl_save_mapper_394(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_394(BYTE nidx);
void extcl_rd_ppu_mem_394(BYTE nidx, WORD address);
BYTE extcl_rd_chr_394(BYTE nidx, WORD address);
void extcl_ppu_000_to_34x_394(BYTE nidx);
void extcl_ppu_000_to_255_394(BYTE nidx);
void extcl_ppu_256_to_319_394(BYTE nidx);
void extcl_ppu_320_to_34x_394(BYTE nidx);
void extcl_update_r2006_394(BYTE nidx, WORD new_r2006, WORD old_r2006);

#endif /* MAPPER_394_H_ */
