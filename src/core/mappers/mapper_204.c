/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

INLINE static void prg_fix_204(void);
INLINE static void chr_fix_204(void);
INLINE static void mirroring_fix_204(void);

struct _m204 {
	WORD reg;
} m204;

void map_init_204(void) {
	EXTCL_AFTER_MAPPER_INIT(204);
	EXTCL_CPU_WR_MEM(204);
	EXTCL_CPU_RD_MEM(204);
	EXTCL_SAVE_MAPPER(204);

	if (info.reset >= HARD) {
		memset(&m204, 0x00, sizeof(m204));
	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_204(void) {
	prg_fix_204();
	chr_fix_204();
	mirroring_fix_204();
}
void extcl_cpu_wr_mem_204(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	m204.reg = address;
	prg_fix_204();
	chr_fix_204();
	mirroring_fix_204();
}
BYTE extcl_cpu_rd_mem_204(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		switch (m204.reg & 0xFF0F) {
			case 0xF004:
				return (prgrom_size() <= S64K ? dipswitch.value & 0x00FF : prgrom_rd(nidx, address));
			case 0xF008:
				return ((dipswitch.value & 0xFF00) >> 8);
			default:
				return (prgrom_rd(nidx, address));
		}
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_204(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m204.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_204(void) {
	if (m204.reg & 0x20) {
		memmap_auto_32k(0, MMCPU(0x8000), (m204.reg >> 1));
	} else {
		memmap_auto_16k(0, MMCPU(0x8000), m204.reg);
		memmap_auto_16k(0, MMCPU(0xC000), m204.reg);
	}
}
INLINE static void chr_fix_204(void) {
	memmap_auto_8k(0, MMPPU(0x0000), m204.reg & (m204.reg & 0x20 ? 0x0E : 0x0F));
}
INLINE static void mirroring_fix_204(void) {
	if (m204.reg & 0x10) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
