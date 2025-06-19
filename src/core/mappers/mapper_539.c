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

INLINE static void prg_fix_539(void);
INLINE static void wram_fix_539(void);
INLINE static void mirroring_fix_539(void);

struct _m539 {
	BYTE reg[2];
} m539;

void map_init_539(void) {
	EXTCL_AFTER_MAPPER_INIT(539);
	EXTCL_CPU_WR_MEM(539);
	EXTCL_SAVE_MAPPER(539);
	map_internal_struct_init((BYTE *)&m539, sizeof(m539));

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memmap_prg_region_init(0, S256B);
		memmap_wram_region_init(0, S256B);
	}

	if (info.reset >= HARD) {
		memset(&m539, 0x00, sizeof(m539));

		m539.reg[1] = 0x08;
	}
}
void extcl_after_mapper_init_539(void) {
	if (info.reset >= HARD) {
		if (vram_size(0)) {
			for (size_t i = 0; i < vram_size(0); i++) {
				vram_byte(0, i) = (i & 0x02 ? 0xFF : 0x00);
			}
		}
	}
	prg_fix_539();
	wram_fix_539();
	mirroring_fix_539();
}
void extcl_cpu_wr_mem_539(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xFF00) {
		case 0xA000: case 0xA100: case 0xA200: case 0xA300: case 0xA400: case 0xA500: case 0xA600: case 0xA700:
		case 0xA800: case 0xA900: case 0xAA00: case 0xAB00: case 0xAC00: case 0xAD00: case 0xAE00: case 0xAF00:
			m539.reg[0] = value;
			prg_fix_539();
			return;
		case 0xF000: case 0xF100: case 0xF200: case 0xF300: case 0xF400: case 0xF500: case 0xF600: case 0xF700:
		case 0xF800: case 0xF900: case 0xFA00: case 0xFB00: case 0xFC00: case 0xFD00: case 0xFE00: case 0xFF00:
			m539.reg[1] = value;
			mirroring_fix_539();
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_539(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m539.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_539(void) {
	memmap_auto_8k(0, MMCPU(0x8000), 0x0C);
	memmap_auto_8k(0, MMCPU(0xA000), (m539.reg[0] & 0x0F));
	memmap_auto_8k(0, MMCPU(0xC000), 0x0E);
	memmap_auto_8k(0, MMCPU(0xE000), 0x0F);

	memmap_wram_256b(0, MMCPU(0x8200), 0x12);
	memmap_wram_custom_size(0, MMCPU(0xC000), 0, (size_t)(0x100 * 18));
	memmap_wram_256b(0, MMCPU(0xDF00), 0x1F);
}
INLINE static void wram_fix_539(void) {
	// CPU $6000-$7FFF: 8 KiB fixed PRG-ROM bank $D
	//
	// Certain ranges in the CPU address space are overlaid with portions of 8 KiB of PRG-RAM as follows:
	// CPU $6000-$60FF
	// CPU $6200-$62FF
	// CPU $6400-$65FF
	// CPU $8200-$82FF
	// CPU $C000-$D1FF
	// CPU $DF00-$DFFF
	memmap_prgrom_8k(0, MMCPU(0x6000), 0x0D);

	memmap_auto_256b(0, MMCPU(0x6000), 0x18);
	memmap_auto_256b(0, MMCPU(0x6200), 0x1A);
	memmap_auto_custom_size(0, MMCPU(0x6400), 0x1C, (size_t)(0x100 * 2));
}
INLINE static void mirroring_fix_539(void) {
	if (m539.reg[1] & 0x08) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
