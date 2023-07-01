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

INLINE static void prg_fix_246(void);
INLINE static void chr_fix_246(void);
INLINE static void wram_fix_246(void);

struct _m246 {
	BYTE prg[4];
	BYTE chr[4];
} m246;

void map_init_246(void) {
	EXTCL_AFTER_MAPPER_INIT(246);
	EXTCL_CPU_WR_MEM(246);
	EXTCL_CPU_RD_MEM(246);
	EXTCL_SAVE_MAPPER(246);
	mapper.internal_struct[0] = (BYTE *)&m246;
	mapper.internal_struct_size[0] = sizeof(m246);

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memmap_wram_region_init(S2K);
		if (wram_nvram_size() != S2K) {
			if (info.mapper.battery) {
				wram_set_ram_size(0);
				wram_set_nvram_size(S2K);
			} else {
				wram_set_ram_size(S2K);
				wram_set_nvram_size(0);
			}
		}
	}

	if (info.reset >= HARD) {
		memset(&m246, 0x00, sizeof(m246));

		m246.prg[1] = 0x01;
		m246.prg[2] = 0xFE;
		m246.prg[3] = 0xFF;
	}

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_246(void) {
	prg_fix_246();
	chr_fix_246();
	wram_fix_246();
}
void extcl_cpu_wr_mem_246(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x67FF)) {
		if (address & 0x04) {
			m246.chr[address & 0x03] = value;
			chr_fix_246();
		} else {
			m246.prg[address & 0x03] = value;
			prg_fix_246();
		}
		return;
	}
}
BYTE extcl_cpu_rd_mem_246(WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		if ((address > 0xFF00) && (address & 0xFFE4) == 0xFFE4) {
			size_t adr = (((m246.prg[3] | 0x10) << 13) | 0x1000 | (address & 0x0FFF)) & 0x7FFFF;

			return (prgrom_byte(adr));
		}
		return (prgrom_rd(address));
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_246(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m246.prg);
	save_slot_ele(mode, slot, m246.chr);

	return (EXIT_OK);
}

INLINE static void prg_fix_246(void) {
	memmap_auto_8k(MMCPU(0x8000), m246.prg[0]);
	memmap_auto_8k(MMCPU(0xA000), m246.prg[1]);
	memmap_auto_8k(MMCPU(0xC000), m246.prg[2]);
	memmap_auto_8k(MMCPU(0xE000), m246.prg[3]);
}
INLINE static void chr_fix_246(void) {
	memmap_auto_2k(MMPPU(0x0000), m246.chr[0]);
	memmap_auto_2k(MMPPU(0x0800), m246.chr[1]);
	memmap_auto_2k(MMPPU(0x1000), m246.chr[2]);
	memmap_auto_2k(MMPPU(0x1800), m246.chr[3]);
}
INLINE static void wram_fix_246(void) {
	memmap_disable_8k(MMCPU(0x6000));
	memmap_auto_wp_2k(MMCPU(0x6800), 0, TRUE, TRUE);
}
