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
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

struct _m186 {
	BYTE *prg_ram_bank2;
} m186;

void map_init_186(void) {
	EXTCL_CPU_WR_MEM(186);
	EXTCL_CPU_RD_MEM(186);
	EXTCL_SAVE_MAPPER(186);
	mapper.internal_struct[0] = (BYTE *)&m186;
	mapper.internal_struct_size[0] = sizeof(m186);

	info.mapper.extend_wr = TRUE;
	info.prg.ram.banks_8k_plus = 0;
	cpu.prg_ram_wr_active = TRUE;
	cpu.prg_ram_rd_active = TRUE;

	if (info.reset >= HARD) {
		memset(&m186, 0x00, sizeof(m186));
		m186.prg_ram_bank2 = prg_rom();
		map_prg_rom_8k(2, 0, 0);
		map_prg_rom_8k(2, 2, 0);
	}
}
void extcl_cpu_wr_mem_186(WORD address, BYTE value) {
	if ((address < 0x4200) || (address > 0x4EFF)) {
		return;
	}

	if (address > 0x43FF) {
		prg.ram.data[address & 0x0BFF] = value;
		return;
	}

	switch (address & 0x0001) {
		case 0x0000:
			value >>= 6;
			control_bank(info.prg.rom.max.banks_8k)
			m186.prg_ram_bank2 = prg_pnt(value << 13);
			return;
		case 0x0001:
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k_update();
			return;
	}
}
BYTE extcl_cpu_rd_mem_186(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address < 0x4200) || (address > 0x7FFF)) {
		return (openbus);
	}

	switch (address) {
		case 0x4200:
		case 0x4201:
		case 0x4203:
			return (0x00);
		case 0x4202:
			return (0x40);
	}

	if (address < 0x4400) {
		return (0xFF);
	}

	if (address < 0x4F00) {
		return (prg.ram.data[address & 0x1FFF]);
	}

	/* mi mancano informazioni per far funzionare questa mapper */
	if (address > 0x5FFF) {
		return (m186.prg_ram_bank2[address & 0x1FFF]);
	}

	return (openbus);
}
BYTE extcl_save_mapper_186(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_pos(mode, slot, prg_rom(), m186.prg_ram_bank2);
	return (EXIT_OK);
}
