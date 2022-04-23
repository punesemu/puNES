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

INLINE static void ks7031_init_prg(void);
INLINE static void ks7031_update(void);

struct _ks7031 {
	BYTE reg[4];
} ks7031;
struct _ks7031tmp {
	BYTE *prg_6000;
	BYTE *prg_6800;
	BYTE *prg_7000;
	BYTE *prg_7800;
	BYTE *prg_8000;
	BYTE *prg_8800;
	BYTE *prg_9000;
	BYTE *prg_9800;
	BYTE *prg_A000;
	BYTE *prg_A800;
	BYTE *prg_B000;
	BYTE *prg_B800;
	BYTE *prg_C000;
	BYTE *prg_C800;
	BYTE *prg_D000;
	BYTE *prg_D800;
	BYTE *prg_E000;
	BYTE *prg_E800;
	BYTE *prg_F000;
	BYTE *prg_F800;
} ks7031tmp;

void map_init_KS7031(void) {
	EXTCL_CPU_WR_MEM(KS7031);
	EXTCL_CPU_RD_MEM(KS7031);
	EXTCL_SAVE_MAPPER(KS7031);
	mapper.internal_struct[0] = (BYTE *)&ks7031;
	mapper.internal_struct_size[0] = sizeof(ks7031);

	memset(&ks7031, 0x00, sizeof(ks7031));

	info.mapper.extend_rd = TRUE;

	ks7031_init_prg();
	ks7031_update();

	mirroring_V();
}
void extcl_cpu_wr_mem_KS7031(WORD address, BYTE value) {
	ks7031.reg[(address & 0x1800) >> 11] = value;
	ks7031_update();
}
BYTE extcl_cpu_rd_mem_KS7031(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xF800) {
		case 0x6000:
			return (ks7031tmp.prg_6000[address & 0x07FF]);
		case 0x6800:
			return (ks7031tmp.prg_6800[address & 0x07FF]);
		case 0x7000:
			return (ks7031tmp.prg_7000[address & 0x07FF]);
		case 0x7800:
			return (ks7031tmp.prg_7800[address & 0x07FF]);
		case 0x8000:
			return (ks7031tmp.prg_8000[address & 0x07FF]);
		case 0x8800:
			return (ks7031tmp.prg_8800[address & 0x07FF]);
		case 0x9000:
			return (ks7031tmp.prg_9000[address & 0x07FF]);
		case 0x9800:
			return (ks7031tmp.prg_9800[address & 0x07FF]);
		case 0xA000:
			return (ks7031tmp.prg_A000[address & 0x07FF]);
		case 0xA800:
			return (ks7031tmp.prg_A800[address & 0x07FF]);
		case 0xB000:
			return (ks7031tmp.prg_B000[address & 0x07FF]);
		case 0xB800:
			return (ks7031tmp.prg_B800[address & 0x07FF]);
		case 0xC000:
			return (ks7031tmp.prg_C000[address & 0x07FF]);
		case 0xC800:
			return (ks7031tmp.prg_C800[address & 0x07FF]);
		case 0xD000:
			return (ks7031tmp.prg_D000[address & 0x07FF]);
		case 0xD800:
			return (ks7031tmp.prg_D800[address & 0x07FF]);
		case 0xE000:
			return (ks7031tmp.prg_E000[address & 0x07FF]);
		case 0xE800:
			return (ks7031tmp.prg_E800[address & 0x07FF]);
		case 0xF000:
			return (ks7031tmp.prg_F000[address & 0x07FF]);
		case 0xF800:
			return (ks7031tmp.prg_F800[address & 0x07FF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_KS7031(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, ks7031.reg);

	if (mode == SAVE_SLOT_READ) {
		ks7031_init_prg();
		ks7031_update();
	}

	return (EXIT_OK);
}

INLINE static void ks7031_init_prg(void) {
	BYTE value;

	// 0x8000
	value = 15;
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_8000 = prg_pnt(value << 11);
	// 0x8800
	value = 14;
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_8800 = prg_pnt(value << 11);

	// 0x9000
	value = 13;
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_9000 = prg_pnt(value << 11);
	// 0x9800
	value = 12;
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_9800 = prg_pnt(value << 11);

	// 0xA000
	value = 11;
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_A000 = prg_pnt(value << 11);
	// 0xA800
	value = 10;
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_A800 = prg_pnt(value << 11);

	// 0xB000
	value = 9;
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_B000 = prg_pnt(value << 11);
	// 0xB800
	value = 8;
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_B800 = prg_pnt(value << 11);

	// 0xC000
	value = 7;
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_C000 = prg_pnt(value << 11);
	// 0xC800
	value = 6;
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_C800 = prg_pnt(value << 11);

	// 0xD000
	value = 5;
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_D000 = prg_pnt(value << 11);
	// 0xD800
	value = 4;
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_D800 = prg_pnt(value << 11);

	// 0xE000
	value = 3;
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_E000 = prg_pnt(value << 11);
	// 0xE800
	value = 2;
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_E800 = prg_pnt(value << 11);

	// 0xF000
	value = 1;
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_F000 = prg_pnt(value << 11);
	// 0xF800
	value = 0;
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_F800 = prg_pnt(value << 11);
}
INLINE static void ks7031_update(void) {
	WORD value;

	// 0x6000
	value = ks7031.reg[0];
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_6000 = prg_pnt(value << 11);
	// 0x6800
	value = ks7031.reg[1];
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_6800 = prg_pnt(value << 11);

	// 0x7000
	value = ks7031.reg[2];
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_7000 = prg_pnt(value << 11);
	// 0x7800
	value = ks7031.reg[3];
	control_bank(info.prg.rom.max.banks_2k)
	ks7031tmp.prg_7800 = prg_pnt(value << 11);
}
