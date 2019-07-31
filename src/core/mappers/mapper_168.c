/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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
#include "cpu.h"
#include "save_slot.h"

INLINE static void m168_update_chr(void);

void map_init_168(void) {
	EXTCL_CPU_WR_MEM(168);
	EXTCL_SAVE_MAPPER(168);
	EXTCL_WR_CHR(168);
	mapper.internal_struct[0] = (BYTE *) &m168;
	mapper.internal_struct_size[0] = sizeof(m168);

	{
		BYTE i;

		for (i = 0; i < 4; i++) {
			m168.chr_map[i] = m168.chr_map[i + 4] = i;
		}
	}

	map_chr_ram_extra_init(0x2000 * 8);
	m168_update_chr();
}
void extcl_cpu_wr_mem_168(WORD address, BYTE value) {
	if (address == 0xB000) {
		BYTE save = value;

		value = save >> 6;
		control_bank(info.prg.rom[0].max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k_update();

		value = (save & 0x0F) << 2;
		m168.chr_map[4] = value;
		m168.chr_map[5] = value + 1;
		m168.chr_map[6] = value + 2;
		m168.chr_map[7] = value + 3;

		m168_update_chr();
	}
}
BYTE extcl_save_mapper_168(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m168.chr_map);
	save_slot_mem(mode, slot, chr.extra.data, chr.extra.size, FALSE);

	if (mode == SAVE_SLOT_READ) {
		m168_update_chr();
	}

	return (EXIT_OK);
}
void extcl_wr_chr_168(WORD address, BYTE value) {
	BYTE i = address >> 10;

	chr.bank_1k[i][address & 0x3FF] = value;
}

INLINE static void m168_update_chr(void) {
	BYTE i;

	for (i = 0; i < 8 ; i++) {
		chr.bank_1k[i] = &chr.extra.data[m168.chr_map[i] << 10];
	}
}
