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

#ifndef MAPPER_LZ93D50_H_
#define MAPPER_LZ93D50_H_

#include "common.h"
#include "serial_devices_interface.h"

typedef struct _lz93d50 {
	BYTE prg;
	BYTE chr[8];
	BYTE mirroring;
	BYTE wram_enabled;
	struct _lz93d50_irq {
		BYTE enabled;
		WORD count;
		WORD reload;
		BYTE delay;
	} irq;
} _lz93d50;

extern _lz93d50 lz93d50;

void extcl_after_mapper_init_LZ93D50(void);
void extcl_cpu_wr_mem_LZ93D50(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_LZ93D50(WORD address, BYTE openbus);
BYTE extcl_save_mapper_LZ93D50(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_LZ93D50(void);

void init_LZ93D50(BYTE include_wram);
void init_eeprom_LZ93D50(heeprom_i2c *eeprom);
void prg_fix_LZ93D50_base(void);
void prg_swap_LZ93D50_base(WORD address, WORD value);
void chr_fix_LZ93D50_base(void);
void chr_swap_LZ93D50_base(WORD address, WORD value);
void wram_fix_LZ93D50_base(void);
void mirroring_fix_LZ93D50_base(void);

extern void (*LZ93D50_prg_fix)(void);
extern void (*LZ93D50_prg_swap)(WORD address, WORD value);
extern void (*LZ93D50_chr_fix)(void);
extern void (*LZ93D50_chr_swap)(WORD address, WORD value);
extern void (*LZ93D50_wram_fix)(void);
extern void (*LZ93D50_mirroring_fix)(void);

#endif /* MAPPER_LZ93D50_H_ */
