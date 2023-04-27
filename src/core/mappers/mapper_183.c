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
#include "save_slot.h"

void chr_swap_vrc2and4_183(WORD address, WORD value);

INLINE static void prg_fix_183();

struct _m183 {
	BYTE reg[4];
} m183;
struct _m183tmp {
	BYTE *prg_6000;
} m183tmp;

void map_init_183(void) {
	EXTCL_AFTER_MAPPER_INIT(183);
	EXTCL_CPU_WR_MEM(183);
	EXTCL_CPU_RD_MEM(183);
	EXTCL_SAVE_MAPPER(183);
	EXTCL_CPU_EVERY_CYCLE(VRC2and4);
	mapper.internal_struct[0] = (BYTE *)&m183;
	mapper.internal_struct_size[0] = sizeof(m183);
	mapper.internal_struct[1] = (BYTE *)&vrc2and4;
	mapper.internal_struct_size[1] = sizeof(vrc2and4);

	if (info.reset >= HARD) {
		memset(&m183, 0x00, sizeof(m183));
	}

	init_VRC2and4(VRC24_VRC4, 0x04, 0x08, TRUE);
	VRC2and4_chr_swap = chr_swap_vrc2and4_183;

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_183(void) {
	prg_fix_183();
	VRC2and4_chr_fix();
	VRC2and4_wram_fix();
	VRC2and4_mirroring_fix();
}
void extcl_cpu_wr_mem_183(WORD address, BYTE value) {
	switch (address & 0xF800) {
		case 0x6800:
			m183.reg[0] = address & 0x3F;
			prg_fix_183();
			return;
		case 0x8800:
			m183.reg[1] = value;
			prg_fix_183();
			return;
		case 0x9800:
			extcl_cpu_wr_mem_VRC2and4(0x9000, value);
			return;
		case 0xA800:
			m183.reg[2] = value;
			prg_fix_183();
			return;
		case 0xA000:
			m183.reg[3] = value;
			prg_fix_183();
			return;
		case 0x6000:
		case 0x7000:
		case 0x7800:
		case 0x8000:
		case 0x9000:
			return;
		default:
			extcl_cpu_wr_mem_VRC2and4(address, value);
			return;
	}
}
BYTE extcl_cpu_rd_mem_183(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (m183tmp.prg_6000[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_183(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m183.reg);
	extcl_save_mapper_VRC2and4(mode, slot, fp);

	return (EXIT_OK);
}

void chr_swap_vrc2and4_183(WORD address, WORD value) {
	chr_swap_VRC2and4_base(address, (value & 0x1FF));
}

INLINE static void prg_fix_183(void) {
	WORD bank = 0;

	bank = m183.reg[0];
	_control_bank(bank, info.prg.rom.max.banks_8k)
	m183tmp.prg_6000 = prg_pnt(bank << 13);

	bank = m183.reg[1];
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0, bank);

	bank = m183.reg[2];
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, bank);

	bank = m183.reg[3];
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2, bank);

	bank = 0xFF;
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, bank);

	map_prg_rom_8k_update();
}