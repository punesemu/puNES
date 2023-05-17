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

INLINE static void prg_fix_350(void);
INLINE static void wram_fix_350(void);
INLINE static void mirroring_fix_350(void);

struct _bmc350 {
	BYTE reg[2];
} bmc350;

void map_init_350(void) {
	EXTCL_AFTER_MAPPER_INIT(350);
	EXTCL_CPU_WR_MEM(350);
	EXTCL_SAVE_MAPPER(350);
	mapper.internal_struct[0] = (BYTE *)&bmc350;
	mapper.internal_struct_size[0] = sizeof(bmc350);

	memset(&bmc350, 0x00, sizeof(bmc350));

	bmc350.reg[0] = 0x80;
}
void extcl_after_mapper_init_350(void) {
	prg_fix_350();
	wram_fix_350();
	mirroring_fix_350();
}
void extcl_cpu_wr_mem_350(WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x8000:
		case 0xA000:
			bmc350.reg[0] = value;
			prg_fix_350();
			mirroring_fix_350();
			break;
		case 0xC000:
		case 0xE000:
			bmc350.reg[1] = value;
			prg_fix_350();
			break;
	}
}
BYTE extcl_save_mapper_350(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, bmc350.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_350(void) {
	WORD bank = (bmc350.reg[0] & 0x18) | (bmc350.reg[1] & 0x07);
	WORD base = 0;

	switch ((bmc350.reg[0] >> 5) & 0x03) {
		case 0:
			memmap_auto_16k(0x8000, bank);
			memmap_auto_16k(0xC000, bank);
			return;
		case 1:
			memmap_auto_32k(0x8000, bank >> 1);
			return;
		case 2:
		case 3:
			if (bmc350.reg[0] & 0x20) {
				// Second chip only has 128 KiB
				bank &= 0x07;
			}
			base = (bmc350.reg[0] & 0x20);
			memmap_auto_16k(0x8000, (base | bank));
			memmap_auto_16k(0xC000, (base | bank | 0x07));
			return;
	}
}
INLINE static void wram_fix_350(void) {
	memmap_prgrom_8k(0x6000, 1);
}
INLINE static void mirroring_fix_350(void) {
	if (bmc350.reg[0] & 0x80) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
