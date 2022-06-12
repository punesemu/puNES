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

#ifndef MAPPER_CHEAPOCABRA_H_
#define MAPPER_CHEAPOCABRA_H_

#include "common.h"

void map_init_CHEAPOCABRA(void);

void extcl_after_mapper_init_CHEAPOCABRA_GTROM(void);
void extcl_mapper_quit_CHEAPOCABRA_GTROM(void);
void extcl_cpu_wr_mem_CHEAPOCABRA_GTROM(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_CHEAPOCABRA_GTROM(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_CHEAPOCABRA_GTROM(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_CHEAPOCABRA_GTROM(void);
void extcl_wr_nmt_CHEAPOCABRA_GTROM(WORD address, BYTE value);
BYTE extcl_rd_nmt_CHEAPOCABRA_GTROM(WORD address);
void extcl_battery_io_CHEAPOCABRA_GTROM(BYTE mode, FILE *fp);

#endif /* MAPPER_CHEAPOCABRA_H_ */
