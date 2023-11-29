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

INLINE static void prg_fix_428(void);
INLINE static void chr_fix_428(void);
INLINE static void mirroring_fix_428(void);

struct _m428 {
	BYTE reg[5];
	BYTE data;
} m428;

void map_init_428(void) {
	EXTCL_AFTER_MAPPER_INIT(428);
	EXTCL_CPU_WR_MEM(428);
	EXTCL_CPU_RD_MEM(428);
	EXTCL_SAVE_MAPPER(428);
	map_internal_struct_init((BYTE *)&m428, sizeof(m428));

	if (info.reset >= HARD) {
		memset(&m428, 0x00, sizeof(m428));
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_428(void) {
	prg_fix_428();
	chr_fix_428();
	mirroring_fix_428();
}
void extcl_cpu_wr_mem_428(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		m428.reg[address & 0x03] = value;
		prg_fix_428();
		chr_fix_428();
		mirroring_fix_428();
		return;
	} else if (address >= 0x8000) {
		m428.reg[4] = value;
		chr_fix_428();
		return;
	}
}
BYTE extcl_cpu_rd_mem_428(BYTE nidx, WORD address, BYTE openbus) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return ((openbus & 0xFC) | (dipswitch.value & 0x03));
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_428(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m428.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_428(void) {
	WORD bank = m428.reg[1] >> 5;

	if (m428.reg[1] & 0x10) {
		memmap_auto_32k(0, MMCPU(0x8000), (bank >> 1));
	} else {
		memmap_auto_16k(0, MMCPU(0x8000), bank);
		memmap_auto_16k(0, MMCPU(0xC000), bank);
	}
}
INLINE static void chr_fix_428(void) {
	WORD bank = ((m428.reg[1] & 0x07) & ~(m428.reg[2] >> 6)) | (m428.reg[4] & (m428.reg[2] >> 6));

	memmap_auto_8k(0, MMPPU(0x0000), bank);
}
INLINE static void mirroring_fix_428(void) {
	if (m428.reg[1] & 0x08) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
