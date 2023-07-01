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

INLINE static void prg_fix_242(void);
INLINE static void chr_fix_242(void);
INLINE static void mirroring_fix_242(void);

struct _m242 {
	WORD reg;
} m242;
struct _m242tmp {
	BYTE two_chips;
} m242tmp;

void map_init_242(void) {
	EXTCL_AFTER_MAPPER_INIT(242);
	EXTCL_CPU_WR_MEM(242);
	EXTCL_CPU_RD_MEM(242);
	EXTCL_SAVE_MAPPER(242);
	mapper.internal_struct[0] = (BYTE *)&m242;
	mapper.internal_struct_size[0] = sizeof(m242);

	memset(&m242, 0x00, sizeof(m242));

	m242tmp.two_chips = (prgrom_size() & S128K) && (prgrom_size() > S128K);

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_242(void) {
	prg_fix_242();
	chr_fix_242();
	mirroring_fix_242();
}
void extcl_cpu_wr_mem_242(WORD address, UNUSED(BYTE value)) {
	m242.reg = address;
	prg_fix_242();
	chr_fix_242();
	mirroring_fix_242();
}
BYTE extcl_cpu_rd_mem_242(WORD address, UNUSED(BYTE openbus)) {
	if (address >= 0x8000) {
		return (m242.reg & 0x100 ? prgrom_rd(address | dipswitch.value) : prgrom_rd(address));
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_242(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m242.reg);

	return (EXIT_OK);
}

INLINE static void prg_fix_242(void) {
	WORD outer = (m242.reg & 0x0060) >> 2;
	WORD bank = (m242.reg & 0x001C) >> 2;
	WORD bit0 = (m242.reg & 0x0001);
	WORD bit7 = (m242.reg & 0x0080) >> 7;
	WORD bit9 = (m242.reg & 0x0200) >> 9;

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
	if (m242tmp.two_chips) {
		if (m242.reg & 0x0600) {
			outer &= ((info.mapper.prgrom_banks_16k & ~8) - 1);
		} else {
			outer = (info.mapper.prgrom_banks_16k & ~8);
		}
	}

	bank = outer | (bank & ~bit0);
	memmap_auto_16k(MMCPU(0x8000), bank);

	bank = outer | (bit7 ? bank | bit0 : 7 * bit9);
	memmap_auto_16k(MMCPU(0xC000), bank);
}
INLINE static void chr_fix_242(void) {
	BYTE enabled = info.mapper.battery || !(m242.reg & 0x0080) || (prgrom_size() < S512K);

	memmap_auto_wp_8k(MMPPU(0x0000), 0, TRUE, enabled);
}
INLINE static void mirroring_fix_242(void) {
	if (m242.reg & 0x0002) {
		mirroring_H();
	} else  {
		mirroring_V();
	}
}
