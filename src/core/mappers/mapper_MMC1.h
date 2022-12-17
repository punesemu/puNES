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

#ifndef MAPPER_MMC1_H_
#define MAPPER_MMC1_H_

#include "common.h"

enum MMC1_types {
	SNROM,
	SOROM,
	SUROM,
	SXROM,
	SEROM = 5,
	SKROM = 6,
	SJROM = 7,
	FARIDSLROM = 8,
	MAP111 = 9,
	MAP297 = 10,
	MAP374 = 11,
	MAP404 = 12,
	BAD_YOSHI_U = 20,
	MOWPC10
};

typedef struct _mmc1 {
	BYTE reg;
	BYTE pos;
	BYTE prg_mode;
	BYTE chr_mode;
	BYTE ctrl;
	BYTE chr0;
	BYTE chr1;
	BYTE prg0;
	BYTE reset;
	BYTE prg_upper;
	BYTE chr_upper;

	// da non salvare
	BYTE prg_mask;
} _mmc1;

extern _mmc1 mmc1;

void map_init_MMC1(void);
void extcl_cpu_wr_mem_MMC1(WORD address, BYTE value);
BYTE extcl_cpu_rd_ram_MMC1(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_MMC1(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_MMC1_H_ */
