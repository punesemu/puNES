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
#include "irqA12.h"
#include "tas.h"
#include "save_slot.h"
#include "SST39SF040.h"
#include "gui.h"

void prg_swap_406(WORD address, WORD value);

struct _m406tmp {
	BYTE *sst39sf040;
} m406tmp;

void map_init_406(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_MAPPER_QUIT(406);
	EXTCL_CPU_WR_MEM(406);
	EXTCL_CPU_RD_MEM(406);
	EXTCL_SAVE_MAPPER(406);
	EXTCL_CPU_EVERY_CYCLE(406);
	EXTCL_BATTERY_IO(406);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	EXTCL_IRQ_A12_CLOCK(406);
	mapper.internal_struct[0] = (BYTE *)&mmc3;
	mapper.internal_struct_size[0] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));

	init_MMC3();
	MMC3_prg_swap = prg_swap_406;

	if (info.mapper.submapper == DEFAULT) {
		info.mapper.submapper = 0;
	}

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		m406tmp.sst39sf040 = (BYTE *)malloc(prg_size());
		memcpy(m406tmp.sst39sf040, prg_rom(), prg_size());
		// Macronix MX29F040 (0)/AMD AM29F040 (1)
		sst39sf040_init(m406tmp.sst39sf040, prg_size(), info.mapper.submapper == 0 ? 0xC2 : 0x01, 0xA4, 0x5555, 0x2AAA, 65536);
	}

	info.mapper.force_battery_io = TRUE;
	info.mapper.extend_rd = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_mapper_quit_406(void) {
	if (m406tmp.sst39sf040) {
		free(m406tmp.sst39sf040);
		m406tmp.sst39sf040 = NULL;
	}
}
void extcl_cpu_wr_mem_406(WORD address, BYTE value) {
	sst39sf040_write(address, value);

	if (info.mapper.submapper == 0) {
		address = (address & 0xFFFC) | ((address & 0x0001) << 1) | ((address & 0x0002) >> 1);
	} else if ((address <= 0x9000) || (address >= 0xE000)) {
		address ^= 0x6000;
	}

	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_cpu_rd_mem_406(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (address >= 0x8000) {
		return (sst39sf040_read(address));
	}
	// Nessuna WRAM
	if (address >= 0x6000) {
		return (0xFF);
	}
	return (openbus);
}
BYTE extcl_save_mapper_406(BYTE mode, BYTE slot, FILE *fp) {
	extcl_save_mapper_MMC3(mode, slot, fp);
	sst39sf040_save_mapper(mode, slot, fp);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_406(void) {
	sst39sf040_tick();
	extcl_cpu_every_cycle_MMC3();
}
void extcl_battery_io_406(BYTE mode, FILE *fp) {
	if (!fp || (tas.type != NOTAS)) {
		return;
	}

	if (mode == WR_BAT) {
		if (info.prg.ram.bat.banks) {
			map_bat_wr_default(fp);
		}
		if (fwrite(m406tmp.sst39sf040, prg_size(), 1, fp) < 1) {
			log_error(uL("mapper_406;error on write flash chip"));
		}
	} else {
		if (info.prg.ram.bat.banks) {
			map_bat_rd_default(fp);
		}
		if (fread(m406tmp.sst39sf040, prg_size(), 1, fp) < 1) {
			log_error(uL("mapper_406;error on read flash chip"));
		}
	}
}
void extcl_irq_A12_clock_406(void) {
	irqA12_clock()
}

void prg_swap_406(WORD address, WORD value) {
	control_bank_with_AND(0x3F, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
