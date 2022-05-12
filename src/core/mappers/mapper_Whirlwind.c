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

INLINE static void prg_fix_Whirlwind(void);

struct _whirlwind {
	uint32_t reg;
} whirlwind;
struct _whirlwindtmp {
	WORD start;
	BYTE prg;
	BYTE chr;
	BYTE *prg_6000;
} whirlwindtmp;

void map_init_Whirlwind(void) {
	EXTCL_AFTER_MAPPER_INIT(Whirlwind);
	EXTCL_CPU_WR_MEM(Whirlwind);
	EXTCL_CPU_RD_MEM(Whirlwind);
	EXTCL_SAVE_MAPPER(Whirlwind);
	mapper.internal_struct[0] = (BYTE *)&whirlwind;
	mapper.internal_struct_size[0] = sizeof(whirlwind);

	if (mapper.write_vram) {
		info.mapper.submapper = mapper.mirroring == MIRRORING_HORIZONTAL ? M108_1 : M108_3;
	} else {
		info.mapper.submapper = chr_size() > (1024 * 16) ? M108_2 : M108_4;
	}

	switch (info.mapper.submapper) {
		default:
		case M108_1:
			whirlwindtmp.start = 0xF000;
			whirlwindtmp.prg = TRUE;
			whirlwindtmp.chr = FALSE;
			break;
		case M108_2:
			whirlwindtmp.start = 0xE000;
			whirlwindtmp.prg = TRUE;
			whirlwindtmp.chr = TRUE;
			break;
		case M108_3:
			whirlwindtmp.start = 0x8000;
			whirlwindtmp.prg = TRUE;
			whirlwindtmp.chr = FALSE;
			break;
		case M108_4:
			whirlwindtmp.start = 0x8000;
			whirlwindtmp.prg = FALSE;
			whirlwindtmp.chr = TRUE;
			break;
	}

	whirlwind.reg = 0xFF;

	if (info.reset >= HARD) {
		BYTE value = 0xFF;

		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}

	info.prg.ram.banks_8k_plus = FALSE;
}
void extcl_after_mapper_init_Whirlwind(void) {
	prg_fix_Whirlwind();
}
void extcl_cpu_wr_mem_Whirlwind(WORD address, BYTE value) {
	if (address >= whirlwindtmp.start) {
		whirlwind.reg = value;

		if (whirlwindtmp.prg) {
			prg_fix_Whirlwind();
		}

		if (whirlwindtmp.chr) {
			DBWORD bank = value & (info.mapper.submapper == M108_4 ? 0x01 : 0xFF);

			control_bank(info.chr.rom.max.banks_8k);
			bank = value << 13;
			chr.bank_1k[0] = chr_pnt(bank);
			chr.bank_1k[1] = chr_pnt(bank | 0x0400);
			chr.bank_1k[2] = chr_pnt(bank | 0x0800);
			chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
			chr.bank_1k[4] = chr_pnt(bank | 0x1000);
			chr.bank_1k[5] = chr_pnt(bank | 0x1400);
			chr.bank_1k[6] = chr_pnt(bank | 0x1800);
			chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
		}
	}
}
BYTE extcl_cpu_rd_mem_Whirlwind(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address & 0xE000) == 0x6000) {
		return (whirlwindtmp.prg_6000[address & 0x1FFF]);
	}
	return (openbus);

}
BYTE extcl_save_mapper_Whirlwind(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, whirlwind.reg);

	if (mode == SAVE_SLOT_READ) {
		if (save_slot.version < 15) {
			whirlwind.reg >>= 13;
		}
		prg_fix_Whirlwind();
	}

	return (EXIT_OK);
}

INLINE static void prg_fix_Whirlwind(void) {
	WORD value;

	value = whirlwind.reg;
	control_bank(info.prg.rom.max.banks_8k)
	whirlwindtmp.prg_6000 = prg_pnt(value << 13);
}
