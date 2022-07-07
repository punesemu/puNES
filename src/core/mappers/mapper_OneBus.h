/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#ifndef MAPPER_ONEBUS_H_
#define MAPPER_ONEBUS_H_

#include "common.h"

void map_init_OneBus(void);
void extcl_cpu_wr_mem_OneBus(WORD address, BYTE value);
BYTE extcl_save_mapper_OneBus(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_OneBus(void);
BYTE extcl_wr_ppu_reg_OneBus(WORD address, BYTE *value);
BYTE extcl_wr_apu_OneBus(WORD address, BYTE *value);
BYTE extcl_rd_apu_OneBus(WORD address, BYTE openbus, BYTE before);
void extcl_irq_A12_clock_OneBus(void);

#endif /* MAPPER_ONEBUS_H_ */
