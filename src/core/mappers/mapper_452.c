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
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_452(void);
INLINE static void wram_fix_452(void);
INLINE static void mirroring_fix_452(void);

INLINE static BYTE wram_adr_452(WORD address);

struct _m452 {
	WORD reg[2];
} m452;
struct _m452tmp {
	WORD adr1;
	WORD adr2;
} m452tmp;

void map_init_452(void) {
	EXTCL_AFTER_MAPPER_INIT(452);
	EXTCL_CPU_WR_MEM(452);
	EXTCL_CPU_RD_MEM(452);
	EXTCL_SAVE_MAPPER(452);
	mapper.internal_struct[0] = (BYTE *)&m452;
	mapper.internal_struct_size[0] = sizeof(m452);

	if (info.reset >= HARD) {
		memset(&m452, 0x00, sizeof(m452));
	}

//	if (wram_size() < 0x2000) {
//		wram_set_ram_size(0x2000);
//	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_452(void) {
	prg_fix_452();
	wram_fix_452();
	mirroring_fix_452();
}
void extcl_cpu_wr_mem_452(WORD address, BYTE value) {
	if (address <= 0xDFFF) {
		m452.reg[0] = address;
		m452.reg[1] = value;
		prg_fix_452();
		wram_fix_452();
		mirroring_fix_452();
	}
	if (wram_adr_452(address)) {
		wram_wr(0x6000 | (address & 0x1FFF), value);
	}
}
BYTE extcl_cpu_rd_mem_452(WORD address, BYTE openbus) {
	return (wram_adr_452(address) ? wram_rd(0x6000 | (address & 0x1FFF)) : openbus);
}
BYTE extcl_save_mapper_452(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m452.reg);

	if (mode == SAVE_SLOT_READ) {
		wram_fix_452();
	}

	return (EXIT_OK);
}

INLINE static void prg_fix_452(void) {
	WORD bank = 0;

	if (m452.reg[1] & 0x0002) {
		bank = m452.reg[0] >> 1;
		memmap_auto_8k(MMCPU(0x8000), bank);
		memmap_auto_8k(MMCPU(0xA000), bank);
		memmap_auto_8k(MMCPU(0xC000), bank);
		memmap_auto_8k(MMCPU(0xE000), bank);
	} else if (m452.reg[1] & 0x0008) {
		bank = m452.reg[0] >> 1;
		memmap_auto_8k(MMCPU(0x8000), bank);
		memmap_auto_8k(MMCPU(0xA000), (bank | 0x01));
		memmap_auto_8k(MMCPU(0xC000), (bank | 0x02));
		memmap_auto_8k(MMCPU(0xE000), (bank | 0x03 | (m452.reg[1] & 0x0004)));
	} else {
		bank = m452.reg[0] >> 2;
		memmap_auto_16k(MMCPU(0x8000), bank);
		memmap_auto_16k(MMCPU(0xC000), 0);
	}
}
INLINE static void wram_fix_452(void) {
	m452tmp.adr1 = 0x8000 | ((m452.reg[1] & 0x30) << 9);
	m452tmp.adr2 = (m452.reg[1] & 0x0002) ? 0x8000 | (((m452.reg[1] & 0x30) ^ 0x20) << 9) : 0x0000;
	memmap_auto_8k(MMCPU(0x6000), 0);
}
INLINE static void mirroring_fix_452(void) {
	if (m452.reg[1] & 0x0001) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}

INLINE static BYTE wram_adr_452(WORD address) {
	WORD tmp = address & 0xE000;

	return ((tmp == m452tmp.adr1) || (tmp == m452tmp.adr2) ? TRUE : FALSE);
}
