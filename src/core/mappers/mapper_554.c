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

INLINE static void prg_fix_554(void);
INLINE static void chr_fix_554(void);
INLINE static void wram_fix_554(void);

struct _m554 {
	WORD reg;
} m554;

void map_init_554(void) {
	EXTCL_AFTER_MAPPER_INIT(554);
	EXTCL_CPU_WR_MEM(554);
	EXTCL_CPU_RD_MEM(554);
	EXTCL_SAVE_MAPPER(554);
	mapper.internal_struct[0] = (BYTE *)&m554;
	mapper.internal_struct_size[0] = sizeof(m554);

	memset(&m554, 0x00, sizeof(m554));

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_554(void) {
	prg_fix_554();
	chr_fix_554();
	wram_fix_554();
}
void extcl_cpu_wr_mem_554(UNUSED(WORD address), UNUSED(BYTE value)) {}
BYTE extcl_cpu_rd_mem_554(WORD address, BYTE openbus) {
	switch (address & 0xF000) {
		case 0xC000:
			if ((address >= 0xCAB6) && (address <= 0xCAD7)) {

				m554.reg = (address & 0x3C) >> 2;

//				printf("0x%04X : 0x%02X\n", address, m554.reg);

				chr_fix_554();
				wram_fix_554();
			}
			break;
		case 0xE000:
			address &= 0xFFFE;
			if ((address == 0xEBE2) || (address == 0xEE32)) {
				m554.reg = (address & 0x3C) >> 2;


//				printf("0x%04X : 0x%02X\n", address, m554.reg);


				chr_fix_554();
				wram_fix_554();
			}
			break;
		case 0xF000:
			address &= 0xFFFE;
			if (address == 0xFFFC) {
				m554.reg = (address & 0x3C) >> 2;

//				printf("0x%04X : 0x%02X\n", address, m554.reg);

				chr_fix_554();
				wram_fix_554();
			}
			break;
	}
	return (openbus);
}
BYTE extcl_save_mapper_554(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m554.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_554(void) {
	memmap_auto_8k(MMCPU(0x8000), 0x0A);
	memmap_auto_8k(MMCPU(0xA000), 0x0B);
	memmap_auto_8k(MMCPU(0xC000), 0x06);
	memmap_auto_8k(MMCPU(0xE000), 0x07);
}
INLINE static void chr_fix_554(void) {
	memmap_auto_8k(MMPPU(0x0000), m554.reg);
}
INLINE static void wram_fix_554(void) {
	memmap_prgrom_8k(MMCPU(0x6000), m554.reg);
}
