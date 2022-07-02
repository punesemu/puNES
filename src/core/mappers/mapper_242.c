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

INLINE static void prg_fix_242(void);
INLINE static void mirroring_fix_242(void);

static const SWORD dipswitch_242[][32] = {
	{
		0x00,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  -1,
		  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  -1
	}, // 0
	{
		0x10, 0x11, 0x12, 0x0F, 0x0E, 0x0D, 0x0C, 0x14, 0x13, 0x0B, 0x0A, 0x09, 0x08, 0x15, 0x07, 0x16,
		0x06, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x05, 0x04, 0x1E, 0x03, 0x1F, 0x02, 0x01, 0x00
	}, // 1
	{
		0x0F, 0xFF,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
		  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1
	}, // 2
};

struct _m242 {
	WORD reg;
} m242;
struct _m242tmp {
	BYTE two_chips;
	BYTE select;
	BYTE index;
	WORD dipswitch;
} m242tmp;

void map_init_242(void) {
	EXTCL_AFTER_MAPPER_INIT(242);
	EXTCL_CPU_WR_MEM(242);
	EXTCL_CPU_RD_MEM(242);
	EXTCL_SAVE_MAPPER(242);
	EXTCL_WR_CHR(242);
	mapper.internal_struct[0] = (BYTE *)&m242;
	mapper.internal_struct_size[0] = sizeof(m242);

	memset(&m242, 0x00, sizeof(m242));

	m242tmp.two_chips = (prg_size() & 0x20000) && (prg_size() > 0x20000);

	if (info.reset == RESET) {
		do {
			m242tmp.index = (m242tmp.index + 1) & 0x1F;
		} while (dipswitch_242[m242tmp.select][m242tmp.index] < 0);
	} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		if (info.crc32.prg == 0x556C97D2) { // '93 世界冠軍卡超値享受.nes
			m242tmp.select = 1;
			m242tmp.index = 0;
		} else if (info.crc32.prg == 0x7E70BCF4) { // 1992 突破 劃面選關 190-in-1.nes
			m242tmp.select = 2;
			m242tmp.index = 0;
		} else {
			m242tmp.select = 0;
			m242tmp.index = 0;
		}
	}

	m242tmp.dipswitch = dipswitch_242[m242tmp.select][m242tmp.index];

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_242(void) {
	prg_fix_242();
	mirroring_fix_242();
}
void extcl_cpu_wr_mem_242(WORD address, UNUSED(BYTE value)) {
	m242.reg = address;
	prg_fix_242();
	mirroring_fix_242();
}
BYTE extcl_cpu_rd_mem_242(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x8000) && (m242.reg & 0x0100)) {
		return (prg_rom_rd((address | m242tmp.dipswitch)));
	}
	return (openbus);
}
BYTE extcl_save_mapper_242(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m242.reg);
	save_slot_ele(mode, slot, m242tmp.index);
	save_slot_ele(mode, slot, m242tmp.dipswitch);

	return (EXIT_OK);
}
void extcl_wr_chr_242(WORD address, BYTE value) {
	if (!info.prg.ram.bat.banks && (m242.reg & 0x0080)) {
		return;
	}
	chr.bank_1k[address >> 10][address & 0x3FF] = value;
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
	//  0       0       0     Switchable inner 16 KiB bank PPp at CPU $8000-$BFFF, fixed inner bank #0 at CPU $C000-$FFFF (UNROM-like with fixed bank 0)
	//  0       0       1     Switchable inner 16 KIB bank PP0 at CPU $8000-$BFFF, fixed inner bank #0 at CPU $C000-$FFFF (UNROM-like with only even banks reachable, pointless)
	//  1       0       0     Switchable inner 16 KiB bank PPp at CPU $8000-$BFFF, fixed inner bank #7 at CPU $C000-$FFFF (UNROM)
	//  1       0       1     Switchable inner 16 KIB bank PP0 at CPU $8000-$BFFF, fixed inner bank #7 at CPU $C000-$FFFF (UNROM with only even banks reachable, pointless)
	//  ?       1       0     Switchable 16 KiB inner bank PPp at CPU $8000-$BFFF, mirrored at CPU $C000-$FFFF (NROM-128)
	//  ?       1       1     Switchable 32 KiB inner bank PP at CPU $8000-$FFFF (NROM-256)

	if (m242tmp.two_chips) {
		if (m242.reg & 0x0600) {
			outer &= ((info.prg.rom.banks_16k & ~8) - 1);
		} else {
			outer = (info.prg.rom.banks_16k & ~8);
		}
	}

	bank = outer | (bank & ~bit0);
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, bank);

	bank = bit7 ? bank | bit0 : outer | (7 * bit9);
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, bank);

	map_prg_rom_8k_update();
}
INLINE static void mirroring_fix_242(void) {
	if (m242.reg & 0x0002) {
		mirroring_H();
	} else  {
		mirroring_V();
	}
}
