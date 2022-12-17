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

#ifndef MAPPER_VRC4_H_
#define MAPPER_VRC4_H_

#include "common.h"

enum { VRC4A, VRC4B, VRC4C, VRC4D, VRC4E, VRC4UNL, VRC4M559, VRC4BMC, VRC4T230 };

typedef struct _vrc4 {
	WORD chr_rom_high_bank[8];
	BYTE chr_rom_bank[8];
	BYTE swap_mode;
	BYTE irq_enabled;
	BYTE irq_reload;
	BYTE irq_mode;
	BYTE irq_acknowledge;
	BYTE irq_count;
	WORD irq_prescaler;
} _vrc4;

extern _vrc4 vrc4;

void map_init_VRC4(BYTE revision);
void extcl_cpu_wr_mem_VRC4(WORD address, BYTE value);
BYTE extcl_save_mapper_VRC4(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_VRC4(void);

void map_init_VRC4BMC(void);
void extcl_cpu_wr_mem_VRC4BMC(WORD address, BYTE value);

void map_init_VRC4T230(void);
void extcl_after_mapper_init_VRC4T230(void);
void extcl_cpu_wr_mem_VRC4T230(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_VRC4T230(WORD address, BYTE openbus, BYTE before);

WORD address_VRC4(WORD address);

#endif /* MAPPER_VRC4_H_ */
