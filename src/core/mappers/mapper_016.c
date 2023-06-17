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

#include "mappers.h"
#include "info.h"

void prg_swap_fcg_016(WORD address, WORD value);
void chr_swap_fcg_016(WORD address, WORD value);

void prg_swap_lz93d50_016(WORD address, WORD value);
void chr_swap_lz93d50_016(WORD address, WORD value);

struct _m016tmp {
	heeprom_i2c *eeprom;
} m016tmp;

void map_init_016(void) {
	if (info.mapper.submapper == DEFAULT) {
		info.mapper.submapper = 0;
	}

	switch (info.mapper.submapper) {
		default:
			EXTCL_AFTER_MAPPER_INIT(LZ93D50);
			EXTCL_CPU_WR_MEM(LZ93D50);
			EXTCL_SAVE_MAPPER(LZ93D50);
			EXTCL_CPU_EVERY_CYCLE(LZ93D50);
			mapper.internal_struct[0] = (BYTE *)&lz93d50;
			mapper.internal_struct_size[0] = sizeof(lz93d50);

			init_LZ93D50(TRUE, info.reset);
			LZ93D50_prg_swap = prg_swap_lz93d50_016;
			LZ93D50_chr_swap = chr_swap_lz93d50_016;
			return;
		case 1:
			map_init_159();
			return;
		case 2:
			map_init_157();
			return;
		case 3:
			map_init_153();
			return;
		case 4:
			EXTCL_AFTER_MAPPER_INIT(FCG);
			EXTCL_CPU_WR_MEM(FCG);
			EXTCL_SAVE_MAPPER(FCG);
			EXTCL_CPU_EVERY_CYCLE(FCG);
			mapper.internal_struct[0] = (BYTE *)&fcg;
			mapper.internal_struct_size[0] = sizeof(fcg);

			init_FCG(info.reset);
			FCG_prg_swap = prg_swap_fcg_016;
			FCG_chr_swap = chr_swap_fcg_016;
			return;
		case 5:
			EXTCL_AFTER_MAPPER_INIT(016);
			EXTCL_MAPPER_QUIT(016);
			EXTCL_CPU_WR_MEM(LZ93D50);
			EXTCL_CPU_RD_MEM(LZ93D50);
			EXTCL_SAVE_MAPPER(LZ93D50);
			EXTCL_CPU_EVERY_CYCLE(LZ93D50);
			mapper.internal_struct[0] = (BYTE *)&lz93d50;
			mapper.internal_struct_size[0] = sizeof(lz93d50);

			init_LZ93D50(FALSE, info.reset);
			LZ93D50_prg_swap = prg_swap_lz93d50_016;
			LZ93D50_chr_swap = chr_swap_lz93d50_016;
			return;
	}
}
void extcl_after_mapper_init_016(void) {
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (info.mapper.battery) {
			m016tmp.eeprom = eeprom_24c02_create(0, wram_pnt());
			init_eeprom_LZ93D50(m016tmp.eeprom);
		}
	}
	extcl_after_mapper_init_LZ93D50();
}
void extcl_mapper_quit_016(void) {
	if (m016tmp.eeprom) {
		eeprom_i2c_free(m016tmp.eeprom);
		m016tmp.eeprom = NULL;
	}
}

void prg_swap_fcg_016(WORD address, WORD value) {
	prg_swap_FCG_base(address, (value & 0x0F));
}
void chr_swap_fcg_016(WORD address, WORD value) {
	chr_swap_FCG_base(address, (value & 0xFF));
}

void prg_swap_lz93d50_016(WORD address, WORD value) {
	prg_swap_LZ93D50_base(address, (value & 0x0F));
}
void chr_swap_lz93d50_016(WORD address, WORD value) {
	chr_swap_LZ93D50_base(address, (value & 0xFF));
}
