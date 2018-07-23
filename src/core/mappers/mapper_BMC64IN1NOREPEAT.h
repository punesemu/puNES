/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#ifndef MAPPER_BMC64IN1NOREPEAT_H_
#define MAPPER_BMC64IN1NOREPEAT_H_

#include "common.h"

struct _bmc64in1norepeat {
	BYTE reg[4];
} bmc64in1norepeat;

void map_init_BMC64IN1NOREPEAT(void);
void extcl_cpu_wr_mem_BMC64IN1NOREPEAT(WORD address, BYTE value);
BYTE extcl_save_mapper_BMC64IN1NOREPEAT(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_BMC64IN1NOREPEAT_H_ */
