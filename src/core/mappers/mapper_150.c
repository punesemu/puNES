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

INLINE static void prg_fix_150(void);
INLINE static void chr_fix_150(void);
INLINE static void mirroring_fix_150(void);

struct _m150 {
	BYTE index;
	BYTE reg[8];
} m150;
struct _m150tmp {
	BYTE inverted_chr;
} m150tmp;

void map_init_150(void) {
	EXTCL_AFTER_MAPPER_INIT(150);
	EXTCL_CPU_WR_MEM(150);
	EXTCL_CPU_RD_MEM(150);
	EXTCL_SAVE_MAPPER(150);
	map_internal_struct_init((BYTE *)&m150, sizeof(m150));

	if (info.reset >= HARD) {
		memset(&m150, 0x00, sizeof(m150));

		m150.index = dipswitch.value;
	}

	if ((info.crc32.prg == 0xE93400B2) || // Poker III (Sachen) [!].nes
		(info.crc32.prg == 0x24A2B2BC)) { // Poker III (Sachen) [a1].nes
		m150tmp.inverted_chr = TRUE;
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_150(void) {
	prg_fix_150();
	chr_fix_150();
	mirroring_fix_150();
}
void extcl_cpu_wr_mem_150(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x5FFF)) {
		if (address & 0x100) {
			value = dipswitch.value | (value & 0x07);
			if (address & 0x01) {
				m150.reg[m150.index] = value;
				prg_fix_150();
				chr_fix_150();
				mirroring_fix_150();
			} else {
				m150.index = value;
			}
		}
	}
}
BYTE extcl_cpu_rd_mem_150(BYTE nidx, WORD address, BYTE openbus) {
	if ((address >= 0x4000) && (address <= 0x5FFF)) {
		if ((address & 0x101) == 0x101) {
			return ((m150.reg[m150.index] & (~dipswitch.value & 0x07)) | (openbus & ~(~dipswitch.value & 0x07)));
		}
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_150(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m150.index);
	save_slot_ele(mode, slot, m150.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_150(void) {
	memmap_auto_32k(0, MMCPU(0x8000), (m150.reg[5] | (m150.reg[2] & 0x01)));
}
INLINE static void chr_fix_150(void) {
	WORD bank = info.mapper.id == 243
		? m150tmp.inverted_chr
			? (m150.reg[2] << 3) | ((m150.reg[6] & 0x03) << 1) | (m150.reg[4] & 0x01)
			: (m150.reg[6] << 2) | ((m150.reg[4] & 0x01) << 1) | (m150.reg[2] & 0x01)
		: (m150.reg[2] << 3) | ((m150.reg[4] & 0x01) << 2) | (m150.reg[6] & 0x03);

	memmap_auto_8k(0, MMPPU(0x0000), bank);
}
INLINE static void mirroring_fix_150(void) {
	switch ((m150.reg[7] & 0x06) >> 1) {
		default:
		case 0:
			mirroring_SCR0x3_SCR1x1(0);
			break;
		case 1:
			mirroring_H(0);
			break;
		case 2:
			mirroring_V(0);
			break;
		case 3:
			mirroring_SCR1(0);
			break;
	}
}
