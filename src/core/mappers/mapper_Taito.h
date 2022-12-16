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

#ifndef MAPPER_TAITO_H_
#define MAPPER_TAITO_H_

#include "common.h"

enum {
	TC0190FMC,
	TC0690,
	X1005A,
	X1005B,
	X1017,
	BAD_INES_FLINJ = 100,
	X1005_NO_BAT = 101
};

void map_init_Taito(BYTE model);

void extcl_cpu_wr_mem_Taito_TC0190FMC(WORD address, BYTE value);

void extcl_cpu_wr_mem_Taito_TC0690(WORD address, BYTE value);
void extcl_ppu_000_to_34x_Taito_TC0690(void);
void extcl_ppu_000_to_255_Taito_TC0690(void);
void extcl_ppu_256_to_319_Taito_TC0690(void);
void extcl_ppu_320_to_34x_Taito_TC0690(void);

void extcl_cpu_wr_mem_Taito_X1005(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_Taito_X1005(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_Taito_X1005(BYTE mode, BYTE slot, FILE *fp);
void extcl_battery_io_Taito_X1005(BYTE mode, FILE *fp);

void extcl_cpu_wr_mem_Taito_X1017(WORD address, BYTE value);
BYTE extcl_save_mapper_Taito_X1017(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_TAITO_H_ */
