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
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_80013B(void);
INLINE static void mirroring_fix_80013B(void);

struct _bmc80013b {
	BYTE extra;
	BYTE reg[2];
} bmc80013b;

void map_init_80013B(void) {
	EXTCL_AFTER_MAPPER_INIT(80013B);
	EXTCL_CPU_WR_MEM(80013B);
	EXTCL_SAVE_MAPPER(80013B);
	mapper.internal_struct[0] = (BYTE *)&bmc80013b;
	mapper.internal_struct_size[0] = sizeof(bmc80013b);

	memset(&bmc80013b, 0x00, sizeof(bmc80013b));

	bmc80013b.extra = 0x80;
}
void extcl_after_mapper_init_80013B(void) {
	prg_fix_80013B();
	mirroring_fix_80013B();
}
void extcl_cpu_wr_mem_80013B(WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x8000:
			bmc80013b.reg[0] = value;
			prg_fix_80013B();
			mirroring_fix_80013B();
			break;
		case 0xA000:
		case 0xC000:
		case 0xE000:
			bmc80013b.reg[1] = value;
			bmc80013b.extra = !(address & 0x4000) ? 0x80 : 0x00;
			prg_fix_80013B();
			break;
	}
}
BYTE extcl_save_mapper_80013B(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, bmc80013b.reg);
	save_slot_ele(mode, slot, bmc80013b.extra);

	return (EXIT_OK);
}

INLINE static void prg_fix_80013B(void) {
	BYTE value;

	value = bmc80013b.extra | (bmc80013b.reg[1] & 0x70) | (bmc80013b.reg[0] & 0x0F);
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);
	value = bmc80013b.reg[1] & 0x7F;
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, value);
	map_prg_rom_8k_update();
}
INLINE static void mirroring_fix_80013B(void) {
	if (bmc80013b.reg[0] & 0x10) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
