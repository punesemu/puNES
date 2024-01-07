/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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
} m347tmp;

void map_init_347(void) {
	EXTCL_AFTER_MAPPER_INIT(347);
	EXTCL_CPU_WR_MEM(347);
	EXTCL_SAVE_MAPPER(347);
	map_internal_struct_init((BYTE *)&m347, sizeof(m347));

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memmap_prg_region_init(0, S1K);
		memmap_wram_region_init(0, S1K);
	}

	m347.reg[0] = 0x000F;
	m347.reg[1] = 0x000F;

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
}
void extcl_after_mapper_init_347(void) {
	prg_fix_347();
	wram_fix_347();
	mirroring_fix_347();
}
void extcl_cpu_wr_mem_347(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE value)) {
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
		default:
			break;
	}
}
BYTE extcl_save_mapper_347(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m347.reg);
	return (EXIT_OK);
}

INLINE static void prg_fix_347(void) {
	DBWORD chunkc0 = m347tmp.old_mask_rom
		? (((m347.reg[1] & 0x000F) * 0x1000) + 0x08400)
		: (((m347.reg[1] & 0x000F) * 0x1000) + 0x00000);

	memmap_auto_custom_size(0, MMCPU(0x8000), prgrom_calc_chunk(0, 0x18000), (size_t)(0x400 * 14));
	memmap_wram_custom_size(0, MMCPU(0xB800), wram_calc_chunk(0, 0xC00), (size_t)(0x400 * 2));
	memmap_auto_custom_size(0, MMCPU(0xC000), prgrom_calc_chunk(0, chunkc0), (size_t)(0x400 * 3));
	memmap_wram_custom_size(0, MMCPU(0xCC00), wram_calc_chunk(0, 0x1400), (size_t)(0x400 * 3));
}
INLINE static void wram_fix_347(void) {
	DBWORD chunk6c = (((m347.reg[1] & 0x000F) * 0x1000) + (m347tmp.old_mask_rom ? 0x08000 : 0x00C00));
	DBWORD chunk70 = (((m347.reg[0] & 0x0007) * 0x1000) + (m347tmp.old_mask_rom ? 0x00000 : 0x10000));

	memmap_auto_custom_size(0, MMCPU(0x6000), wram_calc_chunk(0, 0), (size_t)(0x400 * 3));
	memmap_prgrom_custom_size(0, MMCPU(0x6C00), prgrom_calc_chunk(0, chunk6c), 0x400);
	memmap_prgrom_custom_size(0, MMCPU(0x7000), prgrom_calc_chunk(0, chunk70), 0x1000);
}
INLINE static void mirroring_fix_347(void) {
	if (m347.reg[0] & 0x0008) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
