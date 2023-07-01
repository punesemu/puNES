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

#ifndef MAPPER_086_H_
#define MAPPER_086_H_

#include "common.h"

typedef struct _m086 {
	BYTE reg;
	struct _m086_snd {
		BYTE speech;
		BYTE playing;
		BYTE um_rate;
		DBWORD um_sample;
		DBWORD um_count;
		SWORD out;
	} snd;
} _m086;

extern _m086 m086;

void map_init_086(void);
void extcl_after_mapper_init_086(void);
void extcl_mapper_quit_086(void);
void extcl_cpu_wr_mem_086(WORD address, BYTE value);
BYTE extcl_save_mapper_086(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_086(void);

#endif /* MAPPER_086_H_ */
