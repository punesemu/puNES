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

INLINE static void prg_fix_077(void);
INLINE static void chr_fix_077(void);
INLINE static void mirroring_fix_077(void);

struct m077 {
	BYTE reg;
} m077;

void map_init_077() {
	EXTCL_AFTER_MAPPER_INIT(077);
	EXTCL_CPU_WR_MEM(077);
	EXTCL_SAVE_MAPPER(077);
	mapper.internal_struct[0] = (BYTE *)&m077;
	mapper.internal_struct_size[0] = sizeof(m077);

	if (info.reset >= HARD) {
		memset(&m077, 0x00, sizeof(m077));
	}
}
void extcl_after_mapper_init_077(void) {
	prg_fix_077();
	chr_fix_077();
	mirroring_fix_077();
}
void extcl_cpu_wr_mem_077(WORD address, BYTE value) {
	// bus conflict
	m077.reg = value & prgrom_rd(address);
	prg_fix_077();
	chr_fix_077();
}
BYTE extcl_save_mapper_077(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m077.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_077(void) {
	memmap_auto_32k(MMCPU(0x8000), (m077.reg & 0x0F));
}
INLINE static void chr_fix_077(void) {
	memmap_chrrom_2k(MMPPU(0x0000), (m077.reg >> 4));
	memmap_vram_2k(MMPPU(0x0800), 0);
	memmap_vram_2k(MMPPU(0x1000), 1);
	memmap_vram_2k(MMPPU(0x1800), 2);
}
INLINE static void mirroring_fix_077(void) {
	memmap_vram_2k(MMCPU(0x2000), 3);
	memmap_vram_2k(MMCPU(0x3000), 3);

	memmap_vram_1k(MMCPU(0x2800), 0);
	memmap_vram_1k(MMCPU(0x3800), 0);

	memmap_vram_1k(MMCPU(0x2C00), 1);
	memmap_vram_1k(MMCPU(0x3C00), 1);
}

