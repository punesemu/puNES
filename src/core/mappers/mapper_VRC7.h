/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

#ifndef MAPPER_VRC7_H_
#define MAPPER_VRC7_H_

#include "common.h"

enum { VRC7A, VRC7B };

struct _vrc7 {
	BYTE reg;
	BYTE enabled;
	BYTE reload;
	BYTE mode;
	BYTE acknowledge;
	BYTE count;
	BYTE delay;
	WORD prescaler;
} vrc7;

void map_init_VRC7(BYTE revision);
void extcl_cpu_wr_mem_VRC7(WORD address, BYTE value);
BYTE extcl_save_mapper_VRC7(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_VRC7(void);
void extcl_snd_start_VRC7(WORD samplarate);

#endif /* MAPPER_VRC7_H_ */
