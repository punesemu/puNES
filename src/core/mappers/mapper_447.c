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
#include "save_slot.h"

void prg_fix_vrc2and4_447(void);
void prg_swap_vrc2and4_447(WORD address, WORD value);
void chr_swap_vrc2and4_447(WORD address, WORD value);

struct _m447 {
	BYTE reg;
} m447;

void map_init_447(void) {
	EXTCL_AFTER_MAPPER_INIT(VRC2and4);
	EXTCL_CPU_WR_MEM(447);
	EXTCL_CPU_RD_MEM(447);
	EXTCL_SAVE_MAPPER(447);
	EXTCL_CPU_EVERY_CYCLE(VRC2and4);
	mapper.internal_struct[0] = (BYTE *)&m447;
	mapper.internal_struct_size[0] = sizeof(m447);
	mapper.internal_struct[1] = (BYTE *)&vrc2and4;
	mapper.internal_struct_size[1] = sizeof(vrc2and4);

	memset(&m447, 0x00, sizeof(m447));

	init_VRC2and4(VRC24_VRC4, 0x04, 0x08, TRUE, info.reset);
	VRC2and4_prg_fix = prg_fix_vrc2and4_447;
	VRC2and4_prg_swap = prg_swap_vrc2and4_447;
	VRC2and4_chr_swap = chr_swap_vrc2and4_447;

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;
}
void extcl_cpu_wr_mem_447(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (!(m447.reg & 0x01) && memmap_adr_is_writable(MMCPU(address))) {
			m447.reg = address & 0xFF;
			VRC2and4_prg_fix();
			VRC2and4_chr_fix();
			return;
		}
	}
	extcl_cpu_wr_mem_VRC2and4(address, value);
}
BYTE extcl_cpu_rd_mem_447(WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		return (dipswitch.used && (m447.reg & 0x08)
			? prgrom_rd((address & 0xFFFC) | (dipswitch.value & 0x03))
			: prgrom_rd(address));
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_447(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m447.reg);
	return (extcl_save_mapper_VRC2and4(mode, slot, fp));
}

void prg_fix_vrc2and4_447(void) {
	if (m447.reg & 0x04) {
		WORD A14 = ~m447.reg & 0x02;
		WORD base = m447.reg << 4;
		WORD mask = 0x0F;
		WORD bank = 0;

		bank = (vrc2and4.prg[0] & ~A14);
		VRC2and4_prg_swap(0x8000, (base | (bank & mask)));
		bank = (vrc2and4.prg[1] & ~A14);
		VRC2and4_prg_swap(0xA000, (base | (bank & mask)));
		bank = (vrc2and4.prg[0] | A14);
		VRC2and4_prg_swap(0xC000, (base | (bank & mask)));
		bank = (vrc2and4.prg[1] | A14);
		VRC2and4_prg_swap(0xE000, (base | (bank & mask)));
		return;
	}
	prg_fix_VRC2and4_base();
}
void prg_swap_vrc2and4_447(WORD address, WORD value) {
	prg_swap_VRC2and4_base(address, ((m447.reg << 4) | (value & 0x0F)));
}
void chr_swap_vrc2and4_447(WORD address, WORD value) {
	chr_swap_VRC2and4_base(address, ((m447.reg << 7) | (value & 0x7F)));
}
