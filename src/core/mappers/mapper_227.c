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

INLINE static void prg_fix_227(void);
INLINE static void chr_fix_227(void);
INLINE static void mirroring_fix_227(void);

struct _m227 {
	WORD reg;
} m227;

void map_init_227(void) {
	EXTCL_AFTER_MAPPER_INIT(227);
	EXTCL_CPU_WR_MEM(227);
	EXTCL_CPU_RD_MEM(227);
	EXTCL_SAVE_MAPPER(227);
	mapper.internal_struct[0] = (BYTE *)&m227;
	mapper.internal_struct_size[0] = sizeof(m227);

	memset(&m227, 0x00, sizeof(m227));

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_227(void) {
	prg_fix_227();
	chr_fix_227();
	mirroring_fix_227();
}
void extcl_cpu_wr_mem_227(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
	m227.reg = address;
	prg_fix_227();
	chr_fix_227();
	mirroring_fix_227();
}
BYTE extcl_cpu_rd_mem_227(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		return (m227.reg & 0x400 ? prgrom_rd(nidx, address | dipswitch.value) : prgrom_rd(nidx, address));
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_227(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m227.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_227(void) {
	WORD outer = ((m227.reg & 0x0100) >> 3) | ((m227.reg & 0x0060) >> 2);
	WORD bank = (m227.reg & 0x001C) >> 2;
	WORD bit0 = (m227.reg & 0x0001);
	WORD bit7 = (m227.reg & 0x0080) >> 7;
	WORD bit9 = (m227.reg & 0x0200) >> 9;

	//Bit 9   Bit 7   Bit 0   Meaning
	//$200s   $080s   $001s
	// (L)     (O)     (S)
	//  0       0       0     Switchable inner 16 KiB bank PPp at CPU $8000-$BFFF,
	//                        fixed inner bank #0 at CPU $C000-$FFFF (UNROM-like with fixed bank 0)
	//  0       0       1     Switchable inner 16 KIB bank PP0 at CPU $8000-$BFFF,
	//                        fixed inner bank #0 at CPU $C000-$FFFF (UNROM-like with only even banks reachable, pointless)
	//  1       0       0     Switchable inner 16 KiB bank PPp at CPU $8000-$BFFF,
	//                        fixed inner bank #7 at CPU $C000-$FFFF (UNROM)
	//  1       0       1     Switchable inner 16 KIB bank PP0 at CPU $8000-$BFFF,
	//                        fixed inner bank #7 at CPU $C000-$FFFF (UNROM with only even banks reachable, pointless)
	//  ?       1       0     Switchable 16 KiB inner bank PPp at CPU $8000-$BFFF, mirrored at CPU $C000-$FFFF (NROM-128)
	//  ?       1       1     Switchable 32 KiB inner bank PP at CPU $8000-$FFFF (NROM-256)
	bank = outer | (bank & ~bit0);
	memmap_auto_16k(0, MMCPU(0x8000), bank);

	bank = bit7 ? bank | bit0 : outer | (7 * bit9);
	memmap_auto_16k(0, MMCPU(0xC000), bank);
}
INLINE static void chr_fix_227(void) {
	BYTE enabled = (info.mapper.battery || !(m227.reg & 0x0080));

	memmap_auto_wp_8k(0, MMPPU(0x0000), 0, TRUE, enabled);
}
INLINE static void mirroring_fix_227(void) {
	if (m227.reg & 0x0002) {
		mirroring_H(0);
	} else  {
		mirroring_V(0);
	}
}
