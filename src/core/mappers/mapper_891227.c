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
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_891227(void);
INLINE static void mirroring_fix_BMCFK23C(void);

struct _bmc891227 {
	BYTE reg[2];
} bmc891227;
struct _bmc891227tmp {
	BYTE *prg_6000;
} bmc891227tmp;

void map_init_891227(void) {
	EXTCL_AFTER_MAPPER_INIT(891227);
	EXTCL_CPU_WR_MEM(891227);
	EXTCL_CPU_RD_MEM(891227);
	EXTCL_SAVE_MAPPER(891227);
	mapper.internal_struct[0] = (BYTE *)&bmc891227;
	mapper.internal_struct_size[0] = sizeof(bmc891227);

	memset(&bmc891227, 0x00, sizeof(bmc891227));

	bmc891227.reg[0] = 0x80;

	bmc891227tmp.prg_6000 = prg_pnt(1 << 13);
}
void extcl_after_mapper_init_891227(void) {
	prg_fix_891227();
	mirroring_fix_BMCFK23C();
}
void extcl_cpu_wr_mem_891227(WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x8000:
		case 0xA000:
			bmc891227.reg[0] = value;
			prg_fix_891227();
			mirroring_fix_BMCFK23C();
			break;
		case 0xC000:
		case 0xE000:
			bmc891227.reg[1] = value;
			prg_fix_891227();
			break;
	}
}
BYTE extcl_cpu_rd_mem_891227(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address & 0xE000) == 0x6000) {
		return (bmc891227tmp.prg_6000[address & 0x1FFF]);
	}
    return (openbus);
}
BYTE extcl_save_mapper_891227(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, bmc891227.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_891227(void) {
	BYTE value = (bmc891227.reg[0] & 0x18) | (bmc891227.reg[1] & 0x07);

	switch ((bmc891227.reg[0] >> 5) & 0x03) {
		case 0:
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k(2, 2, value);
			break;
		case 1:
			value >>= 1;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			break;
		case 2:
		case 3:
			if (bmc891227.reg[0] & 0x20) {
				// Second chip only has 128 KiB
				value &= 0x07;
			}
			value |= (bmc891227.reg[0] & 0x20);
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			value |= 0x07;
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, value);
			break;
	}
	map_prg_rom_8k_update();
}
INLINE static void mirroring_fix_BMCFK23C(void) {
	if (bmc891227.reg[0] & 0x80) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
