/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

#include "mappers.h"
#include "save_slot.h"
#include "EE93Cx6.h"

void prg_fix_vrc2and4_529(void);
void chr_swap_vrc2and4_529(WORD address, WORD value);
void wram_fix_vrc2and4_529(void);

struct _m529tmp {
	BYTE cc93c56;
} m529tmp;

void map_init_529(void) {
	EXTCL_AFTER_MAPPER_INIT(VRC2and4);
	EXTCL_CPU_INIT_PC(529);
	EXTCL_CPU_WR_MEM(529);
	EXTCL_CPU_RD_MEM(529);
	EXTCL_SAVE_MAPPER(VRC2and4);
	EXTCL_CPU_EVERY_CYCLE(VRC2and4);
	map_internal_struct_init((BYTE *)&vrc2and4, sizeof(vrc2and4));

	init_VRC2and4(VRC24_VRC4, 0x04, 0x08, TRUE, info.reset);
	VRC2and4_prg_fix = prg_fix_vrc2and4_529;
	VRC2and4_chr_swap = chr_swap_vrc2and4_529;
	VRC2and4_wram_fix = wram_fix_vrc2and4_529;

	m529tmp.cc93c56 = wram_nvram_size() == S256B;
}
void extcl_cpu_init_pc_529(UNUSED(BYTE nidx)) {
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (m529tmp.cc93c56) {
			ee93cx6_init(wram_nvram_pnt(), wram_nvram_size(), 8);
		}
	}
}
void extcl_cpu_wr_mem_529(BYTE nidx, WORD address, BYTE value) {
	if ((address & 0x0800) && m529tmp.cc93c56) {
		// D~[.... .ECD]
		//          ||+- Serial Data Input to 93C56 EEPROM
		//          |+-- Serial Clock to 93C56 EEPROM
		//          +--- Chip Select to 93C56 EEPROM
		ee93cx6_write((value & 0x04) >> 2, (value & 0x02) >> 1, value & 0x01);
		return;
	}
	extcl_cpu_wr_mem_VRC2and4(nidx, address, value);
}
BYTE extcl_cpu_rd_mem_529(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	switch (address & 0xF000) {
		case 0x5000:
			return (m529tmp.cc93c56 ? ee93cx6_read() ? 0x01 : 0x00 : 0x01);
		default:
			return (wram_rd(nidx, address));
	}
}

void prg_fix_vrc2and4_529(void) {
	memmap_auto_16k(0, MMCPU(0x8000), vrc2and4.prg[1]);
	memmap_auto_16k(0, MMCPU(0xC000), 0xFF);
}
void chr_swap_vrc2and4_529(WORD address, WORD value) {
	chr_swap_VRC2and4_base(address, (value & 0x1FF));
}
void wram_fix_vrc2and4_529(void) {
	if (m529tmp.cc93c56) {
		if (wram_ram_size()) {
			memmap_wram_ram_wp_8k(0, MMCPU(0x6000), 0, !vrc2and4.wram_protect, !vrc2and4.wram_protect);
		} else {
			memmap_disable_8k(0, MMCPU(0x6000));
		}
	} else {
		wram_fix_VRC2and4_base();
	}
}
