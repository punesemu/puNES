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

INLINE static void prg_fix_057(void);
INLINE static void chr_fix_057(void);
INLINE static void mirroring_fix_057(void);

struct _m057 {
	BYTE reg[2];
} m057;

void map_init_057(void) {
	EXTCL_AFTER_MAPPER_INIT(057);
	EXTCL_CPU_WR_MEM(057);
	EXTCL_CPU_RD_MEM(057);
	EXTCL_SAVE_MAPPER(057);
	map_internal_struct_init((BYTE *)&m057, sizeof(m057));

	memset(&m057, 0x00, sizeof(m057));
}
void extcl_after_mapper_init_057(void) {
	prg_fix_057();
	chr_fix_057();
	mirroring_fix_057();
}
void extcl_cpu_wr_mem_057(UNUSED(BYTE nidx), WORD address, BYTE value) {
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
BYTE extcl_cpu_rd_mem_057(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if ((address >= 0x6000) && (address <= 0x6FFF)) {
		return (dipswitch.value);
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_057(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m057.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_057(void) {
	WORD bank = (m057.reg[1] & 0xE0) >> 5;

	if (m057.reg[1] & 0x10) {
		memmap_auto_32k(0, MMCPU(0x8000), (bank >> 1));
	} else {
		memmap_auto_16k(0, MMCPU(0x8000), bank);
		memmap_auto_16k(0, MMCPU(0xC000), bank);
	}
}
INLINE static void chr_fix_057(void) {
	WORD bank = ((m057.reg[0] & 0x40) >> 3) | (m057.reg[1] & 0x07);

	if (!(m057.reg[0] & 0x80)) {
		memmap_auto_8k(0, MMPPU(0x0000), ((bank & ~0x03) | (m057.reg[0] & 0x03)));
	} else {
		memmap_auto_8k(0, MMPPU(0x0000), bank);
	}
}
INLINE static void mirroring_fix_057(void) {
	if (m057.reg[1] & 0x08) {
		mirroring_H(0);
	} else  {
		mirroring_V(0);
	}
}
