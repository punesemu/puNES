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

INLINE static void prg_fix_221(void);
INLINE static void mirroring_fix_221(void);

struct _m221 {
	WORD reg[2];
} m221;

void map_init_221(void) {
	EXTCL_AFTER_MAPPER_INIT(221);
	EXTCL_CPU_WR_MEM(221);
	EXTCL_SAVE_MAPPER(221);
	EXTCL_WR_CHR(221);
	mapper.internal_struct[0] = (BYTE *)&m221;
	mapper.internal_struct_size[0] = sizeof(m221);

	if (info.reset >= HARD) {
		memset(&m221, 0x00, sizeof(m221));
	}
}
void extcl_after_mapper_init_221(void) {
	prg_fix_221();
	mirroring_fix_221();
}
void extcl_cpu_wr_mem_221(WORD address, UNUSED(BYTE value)) {
	switch (address & 0xF000) {
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
			// A~FEDC BA98 7654 3210
			//   -------------------
			//   10.. ..Bp OOO. ..PM
			//          || |||    |+- Select nametable mirroring type
			//          || |||    |    0: Vertical
			//          || |||    |    1: Horizontal
			//          || |||    +-- Select NROM-128/Other PRG-ROM modes
			//          || |||         0: NROM-128 (Inner Bank selects 16 KiB PRG-ROM bank
			//          || |||            at CPU $8000-$BFFF mirrored at CPU $C000-$FFFF)
			//          || |||         1: Other mode (decided by bit 8)
			//          || +++------- Select 128 KiB Outer PRG-ROM bank (PRG A17-A19)
			//          |+----------- Select PRG-ROM mode if bit 1=1
			//          |              0: NROM-128 (Inner Bank SHR 1 selects 32 KiB PRG-ROM
			//          |                 bank at CPU $8000-$FFFF)
			//          |              1: UNROM (Inner Bank selects 16 KiB PRG-ROM bank at
			//          |                 CPU $8000-$BFFF, CPU $C000-$FFFF fixed to Inner Bank #7)
			//          +------------ Select 1 MiB Outer PRG-ROM bank (PRG A20)
			m221.reg[0] = address;
			prg_fix_221();
			mirroring_fix_221();
			break;
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			// A~FEDC BA98 7654 3210
			//   -------------------
			//   11.. .... .... CIII
			//                  |+++- 16/32 KiB (depending on PRG-ROM mode) Inner Bank number
			//                  +---- Select CHR-RAM write-protection
			//                         0: Disabled, CHR-RAM write-enabled
			//                         1: Enabled, CHR-RAM write-protected
			m221.reg[1] = address;
			prg_fix_221();
			break;
	}
}
BYTE extcl_save_mapper_221(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m221.reg);

	return (EXIT_OK);
}
void extcl_wr_chr_221(WORD address, BYTE value) {
	if (mapper.write_vram && !(m221.reg[1] & 0x08)) {
		chr.bank_1k[address >> 10][address & 0x3FF] = value;
	}
}

INLINE static void prg_fix_221(void) {
	WORD outer = ((m221.reg[0] & 0x200) >> 3) | ((m221.reg[0] & 0xE0) >> 2);
	WORD inner = m221.reg[1] & 0x07;
	WORD bank;

	if (!(m221.reg[0] & 0x02)) {
		bank = outer | inner;
		_control_bank(bank, info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, bank);
		map_prg_rom_8k(2, 2, bank);
	} else {
		if (m221.reg[0] & 0x100) {
			bank = outer | inner;
			_control_bank(bank, info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, bank);

			bank = outer | 0x07;
			_control_bank(bank, info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, bank);
		} else {
			bank = (outer | inner) >> 1;
			_control_bank(bank, info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, bank);
		}
	}
	map_prg_rom_8k_update();
}

INLINE static void mirroring_fix_221(void) {
	if (m221.reg[0] & 0x01) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
