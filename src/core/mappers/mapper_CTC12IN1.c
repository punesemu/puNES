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

INLINE static void prg_fix_CTC12IN1(void);
INLINE static void mirroring_fix_CTC12IN1(void);

struct _ctc12in1 {
	BYTE reg;
} ctc12in1;
struct _ctc12in1tmp {
	BYTE *prg_6000;
} ctc12in1tmp;

void map_init_CTC12IN1(void) {
	EXTCL_AFTER_MAPPER_INIT(CTC12IN1);
	EXTCL_CPU_WR_MEM(CTC12IN1);
	EXTCL_CPU_RD_MEM(CTC12IN1);
	EXTCL_SAVE_MAPPER(CTC12IN1);
	mapper.internal_struct[0] = (BYTE *)&ctc12in1;
	mapper.internal_struct_size[0] = sizeof(ctc12in1);

	memset(&ctc12in1, 0x00, sizeof(ctc12in1));

	ctc12in1tmp.prg_6000 = prg_pnt(1 << 13);
}
void extcl_after_mapper_init_CTC12IN1(void) {
	prg_fix_CTC12IN1();
	mirroring_fix_CTC12IN1();
}
void extcl_cpu_wr_mem_CTC12IN1(WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x8000:
		case 0xA000:
			ctc12in1.reg =(ctc12in1.reg & 0x07) | (value & ~0x07);
			prg_fix_CTC12IN1();
			mirroring_fix_CTC12IN1();
			break;
		case 0xC000:
		case 0xE000:
			ctc12in1.reg =(ctc12in1.reg & ~0x07) | (value & 0x07);
			prg_fix_CTC12IN1();
			mirroring_fix_CTC12IN1();
			break;
	}
}
BYTE extcl_cpu_rd_mem_CTC12IN1(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address & 0xE000) == 0x6000) {
		return (ctc12in1tmp.prg_6000[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_CTC12IN1(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, ctc12in1.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_CTC12IN1(void) {
	BYTE value = ctc12in1.reg;

	switch ((ctc12in1.reg >> 6) & 0x03) {
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
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			value |= 0x07;
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, value);
			break;
	}
	map_prg_rom_8k_update();
}
INLINE static void mirroring_fix_CTC12IN1(void) {
	if (ctc12in1.reg & 0x20) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
