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

INLINE static void prg_fix_452(void);
INLINE static void wram_fix_452(void);
INLINE static void mirroring_fix_452(void);

struct _m452 {
	WORD reg[2];
} m452;

void map_init_452(void) {
	EXTCL_AFTER_MAPPER_INIT(452);
	EXTCL_CPU_WR_MEM(452);
	EXTCL_SAVE_MAPPER(452);
	mapper.internal_struct[0] = (BYTE *)&m452;
	mapper.internal_struct_size[0] = sizeof(m452);

	memset(&m452, 0x00, sizeof(m452));

	if (info.mapper.submapper == DEFAULT) {
		info.mapper.submapper = 0;
	}
}
void extcl_after_mapper_init_452(void) {
	prg_fix_452();
	wram_fix_452();
	mirroring_fix_452();
}
void extcl_cpu_wr_mem_452(WORD address, BYTE value) {
	if (!memmap_adr_is_writable(MMCPU(address))) {
		m452.reg[0] = address;
		m452.reg[1] = value;
		prg_fix_452();
		mirroring_fix_452();
	}
}
BYTE extcl_save_mapper_452(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m452.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_452(void) {
	WORD bank = (m452.reg[0] >> 1);

	if (info.mapper.submapper == 1) {
		switch (m452.reg[0] & 0xF000) {
			case 0xA000:
				memmap_auto_16k(MMCPU(0x8000), (m452.reg[0] >> 1));
				memmap_auto_16k(MMCPU(0xC000), 0);
				memmap_wram_8k(MMCPU((0x8000 | ((m452.reg[0] & 0x0600) << 4))), 0);
				return;
			case 0xC000:
				memmap_auto_16k(MMCPU(0x8000), bank);
				memmap_auto_16k(MMCPU(0xC000), (bank | 1));
				memmap_wram_8k(MMCPU((0x8000 | ((m452.reg[0] & 0x0600) << 4))), 0);
				return;
			case 0xD000:
				memmap_auto_8k(MMCPU(0x8000), m452.reg[0]);
				memmap_auto_8k(MMCPU(0xA000), m452.reg[0]);
				memmap_auto_8k(MMCPU(0xC000), m452.reg[0]);
				memmap_auto_8k(MMCPU(0xE000), m452.reg[0]);
				memmap_wram_8k(MMCPU((0x8000 | ((m452.reg[0] & 0x0200) << 4))), 0);
				memmap_wram_8k(MMCPU((0xC000 | ((m452.reg[0] & 0x0200) << 4))), 0);
				return;
			case 0xE000:
				memmap_auto_16k(MMCPU(0x8000), bank);
				memmap_auto_16k(MMCPU(0xC000), (m452.reg[0] & 0x100 ? bank | 0x07 : 0));
				memmap_wram_8k(MMCPU((0x8000 | ((m452.reg[0] & 0x0600) << 4))), 0);
				return;
			default:
				memmap_auto_16k(MMCPU(0x8000), bank);
				memmap_auto_16k(MMCPU(0xC000), 0);
				return;
		}
		return;
	} else if (m452.reg[1] & 0x0002) {
		memmap_auto_8k(MMCPU(0x8000), bank);
		memmap_auto_8k(MMCPU(0xA000), bank);
		memmap_auto_8k(MMCPU(0xC000), bank);
		memmap_auto_8k(MMCPU(0xE000), bank);
		memmap_wram_8k(MMCPU((0x8000 | (((m452.reg[1] & 0x30) << 9) ^ 0x4000))), 0);
	} else if (m452.reg[1] & 0x0008) {
		bank &= ~1;
		memmap_auto_8k(MMCPU(0x8000), bank);
		memmap_auto_8k(MMCPU(0xA000), (bank | 0x01));
		memmap_auto_8k(MMCPU(0xC000), (bank | 0x02));
		memmap_auto_8k(MMCPU(0xE000), (bank | 0x03 | (m452.reg[1] & 0x0004)));
	} else {
		bank >>= 1;
		memmap_auto_16k(MMCPU(0x8000), bank);
		memmap_auto_16k(MMCPU(0xC000), 0);
	}
	memmap_wram_8k(MMCPU((0x8000 | ((m452.reg[1] & 0x30) << 9))), 0);
}
INLINE static void wram_fix_452(void) {
	memmap_disable_8k(MMCPU(0x6000));
}
INLINE static void mirroring_fix_452(void) {
	if (info.mapper.submapper == 1 ? m452.reg[0] & 0x0800 : m452.reg[1] & 0x01) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
