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

INLINE static void prg_fix_519(void);
INLINE static void chr_fix_519(void);
INLINE static void mirroring_fix_519(void);

struct _m519 {
	WORD reg[2];
	BYTE lock;
	BYTE chr_outer;
	BYTE read5xxx[4];
} m519;

void map_init_519(void) {
	EXTCL_AFTER_MAPPER_INIT(519);
	EXTCL_CPU_WR_MEM(519);
	EXTCL_CPU_RD_MEM(519);
	EXTCL_SAVE_MAPPER(519);
	mapper.internal_struct[0] = (BYTE *)&m519;
	mapper.internal_struct_size[0] = sizeof(m519);

	memset(&m519, 0x00, sizeof(m519));

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_519(void) {
	prg_fix_519();
	chr_fix_519();
	mirroring_fix_519();
}
void extcl_cpu_wr_mem_519(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		if (address & 0x0800) {
			m519.read5xxx[address & 0x03] = value & 0x0F;
		}
	} else if (address >= 0x8000) {
		m519.reg[0] = address;
		m519.reg[1] = value;
		if (!m519.lock) {
			m519.lock = (m519.reg[0] & 0x0100) >> 8;
			m519.chr_outer = m519.reg[1] & 0x7C;
			prg_fix_519();
			mirroring_fix_519();
		}
		chr_fix_519();
	}
}
BYTE extcl_cpu_rd_mem_519(BYTE nidx, WORD address, BYTE openbus) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return (address & 0x800 ? m519.read5xxx[address & 0x03] : openbus);
	} else if (address >= 0x8000) {
		return (m519.reg[0] & 0x0040
			? prgrom_rd(nidx, ((address & 0xFFF0) | dipswitch.value))
			: prgrom_rd(nidx, address));
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_519(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m519.reg);
	save_slot_ele(mode, slot, m519.lock);
	save_slot_ele(mode, slot, m519.chr_outer);
	save_slot_ele(mode, slot, m519.read5xxx);
	return (EXIT_OK);
}

INLINE static void prg_fix_519(void) {
	if (m519.reg[0] & 0x80) {
		memmap_auto_16k(0, MMCPU(0x8000), m519.reg[0]);
		memmap_auto_16k(0, MMCPU(0xC000), m519.reg[0]);
	} else {
		memmap_auto_32k(0, MMCPU(0x8000), (m519.reg[0] >> 1));
	}
}
INLINE static void chr_fix_519(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m519.chr_outer | (m519.reg[1] & 0x03)));
}
INLINE static void mirroring_fix_519(void) {
	if (m519.reg[1] & 0x80) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
