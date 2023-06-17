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

INLINE static void prg_fix_057(void);
INLINE static void chr_fix_057(void);
INLINE static void mirroring_fix_057(void);

INLINE static void tmp_fix_057(BYTE max, BYTE index, const WORD *ds);

struct _m057 {
	BYTE reg[2];
} m057;
struct _m057tmp {
	BYTE ds_used;
	BYTE max;
	BYTE index;
	const WORD *dipswitch;
} m057tmp;

void map_init_057(void) {
	EXTCL_AFTER_MAPPER_INIT(057);
	EXTCL_CPU_WR_MEM(057);
	EXTCL_CPU_RD_MEM(057);
	EXTCL_SAVE_MAPPER(057);
	mapper.internal_struct[0] = (BYTE *)&m057;
	mapper.internal_struct_size[0] = sizeof(m057);

	memset(&m057, 0x00, sizeof(m057));

	if (info.reset == RESET) {
		if (m057tmp.ds_used) {
			m057tmp.index = (m057tmp.index + 1) % m057tmp.max;
		}
	} else if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memset(&m057tmp, 0x00, sizeof(m057tmp));

		if ((info.crc32.prg == 0xF77A2663) || // 4-in-1 (ES-Q803B_20210617) (Unl) [p1].nes
			(info.crc32.prg == 0xDB6228A0) || // 4-in-1_YH-4132.nes
			(info.crc32.prg == 0x71B7EC3A) || // (YH-4131) Exciting Sport Turbo 4-in-1.nes
			(info.crc32.prg == 0xEE722DE3) || // (YH-4135) Exciting Sport Turbo 4-in-1.nes
			(info.crc32.prg == 0xD35D3D8F)) { // (YH-4136) Exciting Sport Turbo 4-in-1.nes
			static WORD ds[] = { 0x01, 0x00 };

			tmp_fix_057(LENGTH(ds), 0, &ds[0]);
		} else if (info.crc32.prg == 0xC74F9C72) { // 1998 Series No. 10.nes
			static WORD ds[] = { 0x02, 0x01, 0x00, 0x03 };

			tmp_fix_057(LENGTH(ds), 0, &ds[0]);
		} else if (info.crc32.prg == 0xA8930B3B) { // 32-in-1 (42, 52, 62-in-1) (ABCARD-02) (Unl) [p1].nes
			static WORD ds[] = { 0x03, 0x02, 0x01, 0x00 };

			tmp_fix_057(LENGTH(ds), 0, &ds[0]);
		} else {
			static WORD ds[] = { 0x07, 0xFD, 0x03, 0xFE };

			tmp_fix_057(LENGTH(ds), 0, &ds[0]);
		}
	}
}
void extcl_after_mapper_init_057(void) {
	prg_fix_057();
	chr_fix_057();
	mirroring_fix_057();
}
void extcl_cpu_wr_mem_057(WORD address, BYTE value) {
	BYTE index = (address & 0x800) >> 11;

	if (address & 0x2000) {
		m057.reg[index] = (m057.reg[index] & 0xB0) | (value & 0x40);
	} else {
		m057.reg[index] = value;
	}
	prg_fix_057();
	chr_fix_057();
	mirroring_fix_057();
}
BYTE extcl_cpu_rd_mem_057(WORD address, UNUSED(BYTE openbus)) {
	if ((address >= 0x6000) && (address <= 0x6FFF)) {
		return (m057tmp.dipswitch[m057tmp.index]);
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_057(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m057.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_057(void) {
	WORD bank = (m057.reg[1] & 0xE0) >> 5;

	if (m057.reg[1] & 0x10) {
		memmap_auto_32k(MMCPU(0x8000), (bank >> 1));
	} else {
		memmap_auto_16k(MMCPU(0x8000), bank);
		memmap_auto_16k(MMCPU(0xC000), bank);
	}
}
INLINE static void chr_fix_057(void) {
	WORD bank = ((m057.reg[0] & 0x40) >> 3) | (m057.reg[1] & 0x07);

	if (!(m057.reg[0] & 0x80)) {
		memmap_auto_8k(MMPPU(0x0000), ((bank & ~0x03) | (m057.reg[0] & 0x03)));
	} else {
		memmap_auto_8k(MMPPU(0x0000), bank);
	}
}
INLINE static void mirroring_fix_057(void) {
	if (m057.reg[1] & 0x08) {
		mirroring_H();
	} else  {
		mirroring_V();
	}
}

INLINE static void tmp_fix_057(BYTE max, BYTE index, const WORD *ds) {
	m057tmp.ds_used = TRUE;
	m057tmp.max = max;
	m057tmp.index = index;
	m057tmp.dipswitch = ds;
}
