/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#ifndef MAPPER_MMC2ANDMMC4_H_
#define MAPPER_MMC2ANDMMC4_H_

#include "common.h"

/* MMC4 */
enum { BAD_INES_FWJ };

void map_init_MMC2and4(void);
void extcl_cpu_wr_mem_MMC2and4(WORD address, BYTE value);
BYTE extcl_save_mapper_MMC2and4(BYTE mode, BYTE slot, FILE *fp);
void extcl_after_rd_chr_MMC2and4(WORD address);
void extcl_update_r2006_MMC2and4(WORD new_r2006, WORD old_r2006);

#endif /* MAPPER_MMC2ANDMMC4_H_ */
