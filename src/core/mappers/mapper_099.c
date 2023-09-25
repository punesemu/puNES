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

INLINE static void prg_fix_099(BYTE nidx);
INLINE static void wram_fix_099(BYTE nidx);
INLINE static void chr_fix_099(BYTE nidx);

struct _m099 {
	WORD reg[2];
} m099;

void map_init_099(void) {
	EXTCL_AFTER_MAPPER_INIT(099);
	EXTCL_CPU_WR_MEM(099);
	EXTCL_CPU_WR_R4016(099);
	mapper.internal_struct[0] = (BYTE *)&m099;
	mapper.internal_struct_size[0] = sizeof(m099);

	if (info.reset >= HARD) {
		memset(&m099, 0x00, sizeof(m099));
	}
}
void extcl_after_mapper_init_099(void) {
	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		prg_fix_099(nesidx);
		wram_fix_099(nesidx);
		chr_fix_099(nesidx);
	}
}
void extcl_cpu_wr_mem_099(UNUSED(BYTE nidx), UNUSED(WORD address), UNUSED(BYTE value)) {}
void extcl_cpu_wr_r4016_099(BYTE nidx, BYTE value) {


//	if ((value != 0x02) && (value != 0x03))
//	if (value & 0x04)
//	printf("extcl_cpu_wr_r4016_099 : %d 0x%02X\n", nidx, value);


	m099.reg[nidx] = value;

	prg_fix_099(nidx);
	wram_fix_099(nidx);
	chr_fix_099(nidx);
}
BYTE extcl_save_mapper_099(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m099.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_099(BYTE nidx) {
	memmap_auto_32k(nidx, MMCPU(0x8000), nidx);
//	if (prgrom_size() > S32K) {
//		memmap_auto_8k(nidx, MMCPU(0x8000), m099.reg[nidx] & 0x04);
//	}
}
INLINE static void wram_fix_099(BYTE nidx) {
	memmap_auto_8k(nidx, MMCPU(0x6000), 0);
}
INLINE static void chr_fix_099(BYTE nidx) {
	memmap_auto_8k(nidx, MMPPU(0x0000), ((nidx << 1) | (m099.reg[nidx] >> 2)));
}