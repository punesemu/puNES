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
#include "mem_map.h"
#include "ppu.h"
#include "tas.h"
#include "save_slot.h"
#include "SST39SF040.h"
#include "gui.h"

void prg_swap_mmc1_111(WORD address, WORD value);
void chr_swap_mmc1_111(WORD address, WORD value);

INLINE static void prg_fix_gtrom_111(void);
INLINE static void chr_fix_gtrom_111(void);
INLINE static void mirroring_fix_gtrom_111(void);

struct _m111 {
	BYTE reg;
} m111;
struct _gtromtmp {
	BYTE *sst39sf040;
	BYTE *vram;
} gtromtmp;

void map_init_111(void) {
	if (!mapper.write_vram) {
		EXTCL_AFTER_MAPPER_INIT(MMC1);
		EXTCL_CPU_WR_MEM(111_MMC1);
		EXTCL_SAVE_MAPPER(111_MMC1);
		mapper.internal_struct[0] = (BYTE*)&m111;
		mapper.internal_struct_size[0] = sizeof(m111);
		mapper.internal_struct[1] = (BYTE *)&mmc1;
		mapper.internal_struct_size[1] = sizeof(mmc1);

		if (info.reset >= HARD) {
			memset(&m111, 0x00, sizeof(m111));
		}

		init_MMC1(MMC1B);
		MMC1_prg_swap = prg_swap_mmc1_111;
		MMC1_chr_swap = chr_swap_mmc1_111;
	} else {
		EXTCL_AFTER_MAPPER_INIT(111_GTROM);
		EXTCL_MAPPER_QUIT(111_GTROM);
		EXTCL_CPU_WR_MEM(111_GTROM);
		EXTCL_CPU_RD_MEM(111_GTROM);
		EXTCL_SAVE_MAPPER(111_GTROM);
		EXTCL_CPU_EVERY_CYCLE(111_GTROM);
		EXTCL_WR_NMT(111_GTROM);
		EXTCL_RD_NMT(111_GTROM);
		EXTCL_BATTERY_IO(111_GTROM);
		mapper.internal_struct[0] = (BYTE*)&m111;
		mapper.internal_struct_size[0] = sizeof(m111);

		if (info.reset >= HARD) {
			m111.reg = 0xFF;
		}

		if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
			gtromtmp.sst39sf040 = (BYTE *)malloc(prg_size());
			memcpy(gtromtmp.sst39sf040, prg_rom(), prg_size());
			sst39sf040_init(gtromtmp.sst39sf040, prg_size(), 0xBF, 0xB7, 0x5555, 0x2AAA, 4096);
		}

		info.prg.ram.banks_8k_plus = info.prg.ram.bat.banks = 0;
		info.chr.rom.banks_8k = 4;

		info.mapper.force_battery_io = TRUE;
		info.mapper.extend_wr = info.mapper.extend_rd = TRUE;
	}
}

void extcl_cpu_wr_mem_111_MMC1(WORD address, BYTE value) {
	mmc1.reg[(address >> 13) & 0x03] = value;
	MMC1_prg_fix();
	MMC1_chr_fix();
	MMC1_wram_fix();
	MMC1_mirroring_fix();
}
BYTE extcl_save_mapper_111_MMC1(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m111.reg);
	extcl_save_mapper_MMC1(mode, slot, fp);
	return (EXIT_OK);
}

void extcl_after_mapper_init_111_GTROM(void) {
	prg_fix_gtrom_111();
	chr_fix_gtrom_111();
	mirroring_fix_gtrom_111();
}
void extcl_mapper_quit_111_GTROM(void) {
	if (gtromtmp.sst39sf040) {
		free(gtromtmp.sst39sf040);
		gtromtmp.sst39sf040 = NULL;
	}
}
void extcl_cpu_wr_mem_111_GTROM(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x5000:
		case 0x7000:
			m111.reg = value;
			prg_fix_gtrom_111();
			chr_fix_gtrom_111();
			mirroring_fix_gtrom_111();
			break;
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			sst39sf040_write(address, value);
			break;
	}
}
BYTE extcl_cpu_rd_mem_111_GTROM(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0xF000) {
		case 0x5000:
		case 0x7000:
			m111.reg = openbus;
			prg_fix_gtrom_111();
			chr_fix_gtrom_111();
			mirroring_fix_gtrom_111();
			return (m111.reg);
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			return (sst39sf040_read(address));
	}
	return (openbus);
}
BYTE extcl_save_mapper_111_GTROM(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m111.reg);
	sst39sf040_save_mapper(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		mirroring_fix_gtrom_111();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_111_GTROM(void) {
	sst39sf040_tick();
}
BYTE extcl_wr_nmt_111_GTROM(WORD address, BYTE value) {
	//if (!ppu.vblank && r2001.visible && (ppu.screen_y < SCR_ROWS)) {
		gtromtmp.vram[address & 0x1FFF] = value;
	//} else {
	//	gtromtmp.vram[address & 0x1FFF] = value;
	//}
	// write is done
	return (TRUE);
}
BYTE extcl_rd_nmt_111_GTROM(WORD address) {
	//if (!ppu.vblank && r2001.visible && (ppu.screen_y < SCR_ROWS)) {
		return (gtromtmp.vram[address & 0x1FFF]);
	//} else {
	//	return (gtromtmp.vram[address & 0x1FFF]);
	//}
}
void extcl_battery_io_111_GTROM(BYTE mode, FILE *fp) {
	if (!fp || (tas.type != NOTAS)) {
		return;
	}

	if (mode == WR_BAT) {
		if (info.prg.ram.bat.banks) {
			map_bat_wr_default(fp);
		}
		if (fwrite(gtromtmp.sst39sf040, prg_size(), 1, fp) < 1) {
			log_error(uL("CHEAPOCABRA;error on write flash chip"));
		}
	} else {
		if (info.prg.ram.bat.banks) {
			map_bat_rd_default(fp);
		}
		if (fread(gtromtmp.sst39sf040, prg_size(), 1, fp) < 1) {
			log_error(uL("CHEAPOCABRA;error on read flash chip"));
		}
	}
}

void prg_swap_mmc1_111(WORD address, WORD value) {
	prg_swap_MMC1_base(address, (value & 0x0F));
}
void chr_swap_mmc1_111(WORD address, WORD value) {
	chr_swap_MMC1_base(address, (value & 0x3F));
}

INLINE static void prg_fix_gtrom_111(void) {
	BYTE value = m111.reg & 0x0F;

	control_bank(info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_gtrom_111(void) {
	DBWORD bank = (m111.reg & 0x10) >> 4;

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
INLINE static void mirroring_fix_gtrom_111(void) {
	DBWORD bank = 0x02 | ((m111.reg & 0x20) >> 5);

	_control_bank(bank, info.chr.rom.max.banks_8k)
	bank <<= 13;
	gtromtmp.vram = chr_pnt(bank);
}
