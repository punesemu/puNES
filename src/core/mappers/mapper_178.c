/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

// TODO : aggiungere emulazione infrared

INLINE static void prg_fix_178(void);
INLINE static void chr_fix_178(void);
INLINE static void wram_fix_178(void);
INLINE static void mirroring_fix_178(void);

struct _m178 {
	BYTE reg[4];
} m178;
struct _m178tmp {
	BYTE baddumps;
} m178tmp;

void map_init_178(void) {
	EXTCL_AFTER_MAPPER_INIT(178);
	EXTCL_CPU_WR_MEM(178);
	EXTCL_SAVE_MAPPER(178);
	map_internal_struct_init((BYTE *)&m178, sizeof(m178));

	if (info.reset >= HARD) {
		memset(&m178, 0x00, sizeof(m178));
	}

	// Education Computer 32-in-1 (Game Star)(Unl)[!].nes
	// 宠物: 小精灵 IV (Chǒngwù: Xiǎo Jīnglíng IV)
	if ((info.crc32.total == 0xF834F634) || (info.crc32.total == 0xB0B13DBD)) {
		m178tmp.baddumps = TRUE;
	} else {
		m178tmp.baddumps = FALSE;
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_178(void) {
	prg_fix_178();
	chr_fix_178();
	wram_fix_178();
	mirroring_fix_178();
}
void extcl_cpu_wr_mem_178(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xFF00) {
		case 0x4800: {
			BYTE reg = address & 0x03;

			if (m178tmp.baddumps) {
				reg = ((reg & 0x01) << 1) | ((reg & 0x02) >> 1);
			}
			m178.reg[reg] = value;
			prg_fix_178();
			chr_fix_178();
			wram_fix_178();
			mirroring_fix_178();
			return;
		}
		default:
			return;
	}
}
BYTE extcl_save_mapper_178(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m178.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_178(void) {
	WORD base = ((m178.reg[1] & 0x07) << (m178tmp.baddumps ? 1 : 0)) | (m178.reg[2] << 3);
	WORD nrom = (~m178.reg[0] & 0x04) >> 2;
	WORD unrom = (m178.reg[0] & 0x02) >> 1;

	memmap_auto_16k(0, MMCPU(0x8000), (base & ~(nrom * !unrom)));
	memmap_auto_16k(0, MMCPU(0xC000), (base | (nrom | unrom * 6)));
}
INLINE static void chr_fix_178(void) {
	memmap_auto_8k(0, MMPPU(0x0000), ((info.mapper.id == 551) ? m178.reg[3] : 0));
}
INLINE static void wram_fix_178(void) {
	memmap_auto_8k(0, MMCPU(0x6000), (info.mapper.id == 551) ? 0 : m178.reg[3]);
}
INLINE static void mirroring_fix_178(void) {
	if (info.mapper.id != 511) {
		if (m178.reg[0] & 0x01) {
			mirroring_H(0);
		} else {
			mirroring_V(0);
		}
	}
}
