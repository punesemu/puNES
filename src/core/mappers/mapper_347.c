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
#include "mem_map.h"
#include "info.h"
#include "mappers.h"
#include "save_slot.h"

INLINE static void prg_fix_347(void);
INLINE static void wram_fix_347(void);
INLINE static void mirroring_fix_347(void);

struct _m347 {
	WORD reg[2];
} m347;
struct _m347tmp {
	BYTE old_mask_rom;
	BYTE *prg_B800;
	BYTE *prg_C000;
	BYTE *prg_CC00;
	BYTE *prg_8000;
} m347tmp;

void map_init_347(void) {
	EXTCL_AFTER_MAPPER_INIT(347);
	EXTCL_CPU_WR_MEM(347);
	EXTCL_CPU_RD_MEM(347);
	EXTCL_SAVE_MAPPER(347);
	mapper.internal_struct[0] = (BYTE *)&m347;
	mapper.internal_struct_size[0] = sizeof(m347);

	m347.reg[0] = 0x000F;
	m347.reg[1] = 0x000F;

	//info.prg.ram.banks_8k_plus = 1;

	// Yume Koujou - Doki Doki Panic (Kaiser).nes
	if (info.crc32.prg == 0xFA4DAC91) {
		// As the FCEUX source code comment indicates, the actual bank order in the 128 KiB mask ROM was unknown until
		// July 2020. Emulators expected the ROM image to be laid out like this:
		//    the first 32 KiB to contain the eight banks selected by register $8000 mapped to $7000-$7FFF;
		//    the next 64 KiB to contain the sixteen banks selected by register $9000, with the first 1 KiB mapped
		//        to CPU $6C00-$6FFF and the second 3 KiB mapped to CPU $C000-$CBFF;
		//    the final 32 KiB mapped to CPU $8000-$FFFF except where replaced by RAM and the switchable PRG-ROM bank.
		m347tmp.old_mask_rom = TRUE;
	} else {
		//The actual mask ROM layout is as follows:
		//    the first 64 KiB contain the sixteen banks selected by register $9000, with the first 3 KiB mapped to
		//        CPU $C000-$CBFF and the second 1 KiB mapped to CPU $6C00-$6FFF;
		//    the next 32 KiB contain the eight banks selected by register $8000 mapped to $7000-$7FFF;
		//    the final 32 KiB mapped to CPU $8000-$FFFF except where replaced by RAM and the switchable PRG-ROM bank.
		m347tmp.old_mask_rom = FALSE;
	}

	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_347(void) {
	prg_fix_347();
	wram_fix_347();
	mirroring_fix_347();
}
void extcl_cpu_wr_mem_347(WORD address, BYTE value) {
	switch (address & 0xFC00) {
		case 0x8000:
		case 0x8400:
		case 0x8800:
		case 0x8C00:
			m347.reg[0] = address & 0x000F;
			prg_fix_347();
			wram_fix_347();
			mirroring_fix_347();
			break;
		case 0x9000:
		case 0x9400:
		case 0x9800:
		case 0x9C00:
			m347.reg[1] = address & 0x000F;
			prg_fix_347();
			wram_fix_347();
			break;
		case 0xB800:
		case 0xBC00:
			m347tmp.prg_B800[address - 0xB800] = value;
			break;
		case 0xCC00:
		case 0xD000:
		case 0xD400:
			m347tmp.prg_CC00[address - 0xCC00] = value;
			break;
		default:
			break;
	}
}
BYTE extcl_cpu_rd_mem_347(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xFC00) {
		case 0xB800:
		case 0xBC00:
			return (m347tmp.prg_B800[address - 0xB800]);
		case 0xC000:
		case 0xC400:
		case 0xC800:
			return (m347tmp.prg_C000[address - 0xC000]);
		case 0xCC00:
		case 0xD000:
		case 0xD400:
			return (m347tmp.prg_CC00[address - 0xCC00]);
		default:
			if (address < 0x8000) {
				return (openbus);
			}
			return (m347tmp.prg_8000[address - 0x8000]);
	}
}
BYTE extcl_save_mapper_347(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m347.reg);

	if (mode == SAVE_SLOT_READ) {
		prg_fix_347();
		wram_fix_347();
	}

	return (EXIT_OK);
}

INLINE static void prg_fix_347(void) {
	if (m347tmp.old_mask_rom) {
		m347tmp.prg_B800 = &wram.data[0x0C00];
		m347tmp.prg_C000 = prg_pnt(((m347.reg[1] & 0x000F) * 0x1000) + 0x08400);
		m347tmp.prg_CC00 = &wram.data[0x1400];
		m347tmp.prg_8000 = prg_pnt(0x18000);
	} else {
		m347tmp.prg_B800 = &wram.data[0x0C00];
		m347tmp.prg_C000 = prg_pnt(((m347.reg[1] & 0x000F) * 0x1000) + 0x00000);
		m347tmp.prg_CC00 = &wram.data[0x1400];
		m347tmp.prg_8000 = prg_pnt(0x18000);
	}
}
INLINE static void wram_fix_347(void) {
	wram_map_auto_1k(0x6000, 0);
	wram_map_auto_1k(0x6400, 1);
	wram_map_auto_1k(0x6800, 2);
	wram_map_prg_rom_1k(0x6C00, (((m347.reg[1] & 0x000F) * 0x1000) + (m347tmp.old_mask_rom ? 0x08000 : 0x00C00)) / 0x0400);
	wram_map_prg_rom_4k(0x7000, (((m347.reg[0] & 0x0007) * 0x1000) + (m347tmp.old_mask_rom ? 0x00000 : 0x10000)) / 0x1000);
}
INLINE static void mirroring_fix_347(void) {
	if (m347.reg[0] & 0x0008) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
