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

#ifndef MAPPER_NAMCO_H_
#define MAPPER_NAMCO_H_

#include "common.h"

enum _namco_types {
	N163,
	CHINA_ER_SAN2,
	NAMCO_HARD_WIRED_V,
	NAMCO_HARD_WIRED_H,
	MINDSEEKER
};

typedef struct _chinaersan2 {
	BYTE enable;
	BYTE ram[256];
	struct _chinaersan2_font {
		BYTE *data;
		size_t size;
	} font;
} _chinaersan2;
typedef struct _n163 {
	BYTE prg[3];
	BYTE chr[12];
	BYTE write_protect;
	struct _snd_n163 {
		BYTE enabled;
		BYTE adr;
		BYTE auto_inc;
		BYTE tick;
		BYTE channel;
		BYTE channel_start;
		SWORD output[8];
	} snd;
	struct _irq_n163 {
		BYTE delay;
		DBWORD count;
	} irq;
} _n163;

extern _chinaersan2 chinaersan2;
extern _n163 n163;

void map_init_Namco(BYTE model);
void map_init_NSF_Namco(BYTE model);

void extcl_after_mapper_init_Namco_163(void);
void extcl_mapper_quit_Namco_163(void);
void extcl_cpu_wr_mem_Namco_163(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_Namco_163(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_Namco_163(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_Namco_163(void);
void extcl_wr_chr_Namco_163(WORD address, BYTE value);
void extcl_apu_tick_Namco_163(void);
void extcl_battery_io_Namco_163(BYTE mode, FILE *fp);

void chinaersan2_apply_font(void);
BYTE chinaersan2_init(void);
void chinaersan2_quit(void);

#endif /* MAPPER_NAMCO_H_ */
