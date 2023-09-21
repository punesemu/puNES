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

#ifndef MAPPER_018_H_
#define MAPPER_018_H_

#include "common.h"

typedef struct _m018 {
	WORD prg[4];
	WORD chr[8];
	BYTE mirroring;
	struct _m018_snd {
		BYTE speech;
		BYTE playing;
		SWORD out;
	} snd;
	struct _m018_irq {
		BYTE enabled;
		WORD reload;
		WORD count;
		BYTE delay;
	} irq;
} _m018;

extern _m018 m018;

void map_init_018(void);
void extcl_after_mapper_init_018(void);
void extcl_mapper_quit_018(void);
void extcl_cpu_wr_mem_018(BYTE cidx, WORD address, BYTE value);
BYTE extcl_save_mapper_018(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_018(BYTE cidx);

#endif /* MAPPER_018_H_ */
