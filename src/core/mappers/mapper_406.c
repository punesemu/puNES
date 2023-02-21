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

INLINE static void prg_fix_406(BYTE value);
INLINE static void prg_swap_406(WORD address, WORD value);
INLINE static void chr_fix_406(BYTE value);
INLINE static void chr_swap_406(WORD address, WORD value);

struct _m406 {
	WORD mmc3[8];
} m406;
struct _m406tmp {
	BYTE *sst39sf040;
} m406tmp;

void map_init_406(void) {
	EXTCL_AFTER_MAPPER_INIT(406);
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

	mapper.internal_struct[0] = (BYTE *)&m406;
	mapper.internal_struct_size[0] = sizeof(m406);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m406, 0x00, sizeof(m406));

	m406.mmc3[0] = 0;
	m406.mmc3[1] = 2;
	m406.mmc3[2] = 4;
	m406.mmc3[3] = 5;
	m406.mmc3[4] = 6;
	m406.mmc3[5] = 7;
	m406.mmc3[6] = 0;
	m406.mmc3[7] = 0;

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
void extcl_after_mapper_init_406(void) {
	prg_fix_406(mmc3.bank_to_update);
	chr_fix_406(mmc3.bank_to_update);
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

	switch (address & 0xE001) {
		case 0x8000:
			if ((value & 0x40) != (mmc3.bank_to_update & 0x40)) {
				prg_fix_406(value);
			}
			if ((value & 0x80) != (mmc3.bank_to_update & 0x80)) {
				chr_fix_406(value);
			}
			mmc3.bank_to_update = value;
			return;
		case 0x8001: {
			WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

			m406.mmc3[mmc3.bank_to_update & 0x07] = value;

			switch (mmc3.bank_to_update & 0x07) {
				case 0:
					chr_swap_406(cbase ^ 0x0000, value & (~1));
					chr_swap_406(cbase ^ 0x0400, value | 1);
					return;
				case 1:
					chr_swap_406(cbase ^ 0x0800, value & (~1));
					chr_swap_406(cbase ^ 0x0C00, value | 1);
					return;
				case 2:
					chr_swap_406(cbase ^ 0x1000, value);
					return;
				case 3:
					chr_swap_406(cbase ^ 0x1400, value);
					return;
				case 4:
					chr_swap_406(cbase ^ 0x1800, value);
					return;
				case 5:
					chr_swap_406(cbase ^ 0x1C00, value);
					return;
				case 6:
					if (mmc3.bank_to_update & 0x40) {
						prg_swap_406(0xC000, value);
					} else {
						prg_swap_406(0x8000, value);
					}
					return;
				case 7:
					prg_swap_406(0xA000, value);
					return;
			}
			return;
		}
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
	save_slot_ele(mode, slot, m406.mmc3);
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

INLINE static void prg_fix_406(BYTE value) {
	if (value & 0x40) {
		prg_swap_406(0x8000, ~1);
		prg_swap_406(0xC000, m406.mmc3[6]);
	} else {
		prg_swap_406(0x8000, m406.mmc3[6]);
		prg_swap_406(0xC000, ~1);
	}
	prg_swap_406(0xA000, m406.mmc3[7]);
	prg_swap_406(0xE000, ~0);
}
INLINE static void prg_swap_406(WORD address, WORD value) {
	value &= 0x3F;
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
INLINE static void chr_fix_406(BYTE value) {
	WORD cbase = (value & 0x80) << 5;

	chr_swap_406(cbase ^ 0x0000, m406.mmc3[0] & (~1));
	chr_swap_406(cbase ^ 0x0400, m406.mmc3[0] |   1);
	chr_swap_406(cbase ^ 0x0800, m406.mmc3[1] & (~1));
	chr_swap_406(cbase ^ 0x0C00, m406.mmc3[1] |   1);
	chr_swap_406(cbase ^ 0x1000, m406.mmc3[2]);
	chr_swap_406(cbase ^ 0x1400, m406.mmc3[3]);
	chr_swap_406(cbase ^ 0x1800, m406.mmc3[4]);
	chr_swap_406(cbase ^ 0x1C00, m406.mmc3[5]);
}
INLINE static void chr_swap_406(WORD address, WORD value) {
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}
