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
#include "cpu.h"
#include "save_slot.h"

INLINE static void prg_fix_332(void);
INLINE static void chr_fix_332(void);
INLINE static void mirroring_fix_332(void);

INLINE static void tmp_fix_332(BYTE max, BYTE index, const WORD *ds);

struct _m332 {
	BYTE reg[3];
} m332;
struct _m332tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const WORD *dipswitch;
} m332tmp;

void map_init_332(void) {
	EXTCL_AFTER_MAPPER_INIT(332);
	EXTCL_CPU_WR_MEM(332);
	EXTCL_SAVE_MAPPER(332);
	mapper.internal_struct[0] = (BYTE *)&m332;
	mapper.internal_struct_size[0] = sizeof(m332);

	memset(&m332, 0x00, sizeof(m332));

	if (info.reset == RESET) {
		if (m332tmp.ds_used) {
			m332tmp.index = (m332tmp.index + 1) % m332tmp.max;
		}
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memset(&m332tmp, 0x00, sizeof(m332tmp));

		if (info.crc32.prg == 0xB2CD52F5) { // Super 40-in-1 (Multicart WS-1001) [U][!].unf
			static WORD ds[] = { 0x00, 0x40, 0x80 };

			tmp_fix_332(LENGTH(ds), 0, &ds[0]);
		} else {
			static WORD ds[] = { 0x00 };

			tmp_fix_332(LENGTH(ds), 0, &ds[0]);
		}
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_332(void) {
	prg_fix_332();
	chr_fix_332();
	mirroring_fix_332();
}
void extcl_cpu_wr_mem_332(WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (!(m332.reg[0] & 0x20)) {
			m332.reg[address & 0x01] = value;
			prg_fix_332();
			chr_fix_332();
			mirroring_fix_332();
		}
	} else if (address >= 0x8000) {
		m332.reg[2] = value;
		chr_fix_332();
	}
}
BYTE extcl_save_mapper_332(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m332.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_332(void) {
	BYTE enabled = !((m332.reg[1] & 0xC0) & m332tmp.dipswitch[m332tmp.index]);
	WORD bank = ((m332.reg[0] & 0x40) >> 3) | (m332.reg[0] & 0x07);
	WORD nrom = !(m332.reg[0] & 0x08);

	memmap_auto_wp_16k(MMCPU(0x8000), (bank & ~nrom), enabled, enabled);
	memmap_auto_wp_16k(MMCPU(0xC000), (bank |  nrom), enabled, enabled);
}
INLINE static void chr_fix_332(void) {
	WORD base = ((m332.reg[0] & 0x40) >> 3) | (m332.reg[1] & 0x07);
	WORD mask = m332.reg[1] & 0x10 ? 0 : m332.reg[1] & 0x20 ? 1 : 3;

	memmap_auto_8k(MMPPU(0x0000), ((base & ~mask) | (m332.reg[2] & mask)));
}
INLINE static void mirroring_fix_332(void) {
	if (m332.reg[0] & 0x10) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}

INLINE static void tmp_fix_332(BYTE max, BYTE index, const WORD *ds) {
	m332tmp.ds_used = TRUE;
	m332tmp.max = max;
	m332tmp.index = index;
	m332tmp.dipswitch = ds;
}
