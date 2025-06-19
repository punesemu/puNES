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

#include <string.h>
#include "mappers.h"
#include "save_slot.h"

INLINE static void prg_fix_288(void);
INLINE static void chr_fix_288(void);

struct _m288 {
	WORD reg;
} m288;

void map_init_288(void) {
	EXTCL_AFTER_MAPPER_INIT(288);
	EXTCL_CPU_WR_MEM(288);
	EXTCL_CPU_RD_MEM(288);
	EXTCL_SAVE_MAPPER(288);
	map_internal_struct_init((BYTE *)&m288, sizeof(m288));

	if (info.reset >= HARD) {
		memset(&m288, 0x00, sizeof(m288));
	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_288(void) {
	prg_fix_288();
	chr_fix_288();
}
void extcl_cpu_wr_mem_288(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	m288.reg = address;
	prg_fix_288();
	chr_fix_288();
}
BYTE extcl_cpu_rd_mem_288(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		return (m288.reg & 0x0020 ? prgrom_rd(nidx, (address | dipswitch.value)) : prgrom_rd(nidx, address));
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_288(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m288.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_288(void) {
	memmap_auto_32k(0, MMCPU(0x8000), (m288.reg >> 3));
}
INLINE static void chr_fix_288(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m288.reg & 0x07));
}
