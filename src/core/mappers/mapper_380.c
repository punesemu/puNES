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

INLINE static void prg_fix_380(void);
INLINE static void chr_fix_380(void);
INLINE static void mirroring_fix_380(void);

struct _m380 {
	WORD reg;
} m380;

void map_init_380(void) {
	EXTCL_AFTER_MAPPER_INIT(380);
	EXTCL_CPU_WR_MEM(380);
	EXTCL_CPU_RD_MEM(380);
	EXTCL_SAVE_MAPPER(380);
	mapper.internal_struct[0] = (BYTE *)&m380;
	mapper.internal_struct_size[0] = sizeof(m380);

	memset(&m380, 0x00, sizeof(m380));

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_380(void) {
	prg_fix_380();
	chr_fix_380();
	mirroring_fix_380();
}
void extcl_cpu_wr_mem_380(WORD address, UNUSED(BYTE value)) {
	m380.reg = address;
	prg_fix_380();
	chr_fix_380();
	mirroring_fix_380();
}
BYTE extcl_cpu_rd_mem_380(WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		return ((info.mapper.submapper == 0) && (m380.reg & 0x100)
			? (address & 0xFF) | dipswitch.value
			: prgrom_rd(address));
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_380(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m380.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_380(void) {
	WORD bank = (m380.reg & 0x007C) >> 2;
	WORD bit0 = !(m380.reg & 0x0001);
	WORD bit8 = (m380.reg & 0x0100) >> 8;
	WORD bit9 = (m380.reg & 0x0200) >> 9;

	bank = bank & ~(bit0 * bit9);
	memmap_auto_16k(MMCPU(0x8000), bank);

	bank = bank | (bit0 * bit9) | (7 * !bit9) | (info.mapper.submapper == 1 ? 8 * !bit9 * bit8 : 0);
	memmap_auto_16k(MMCPU(0xC000), bank);
}
INLINE static void chr_fix_380(void) {
	BYTE enabled = !(m380.reg & 0x0080);

	memmap_vram_wp_8k(MMPPU(0x0000), 0, TRUE, enabled);
}
INLINE static void mirroring_fix_380(void) {
	if ((info.mapper.submapper == 2) ? m380.reg & 0x040 : m380.reg & 0x0002) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
