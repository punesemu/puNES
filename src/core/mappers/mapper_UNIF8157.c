/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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
#include "mem_map.h"
#include "save_slot.h"

INLINE static void unif8157_update(BYTE value);

struct _unif8157 {
	WORD reg;
} unif8157;
struct _unif8157tmp {
	BYTE reset;
} unif8157tmp;

void map_init_UNIF8157(void) {
	EXTCL_CPU_WR_MEM(UNIF8157);
	EXTCL_CPU_RD_MEM(UNIF8157);
	EXTCL_SAVE_MAPPER(UNIF8157);
	mapper.internal_struct[0] = (BYTE *)&unif8157;
	mapper.internal_struct_size[0] = sizeof(unif8157);

	memset(&unif8157, 0x00, sizeof(unif8157));

	if (info.reset >= HARD) {
		unif8157tmp.reset = 0;
	} else if (info.reset == RESET) {
		unif8157tmp.reset++;
		unif8157tmp.reset = unif8157tmp.reset & 0x01F;
	}

	info.mapper.extend_rd = TRUE;

	unif8157_update(0);
}
void extcl_cpu_wr_mem_UNIF8157(WORD address, BYTE value) {
	unif8157.reg = address;
	unif8157_update(value);
}
BYTE extcl_cpu_rd_mem_UNIF8157(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (address >= 0x8000) {
		if ((unif8157.reg & 0x0100) && (prg.chip[0].size < (1024 * 1024))) {
			address = (address & 0xFFF0) + unif8157tmp.reset;
			return (prg_rom_rd(address));
		}
	}
	return (openbus);
}
BYTE extcl_save_mapper_UNIF8157(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, unif8157.reg);

	return (EXIT_OK);
}

INLINE static void unif8157_update(BYTE value) {
	BYTE base = ((unif8157.reg & 0x0060) | ((unif8157.reg & 0x0100) >> 1)) >> 2;
	BYTE bank = (unif8157.reg & 0x001C) >> 2;
	BYTE lbank = (unif8157.reg & 0x0200) ? 7 : ((unif8157.reg & 0x80) ? bank : 0);

	value = base | bank;
	control_bank(info.prg.rom[0].max.banks_16k)
 	map_prg_rom_8k(2, 0, value);

	value = base | lbank;
	control_bank(info.prg.rom[0].max.banks_16k)
	map_prg_rom_8k(2, 2, value);

	map_prg_rom_8k_update();

	if (unif8157.reg & 0x02) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
