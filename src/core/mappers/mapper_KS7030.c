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
#include "mem_map.h"
#include "info.h"
#include "mappers.h"
#include "save_slot.h"

INLINE static void prg_fix_KS7030(void);
INLINE static void mirroring_fix_KS7030(void);

struct _ks7030 {
	WORD reg[2];
} ks7030;
struct _ks7030tmp {
	BYTE old_mask_rom;
	BYTE *prg_6000;
	BYTE *prg_6C00;
	BYTE *prg_7000;
	BYTE *prg_B800;
	BYTE *prg_C000;
	BYTE *prg_CC00;
	BYTE *prg_8000;
} ks7030tmp;

void map_init_KS7030(void) {
	EXTCL_AFTER_MAPPER_INIT(KS7030);
	EXTCL_CPU_WR_MEM(KS7030);
	EXTCL_CPU_RD_MEM(KS7030);
	EXTCL_SAVE_MAPPER(KS7030);
	mapper.internal_struct[0] = (BYTE *)&ks7030;
	mapper.internal_struct_size[0] = sizeof(ks7030);

	ks7030.reg[0] = 0x000F;
	ks7030.reg[1] = 0x000F;

	info.prg.ram.banks_8k_plus = 1;

	// Yume Koujou - Doki Doki Panic (Kaiser).nes
	if (info.crc32.prg == 0xFA4DAC91) {
		// As the FCEUX source code comment indicates, the actual bank order in the 128 KiB mask ROM was unknown until July 2020. Emulators expected the ROM image to be laid out like this:
		//    the first 32 KiB to contain the eight banks selected by register $8000 mapped to $7000-$7FFF;
		//    the next 64 KiB to contain the sixteen banks selected by register $9000, with the first 1 KiB mapped to CPU $6C00-$6FFF and the second 3 KiB mapped to CPU $C000-$CBFF;
		//    the final 32 KiB mapped to CPU $8000-$FFFF except where replaced by RAM and the switchable PRG-ROM bank.
		ks7030tmp.old_mask_rom = TRUE;
	} else {
		//The actual mask ROM layout is as follows:
		//    the first 64 KiB contain the sixteen banks selected by register $9000, with the first 3 KiB mapped to CPU $C000-$CBFF and the second 1 KiB mapped to CPU $6C00-$6FFF;
		//    the next 32 KiB contain the eight banks selected by register $8000 mapped to $7000-$7FFF;
		//    the final 32 KiB mapped to CPU $8000-$FFFF except where replaced by RAM and the switchable PRG-ROM bank.
		ks7030tmp.old_mask_rom = FALSE;
	}

	info.mapper.ram_plus_op_controlled_by_mapper = TRUE;
	info.mapper.extend_wr = info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_KS7030(void) {
	prg_fix_KS7030();
	mirroring_fix_KS7030();
}
void extcl_cpu_wr_mem_KS7030(WORD address, BYTE value) {
	switch (address & 0xFC00) {
		case 0x6000:
		case 0x6400:
		case 0x6800:
			ks7030tmp.prg_6000[address - 0x6000] = value;
			break;
		case 0x8000:
		case 0x8400:
		case 0x8800:
		case 0x8C00:
			ks7030.reg[0] = address & 0x000F;
			prg_fix_KS7030();
			mirroring_fix_KS7030();
			break;
		case 0x9000:
		case 0x9400:
		case 0x9800:
		case 0x9C00:
			ks7030.reg[1] = address & 0x000F;
			prg_fix_KS7030();
			break;
		case 0xB800:
		case 0xBC00:
			ks7030tmp.prg_B800[address - 0xB800] = value;
			break;
		case 0xCC00:
		case 0xD000:
		case 0xD400:
			ks7030tmp.prg_CC00[address - 0xCC00] = value;
			break;
		default:
			break;
	}
}
BYTE extcl_cpu_rd_mem_KS7030(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (address < 0x6000) {
		return (openbus);
	}
	switch (address & 0xFC00) {
		case 0x6000:
		case 0x6400:
		case 0x6800:
			return (ks7030tmp.prg_6000[address - 0x6000]);
		case 0x6C00:
			return (ks7030tmp.prg_6C00[address - 0x6C00]);
		case 0x7000:
		case 0x7400:
		case 0x7800:
		case 0x7C00:
			return (ks7030tmp.prg_7000[address - 0x7000]);
		case 0xB800:
		case 0xBC00:
			return (ks7030tmp.prg_B800[address - 0xB800]);
		case 0xC000:
		case 0xC400:
		case 0xC800:
			return (ks7030tmp.prg_C000[address - 0xC000]);
		case 0xCC00:
		case 0xD000:
		case 0xD400:
			return (ks7030tmp.prg_CC00[address - 0xCC00]);
		default:
			return (ks7030tmp.prg_8000[address - 0x8000]);
			break;
	}
	return (openbus);
}
BYTE extcl_save_mapper_KS7030(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, ks7030.reg);

	if (mode == SAVE_SLOT_READ) {
		prg_fix_KS7030();
	}

	return (EXIT_OK);
}

INLINE static void prg_fix_KS7030(void) {
	if (ks7030tmp.old_mask_rom) {
		ks7030tmp.prg_6000 = &prg.ram_plus_8k[0x0000];
		ks7030tmp.prg_6C00 = prg_pnt(((ks7030.reg[1] & 0x000F) * 0x1000) + 0x08000);
		ks7030tmp.prg_7000 = prg_pnt(((ks7030.reg[0] & 0x0007) * 0x1000) + 0x00000);
		ks7030tmp.prg_B800 = &prg.ram_plus_8k[0x0C00];
		ks7030tmp.prg_C000 = prg_pnt(((ks7030.reg[1] & 0x000F) * 0x1000) + 0x08400);
		ks7030tmp.prg_CC00 = &prg.ram_plus_8k[0x1400];
		ks7030tmp.prg_8000 = prg_pnt(0x18000);
	} else {
		ks7030tmp.prg_6000 = &prg.ram_plus_8k[0x0000];
		ks7030tmp.prg_6C00 = prg_pnt(((ks7030.reg[1] & 0x000F) * 0x1000) + 0x00C00);
		ks7030tmp.prg_7000 = prg_pnt(((ks7030.reg[0] & 0x0007) * 0x1000) + 0x10000);
		ks7030tmp.prg_B800 = &prg.ram_plus_8k[0x0C00];
		ks7030tmp.prg_C000 = prg_pnt(((ks7030.reg[1] & 0x000F) * 0x1000) + 0x00000);
		ks7030tmp.prg_CC00 = &prg.ram_plus_8k[0x1400];
		ks7030tmp.prg_8000 = prg_pnt(0x18000);
	}
}
INLINE static void mirroring_fix_KS7030(void) {
	if (ks7030.reg[0] & 0x0008) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
