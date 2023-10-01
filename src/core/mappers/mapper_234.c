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

INLINE static void prg_fix_234(void);
INLINE static void chr_fix_234(void);
INLINE static void mirroring_fix_234(void);

struct _m234 {
	BYTE reg[3];
} m234;

void map_init_234(void) {
	EXTCL_AFTER_MAPPER_INIT(234);
	EXTCL_CPU_WR_MEM(234);
	EXTCL_CPU_RD_MEM(234);
	EXTCL_SAVE_MAPPER(234);
	mapper.internal_struct[0] = (BYTE *)&m234;
	mapper.internal_struct_size[0] = sizeof(m234);

	if (info.reset >= HARD) {
		memset(&m234, 0x00, sizeof(m234));
	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_234(void) {
	prg_fix_234();
	chr_fix_234();
	mirroring_fix_234();
}
void extcl_cpu_wr_mem_234(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xFFF8) {
		case 0xFF80:
		case 0xFF88:
		case 0xFF90:
		case 0xFF98:
			if (!(m234.reg[0] & 0x3F)) {
				m234.reg[0] = value;
				prg_fix_234();
				chr_fix_234();
				mirroring_fix_234();
			}
			return;
		case 0xFFC0:
		case 0xFFC8:
		case 0xFFD0:
		case 0xFFD8:
			if (!(m234.reg[0] & 0x3F)) {
				m234.reg[2] = value;
			}
			return;
		case 0xFFE8:
		case 0xFFF0:
			m234.reg[1] = value;
			prg_fix_234();
			chr_fix_234();
			return;
		default:
			return;
	}
}
BYTE extcl_cpu_rd_mem_234(BYTE nidx, WORD address, BYTE openbus) {
	openbus = address >= 0x8000 ? prgrom_rd(nidx, address) : wram_rd(nidx, address);
	if ((address & 0xFF80) == 0xFF80) {
		extcl_cpu_wr_mem_234(nidx, address, openbus);
	}
	return (openbus);
}
BYTE extcl_save_mapper_234(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m234.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_234(void) {
	if (m234.reg[0] & 0x40) {
		memmap_auto_32k(0, MMCPU(0x8000), ((m234.reg[0] & 0x0E) | (m234.reg[1] & 0x01)));
	} else {
		memmap_auto_32k(0, MMCPU(0x8000), (m234.reg[0] & 0x0F));
	}
}
INLINE static void chr_fix_234(void) {
	if (m234.reg[0] & 0x40) {
		memmap_auto_8k(0, MMPPU(0x0000), (((m234.reg[0] & 0x0E) << 2) | ((m234.reg[1] & 0x70) >> 4)));
	} else {
		memmap_auto_8k(0, MMPPU(0x0000), (((m234.reg[0] & 0x0F) << 2) | ((m234.reg[1] & 0x30) >> 4)));
	}
}
INLINE static void mirroring_fix_234(void) {
	if (m234.reg[0] & 0x80) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
