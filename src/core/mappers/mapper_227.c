/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

INLINE static void prg_fix_227(void);
INLINE static void mirroring_fix_227(void);

static const SWORD dipswitch_227[][32] = {
	{
		0x00,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  -1,
		  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  -1
	}, // 0
};

struct _m227 {
	WORD reg;
} m227;
struct _m227tmp {
	BYTE select;
	BYTE index;
	WORD dipswitch;
} m227tmp;

void map_init_227(void) {
	EXTCL_AFTER_MAPPER_INIT(227);
	EXTCL_CPU_WR_MEM(227);
	EXTCL_CPU_RD_MEM(227);
	EXTCL_SAVE_MAPPER(227);
	EXTCL_WR_CHR(227);
	mapper.internal_struct[0] = (BYTE *)&m227;
	mapper.internal_struct_size[0] = sizeof(m227);

	memset(&m227, 0x00, sizeof(m227));

	if (info.reset == RESET) {
		do {
			m227tmp.index = (m227tmp.index + 1) & 0x1F;
		} while (dipswitch_227[m227tmp.select][m227tmp.index] < 0);
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		m227tmp.select = 0;
		m227tmp.index = 0;
	}

	m227tmp.dipswitch = dipswitch_227[m227tmp.select][m227tmp.index];

	if ((info.format != NES_2_0) && (info.mapper.submapper == WAIXING_FW01)) {
		info.prg.ram.banks_8k_plus = 1;
		info.prg.ram.bat.banks = TRUE;
	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_227(void) {
	prg_fix_227();
	mirroring_fix_227();
}
void extcl_cpu_wr_mem_227(WORD address, UNUSED(BYTE value)) {
	m227.reg = address;
	prg_fix_227();
	mirroring_fix_227();
}
BYTE extcl_cpu_rd_mem_227(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x8000) && (m227.reg & 0x0400)) {
		return (prg_rom_rd((address | m227tmp.dipswitch)));
	}
	return (openbus);
}
BYTE extcl_save_mapper_227(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m227.reg);
	save_slot_ele(mode, slot, m227tmp.index);
	save_slot_ele(mode, slot, m227tmp.dipswitch);

	return (EXIT_OK);
}
void extcl_wr_chr_227(WORD address, BYTE value) {
	if (!info.prg.ram.bat.banks && (m227.reg & 0x0080)) {
		return;
	}
	chr.bank_1k[address >> 10][address & 0x3FF] = value;
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
	//  0       0       0     Switchable inner 16 KiB bank PPp at CPU $8000-$BFFF, fixed inner bank #0 at CPU $C000-$FFFF (UNROM-like with fixed bank 0)
	//  0       0       1     Switchable inner 16 KIB bank PP0 at CPU $8000-$BFFF, fixed inner bank #0 at CPU $C000-$FFFF (UNROM-like with only even banks reachable, pointless)
	//  1       0       0     Switchable inner 16 KiB bank PPp at CPU $8000-$BFFF, fixed inner bank #7 at CPU $C000-$FFFF (UNROM)
	//  1       0       1     Switchable inner 16 KIB bank PP0 at CPU $8000-$BFFF, fixed inner bank #7 at CPU $C000-$FFFF (UNROM with only even banks reachable, pointless)
	//  ?       1       0     Switchable 16 KiB inner bank PPp at CPU $8000-$BFFF, mirrored at CPU $C000-$FFFF (NROM-128)
	//  ?       1       1     Switchable 32 KiB inner bank PP at CPU $8000-$FFFF (NROM-256)

    bank = outer | (bank & ~bit0);
    _control_bank(bank, info.prg.rom.max.banks_16k)
    map_prg_rom_8k(2, 0, bank);

    bank = bit7 ? bank | bit0 : outer | (7 * bit9);
    _control_bank(bank, info.prg.rom.max.banks_16k)
    map_prg_rom_8k(2, 2, bank);

	map_prg_rom_8k_update();
}
INLINE static void mirroring_fix_227(void) {
	if (m227.reg & 0x0002) {
		mirroring_H();
	} else  {
		mirroring_V();
	}
}
