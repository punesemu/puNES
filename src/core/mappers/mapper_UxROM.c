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
#include <stdlib.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "ines.h"
#include "tas.h"
#include "save_slot.h"
#include "SST39SF040.h"
#include "gui.h"

INLINE static void mirroring_fix_UNROM512(void);

struct _unrom512 {
	BYTE reg;
} unrom512;
struct _unrom512tmp {
	BYTE mirroring;
	BYTE *sst39sf040;
} unrom512tmp;

void map_init_UxROM(BYTE model) {
	switch (model) {
		case UNLROM:
			EXTCL_CPU_WR_MEM(UnlROM);
			break;
		case UXROM:
			EXTCL_CPU_WR_MEM(UxROM);
			break;
		case UXROMNBC:
			EXTCL_CPU_WR_MEM(UxROM);
			break;
		case UNL1XROM:
			EXTCL_CPU_WR_MEM(Unl1xROM);
			break;
		case UNROM180:
			EXTCL_CPU_WR_MEM(UNROM_180);
			break;
		case UNROM512:
			EXTCL_MAPPER_QUIT(UNROM512);
			EXTCL_CPU_WR_MEM(UNROM512);
			EXTCL_CPU_RD_MEM(UNROM512);
			EXTCL_SAVE_MAPPER(UNROM512);
			EXTCL_CPU_EVERY_CYCLE(UNROM512);
			mapper.internal_struct[0] = (BYTE *)&unrom512;
			mapper.internal_struct_size[0] = sizeof(unrom512);

			memset(&unrom512, 0x00, sizeof(unrom512));

			// questa mapper non ha CHR ROM
			if (info.format != NES_2_0) {
				info.chr.rom.banks_8k = 4;
			}

			if (ines.flags[FL6] & 0x02) {
				EXTCL_BATTERY_IO(UNROM512);
				if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
					unrom512tmp.sst39sf040 = (BYTE *)malloc(prg_size());
					memcpy(unrom512tmp.sst39sf040, prg_rom(), prg_size());
					sst39sf040_init(unrom512tmp.sst39sf040, prg_size(), 0xBF, 0xB7, 0x5555, 0x2AAA, 4096);
				}
				info.mapper.force_battery_io = TRUE;
				info.mapper.extend_rd = TRUE;
			} else {
				unrom512tmp.sst39sf040 = NULL;
			}

			unrom512tmp.mirroring = (ines.flags[FL6] & 0x01) | ((ines.flags[FL6] & 0x08) >> 2);
			// Adventures of Panzer, The (World) (Aftermarket) (Unl).nes
			if (info.crc32.prg == 0xF9B944CF) {
				unrom512tmp.mirroring = MIRRORING_VERTICAL;
			}
			mirroring_fix_UNROM512();
			break;
	}
}

void extcl_cpu_wr_mem_UxROM(WORD address, BYTE value) {
	if (info.mapper.submapper == UXROM) {
		/* bus conflict */
		value &= prg_rom_rd(address);
	}

	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_Unl1xROM(WORD address, BYTE value) {
	/* bus conflict */
	value = (value & prg_rom_rd(address)) >> 2;

	control_bank_with_AND(0x0F, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_UNROM_180(UNUSED(WORD address), BYTE value) {
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, value);
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_UnlROM(UNUSED(WORD address), BYTE value) {
	control_bank_with_AND(0x0F, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);
	map_prg_rom_8k_update();
}

void extcl_mapper_quit_UNROM512(void) {
	if (unrom512tmp.sst39sf040) {
		free(unrom512tmp.sst39sf040);
		unrom512tmp.sst39sf040 = NULL;
	}
}
void extcl_cpu_wr_mem_UNROM512(UNUSED(WORD address), BYTE value) {
	if (!(ines.flags[FL6] & 0x02) || (address >= 0xC000)) {
		DBWORD bank;

		unrom512.reg = value;

		value = unrom512.reg & 0x1F;
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k_update();

		bank = (unrom512.reg & 0x60) >> 5;
		_control_bank(bank, info.chr.rom.max.banks_8k)
		bank <<= 13;
		chr.bank_1k[0] = chr_pnt(bank);
		chr.bank_1k[1] = chr_pnt(bank | 0x0400);
		chr.bank_1k[2] = chr_pnt(bank | 0x0800);
		chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
		chr.bank_1k[4] = chr_pnt(bank | 0x1000);
		chr.bank_1k[5] = chr_pnt(bank | 0x1400);
		chr.bank_1k[6] = chr_pnt(bank | 0x1800);
		chr.bank_1k[7] = chr_pnt(bank | 0x1C00);

		mirroring_fix_UNROM512();
	} else {
		sst39sf040_write(address, value);
	}
}
BYTE extcl_cpu_rd_mem_UNROM512(WORD address, BYTE openbus) {
	switch (address & 0xF000) {
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
			return (sst39sf040_read(address));
	}
	return (openbus);
}
BYTE extcl_save_mapper_UNROM512(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, unrom512.reg);
	sst39sf040_save_mapper(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		mirroring_fix_UNROM512();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_UNROM512(void) {
	sst39sf040_tick();
}
void extcl_battery_io_UNROM512(BYTE mode, FILE *fp) {
	if (mode == WR_BAT) {
		if (unrom512tmp.sst39sf040 && (fwrite(unrom512tmp.sst39sf040, prg_size(), 1, fp) < 1)) {
			log_error(uL("UNROM512;error on write flash chip"));
		}
	} else {
		if (unrom512tmp.sst39sf040 && (fread(unrom512tmp.sst39sf040, prg_size(), 1, fp) < 1)) {
			log_error(uL("UNROM512;error on read flash chip"));
		}
	}
}

INLINE static void mirroring_fix_UNROM512(void) {
	switch (unrom512tmp.mirroring) {
		case 0:
			mirroring_H();
			return;
		case 1:
			mirroring_V();
			return;
		case 2:
			if (unrom512.reg & 0x80) {
				mirroring_SCR1();
			} else {
				mirroring_SCR0();
			}
			return;
		case 3:
			// 4-Screen, cartridge VRAM
			map_nmt_chr_rom_4k(7);
			return;
	}
}
