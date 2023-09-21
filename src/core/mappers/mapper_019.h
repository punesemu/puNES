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

#ifndef MAPPER_019_H_
#define MAPPER_019_H_

#include "common.h"

typedef struct _m019 {
	WORD prg[4];
	WORD chr[8];
	WORD nmt[4];
	BYTE wram_protect;
	struct _snd_m019 {
		BYTE enabled;
		BYTE adr;
		BYTE auto_inc;
		BYTE tick;
		BYTE channel;
		BYTE channel_start;
		SWORD output[8];
	} snd;
	struct _irq_m019 {
		BYTE delay;
		DBWORD count;
	} irq;
} _m019;

extern _m019 m019;

void map_init_019(void);
void map_init_NSF_N163(void);
void extcl_after_mapper_init_019(void);
void extcl_cpu_wr_mem_019(BYTE cidx, WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_019(BYTE cidx, WORD address, BYTE openbus);
BYTE extcl_save_mapper_019(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_019(BYTE cidx);
void extcl_apu_tick_019(void);
void extcl_battery_io_019(BYTE mode, FILE *fp);

#endif /* MAPPER_019_H_ */
