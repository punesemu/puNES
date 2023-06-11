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

#include <string.h>
#include "mappers.h"
#include "info.h"

void prg_swap_lz93d50_159(WORD address, WORD value);
void chr_swap_lz93d50_159(WORD address, WORD value);

struct _m159tmp {
	heeprom_i2c *eeprom;
} m159tmp;

void map_init_159(void) {
	EXTCL_AFTER_MAPPER_INIT(159);
	EXTCL_MAPPER_QUIT(159);
	EXTCL_CPU_WR_MEM(LZ93D50);
	EXTCL_CPU_RD_MEM(LZ93D50);
	EXTCL_SAVE_MAPPER(LZ93D50);
	EXTCL_CPU_EVERY_CYCLE(LZ93D50);
	mapper.internal_struct[0] = (BYTE *)&lz93d50;
	mapper.internal_struct_size[0] = sizeof(lz93d50);

	init_LZ93D50(FALSE);
	LZ93D50_prg_swap = prg_swap_lz93d50_159;
	LZ93D50_chr_swap = chr_swap_lz93d50_159;
}
void extcl_after_mapper_init_159(void) {
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (info.mapper.battery) {
			m159tmp.eeprom = eeprom_24c01_create(0, wram_pnt());
			init_eeprom_LZ93D50(m159tmp.eeprom);
		}
	}
	extcl_after_mapper_init_LZ93D50();
}
void extcl_mapper_quit_159(void) {
	if (m159tmp.eeprom) {
		eeprom_i2c_free(m159tmp.eeprom);
		m159tmp.eeprom = NULL;
	}
}

void prg_swap_lz93d50_159(WORD address, WORD value) {
	prg_swap_LZ93D50_base(address, (value & 0x1F));
}
void chr_swap_lz93d50_159(WORD address, WORD value) {
	chr_swap_LZ93D50_base(address, (value & 0xFF));
}
