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
#include <stdlib.h>
#include "mappers.h"
#include "mem_map.h"
#include "ppu.h"
#include "tas.h"
#include "save_slot.h"
#include "SST39SF040.h"

INLINE static void prg_fix_CHEAPOCABRA_GTROM(void);
INLINE static void chr_fix_CHEAPOCABRA_GTROM(void);
INLINE static void mirroring_fix_CHEAPOCABRA_GTROM(void);

struct _cheapocabra {
	BYTE reg;
} cheapocabra;
struct _gtromtmp {
	BYTE *sst39sf040;
	BYTE *vram;
} gtromtmp;

void map_init_CHEAPOCABRA(void) {
	if (!mapper.write_vram) {
		info.mapper.submapper = M111;
		map_init_MMC1();
	} else {
		EXTCL_AFTER_MAPPER_INIT(CHEAPOCABRA_GTROM);
		EXTCL_MAPPER_QUIT(CHEAPOCABRA_GTROM);
		EXTCL_CPU_WR_MEM(CHEAPOCABRA_GTROM);
		EXTCL_CPU_RD_MEM(CHEAPOCABRA_GTROM);
		EXTCL_SAVE_MAPPER(CHEAPOCABRA_GTROM);
		EXTCL_WR_NMT(CHEAPOCABRA_GTROM);
		EXTCL_RD_NMT(CHEAPOCABRA_GTROM);
		EXTCL_BATTERY_IO(CHEAPOCABRA_GTROM);
		mapper.internal_struct[0] = (BYTE*) &cheapocabra;
		mapper.internal_struct_size[0] = sizeof(cheapocabra);

		if (info.reset >= HARD) {
			memset(&cheapocabra, 0x00, sizeof(cheapocabra));
		}

		if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
			gtromtmp.sst39sf040 = (BYTE *)malloc(prg_size());
			sst39sf040_init(gtromtmp.sst39sf040, prg_size());
			memcpy(gtromtmp.sst39sf040, prg_rom(), prg_size());
		}

		info.prg.ram.banks_8k_plus = info.prg.ram.bat.banks = 0;
		info.chr.rom.banks_8k = 4;

		info.mapper.force_battery_io = TRUE;
		info.mapper.extend_wr = info.mapper.extend_rd = TRUE;
	}
}

void extcl_after_mapper_init_CHEAPOCABRA_GTROM(void) {
	prg_fix_CHEAPOCABRA_GTROM();
	chr_fix_CHEAPOCABRA_GTROM();
	mirroring_fix_CHEAPOCABRA_GTROM();
}
void extcl_mapper_quit_CHEAPOCABRA_GTROM(void) {
	if (gtromtmp.sst39sf040) {
		free(gtromtmp.sst39sf040);
		gtromtmp.sst39sf040 = NULL;
	}
}
void extcl_cpu_wr_mem_CHEAPOCABRA_GTROM(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x5000:
		case 0x7000:
			cheapocabra.reg = value;
			prg_fix_CHEAPOCABRA_GTROM();
			chr_fix_CHEAPOCABRA_GTROM();
			mirroring_fix_CHEAPOCABRA_GTROM();
			break;
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			sst39sf040_write(((cheapocabra.reg & 0x0F) << 15) | (address & 0x7FFF), value);
			break;
	}
}
BYTE extcl_cpu_rd_mem_CHEAPOCABRA_GTROM(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xF000) {
		case 0x5000:
		case 0x7000:
			if (!prg.ram_plus) {
				prg.ram.data[address & 0x1FFF] = openbus;
			} else {
				prg.ram_plus_8k[address & 0x1FFF] = openbus;
			}
			break;
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			return (sst39sf040_read(((cheapocabra.reg & 0x0F) << 15) | (address & 0x7FFF)));
	}
	return (openbus);
}
BYTE extcl_save_mapper_CHEAPOCABRA_GTROM(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, cheapocabra.reg);

	if (mode == SAVE_SLOT_READ) {
		mirroring_fix_CHEAPOCABRA_GTROM();
	}

	return (EXIT_OK);
}
void extcl_wr_nmt_CHEAPOCABRA_GTROM(WORD address, BYTE value) {
	//if (!ppu.vblank && r2001.visible && (ppu.screen_y < SCR_ROWS)) {
		gtromtmp.vram[address & 0x0FFF] = value;
	//} else {
	//	gtromtmp.vram[address & 0x1FFF] = value;
	//}
}
BYTE extcl_rd_nmt_CHEAPOCABRA_GTROM(WORD address) {
	//if (!ppu.vblank && r2001.visible && (ppu.screen_y < SCR_ROWS)) {
		return (gtromtmp.vram[address & 0x0FFF]);
	//} else {
	//	return (gtromtmp.vram[address & 0x1FFF]);
	//}
}

void extcl_battery_io_CHEAPOCABRA_GTROM(BYTE mode, FILE *fp) {
	if (!fp || (tas.type != NOTAS)) {
		return;
	}

	if (mode == WR_BAT) {
		if (info.prg.ram.bat.banks) {
			map_bat_wr_default(fp);
		}
		if (fwrite(gtromtmp.sst39sf040, prg_size(), 1, fp) < 1) {
			fprintf(stderr, "error on write flash chip\n");
		}
	} else {
		if (info.prg.ram.bat.banks) {
			map_bat_rd_default(fp);
		}
		if (fread(gtromtmp.sst39sf040, prg_size(), 1, fp) < 1) {
			fprintf(stderr, "error on read flash chip\n");
		}
	}
}

INLINE static void prg_fix_CHEAPOCABRA_GTROM(void) {
	BYTE value = cheapocabra.reg & 0x0F;

	control_bank(info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_CHEAPOCABRA_GTROM(void) {
	DBWORD bank = (cheapocabra.reg & 0x10) >> 4;

	_control_bank(bank, info.chr.rom.max.banks_8k)
	bank <<= 13;
	chr.bank_1k[0] = chr_pnt(bank | 0x0000);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
}
INLINE static void mirroring_fix_CHEAPOCABRA_GTROM(void) {
	DBWORD bank = 0x02 | !((cheapocabra.reg & 0x20) >> 5);

	_control_bank(bank, info.chr.rom.max.banks_8k)
	gtromtmp.vram = chr_pnt(bank << 13);
}
