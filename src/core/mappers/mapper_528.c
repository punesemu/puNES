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
#include "cpu.h"
#include "mem_map.h"
#include "save_slot.h"

void prg_swap_fme7_528(WORD address, WORD value);
void chr_swap_fme7_528(WORD address, WORD value);
void wram_fix_fme7_528(void);
void wram_swap_fme7_528(WORD value);

struct _m528 {
	BYTE reg;
} m528;

void map_init_528(void) {
	EXTCL_AFTER_MAPPER_INIT(FME7);
	EXTCL_CPU_WR_MEM(528);
	EXTCL_SAVE_MAPPER(528);
	EXTCL_CPU_EVERY_CYCLE(VRC7);
	EXTCL_APU_TICK(FME7);
	mapper.internal_struct[0] = (BYTE *)&m528;
	mapper.internal_struct_size[0] = sizeof(m528);
	mapper.internal_struct[1] = (BYTE *)&fme7;
	mapper.internal_struct_size[1] = sizeof(fme7);
	mapper.internal_struct[2] = (BYTE *)&vrc7;
	mapper.internal_struct_size[2] = sizeof(vrc7);

	memset(&m528, 0x00, sizeof(m528));

	// IRQ
	init_VRC7(0x01, 0x00);
	// tutto il resto
	init_FME7();
	FME7_prg_swap = prg_swap_fme7_528;
	FME7_chr_swap = chr_swap_fme7_528;
	FME7_wram_fix = wram_fix_fme7_528;
	FME7_wram_swap = wram_swap_fme7_528;
}
void extcl_cpu_wr_mem_528(WORD address, BYTE value) {
	WORD bank = address & 0xF000;

	switch (bank) {
		case 0xA000:
		case 0xC000: {
			int index = address & 0x0F;

			switch (index) {
				case 0x0B:
					break;
				case 0x0D:
					extcl_cpu_wr_mem_VRC7(0xF000, value);
					break;
				case 0x0E:
					extcl_cpu_wr_mem_VRC7(0xF001, value);
					break;
				case 0x0F:
					extcl_cpu_wr_mem_VRC7(0xE001, value);
					break;
				default:
					extcl_cpu_wr_mem_FME7(0x8000, index);
					extcl_cpu_wr_mem_FME7(0xA000, value);
					break;
			}
			m528.reg = (bank & 0x4000) >> 10;
			FME7_prg_fix();
			FME7_chr_fix();
			FME7_wram_fix();
			return;
		}
		default:
			return;
	}
}
BYTE extcl_save_mapper_528(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m528.reg);
	extcl_save_mapper_VRC7(mode, slot, fp);
	extcl_save_mapper_FME7(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		FME7_wram_fix();
	}

	return (EXIT_OK);
}

void prg_swap_fme7_528(WORD address, WORD value) {
	WORD base = m528.reg;
	WORD mask = m528.reg | 0x0F;

	prg_swap_FME7_base(address, (base + (value & mask)));
}
void chr_swap_fme7_528(WORD address, WORD value) {
	WORD base = m528.reg << 4;
	WORD mask = 0xFF;

	chr_swap_FME7_base(address, (base | (value & mask)));
}
void wram_fix_fme7_528(void) {
	WORD base = m528.reg;
	WORD mask = m528.reg | 0x0F;

	FME7_wram_swap((base + (fme7.prg[0] & mask)));
}
void wram_swap_fme7_528(WORD value) {
	prg.ram_plus_8k = NULL;
	if (fme7.prg[0] == 0x01) {
		cpu.prg_ram_rd_active = FALSE;
		if (info.prg.ram.banks_8k_plus) {
			cpu.prg_ram_rd_active = TRUE;
			prg.ram_plus_8k = &prg.ram_plus[0];
		}
		cpu.prg_ram_wr_active = cpu.prg_ram_rd_active;
	} else {
		cpu.prg_ram_rd_active = TRUE;
		cpu.prg_ram_wr_active = FALSE;
		control_bank(info.prg.rom.max.banks_8k)
		prg.ram_plus_8k = prg_pnt(value << 13);
	}
}