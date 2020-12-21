/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#ifndef MAPPER_SUNSOFT_H_
#define MAPPER_SUNSOFT_H_

#include "common.h"

enum sunsoft_types {
	SUN1,
	SUN2A,
	SUN2B,
	SUN3,
	SUN4,
	FM7,
	MAHARAJA,
	BARCODEWORLD,
	DODGEDANPEI2
};

typedef struct _square_fm7 {
	BYTE disable;
	BYTE step;
	WORD frequency;
	WORD timer;
	WORD volume;
	SWORD output;
} _square_fm7;
typedef struct _sunsoft_fm7 {
	BYTE address;
	BYTE prg_ram_enable;
	BYTE prg_ram_mode;
	uint32_t prg_ram_address;
	BYTE irq_enable_trig;
	BYTE irq_enable_count;
	WORD irq_count;
	BYTE irq_delay;
	BYTE snd_reg;
	_square_fm7 square[3];

	/* ------------------------------------------------------- */
	/* questi valori non e' necessario salvarli nei savestates */
	/* ------------------------------------------------------- */
	/* */ BYTE clocked;                                     /* */
	/* ------------------------------------------------------- */
} _sunsoft_fm7;

extern _sunsoft_fm7 fm7;

void map_init_Sunsoft(BYTE model);
void map_init_NSF_Sunsoft(BYTE model);

void extcl_cpu_wr_mem_Sunsoft_S1(WORD address, BYTE value);

void extcl_cpu_wr_mem_Sunsoft_S2(WORD address, BYTE value);

void extcl_cpu_wr_mem_Sunsoft_S3(WORD address, BYTE value);
BYTE extcl_save_mapper_Sunsoft_S3(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_Sunsoft_S3(void);

void extcl_cpu_wr_mem_Sunsoft_S4(WORD address, BYTE value);
BYTE extcl_save_mapper_Sunsoft_S4(BYTE mode, BYTE slot, FILE *fp);

void extcl_cpu_wr_mem_Sunsoft_FM7(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_Sunsoft_FM7(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_Sunsoft_FM7(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_Sunsoft_FM7(void);
void extcl_apu_tick_Sunsoft_FM7(void);

#endif /* MAPPER_SUNSOFT_H_ */
