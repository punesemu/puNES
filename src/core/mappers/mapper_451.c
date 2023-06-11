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
#include "irqA12.h"
#include "save_slot.h"
#include "SST39SF040.h"
#include "gui.h"

INLINE static void prg_fix_451(void);
INLINE static void chr_fix_451(void);

struct _m451 {
	WORD reg;
} m451;
struct _m451tmp {
	BYTE *sst39sf040;
} m451tmp;

void map_init_451(void) {
	EXTCL_AFTER_MAPPER_INIT(451);
	EXTCL_MAPPER_QUIT(451);
	EXTCL_CPU_WR_MEM(451);
	EXTCL_CPU_RD_MEM(451);
	EXTCL_SAVE_MAPPER(451);
	EXTCL_CPU_EVERY_CYCLE(451);
	EXTCL_BATTERY_IO(451);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m451;
	mapper.internal_struct_size[0] = sizeof(m451);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m451, 0x00, sizeof(m451));

	init_MMC3();

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		m451tmp.sst39sf040 = (BYTE *)malloc(prgrom_size());
		memcpy(m451tmp.sst39sf040, prgrom_pnt(), prgrom_size());
		// AMIC A29040B
		sst39sf040_init(m451tmp.sst39sf040, prgrom_size(), 0x37, 0x86, 0x0555, 0x02AA, 65536);
	}

	info.mapper.force_battery_io = TRUE;
	info.mapper.extend_rd = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_451(void) {
	prg_fix_451();
	chr_fix_451();
}
void extcl_mapper_quit_451(void) {
	if (m451tmp.sst39sf040) {
		free(m451tmp.sst39sf040);
		m451tmp.sst39sf040 = NULL;
	}
}
void extcl_cpu_wr_mem_451(WORD address, BYTE value) {
	sst39sf040_write(address, value);
	switch (address & 0xE000) {
		case 0xA000:
			extcl_cpu_wr_mem_MMC3(0xA000, address & 0x01);
			break;
		case 0xC000: {
			BYTE tmp = address & 0xFF;

			extcl_cpu_wr_mem_MMC3(0xC000, tmp - 1);
			extcl_cpu_wr_mem_MMC3(0xC001, 0);
			extcl_cpu_wr_mem_MMC3(tmp == 0xFF ? 0xE000 : 0xE001, 0);
			break;
		}
		case 0xE000:
			m451.reg = address & 0x0003;
			prg_fix_451();
			chr_fix_451();
			break;
	}
}
BYTE extcl_cpu_rd_mem_451(WORD address, BYTE openbus) {
	if (address >= 0x8000) {
		return (sst39sf040_read(address));
	}
	return (openbus);
}
BYTE extcl_save_mapper_451(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m451.reg);
	if (extcl_save_mapper_MMC3(mode, slot, fp) == EXIT_ERROR) return EXIT_ERROR;
	return (sst39sf040_save_mapper(mode, slot, fp));
}
void extcl_cpu_every_cycle_451(void) {
	sst39sf040_tick();
	extcl_cpu_every_cycle_MMC3();
}
void extcl_battery_io_451(BYTE mode, FILE *fp) {
	if (mode == WR_BAT) {
		if (fwrite(m451tmp.sst39sf040, prgrom_size(), 1, fp) < 1) {
			log_error(uL("mapper_451;error on write flash chip"));
		}
	} else {
		if (fread(m451tmp.sst39sf040, prgrom_size(), 1, fp) < 1) {
			log_error(uL("mapper_451;error on read flash chip"));
		}
	}
}

INLINE static void prg_fix_451(void) {
	WORD bank = 0;

	memmap_auto_8k(MMCPU(0x8000), bank);

	bank = 0x10 + (m451.reg & 1) + ((m451.reg & 2) << 2);
	memmap_auto_8k(MMCPU(0xA000), bank);

	bank = 0x20 + (m451.reg & 1) + ((m451.reg & 2) << 2);
	memmap_auto_8k(MMCPU(0xC000), bank);

	bank = 0x30;
	memmap_auto_8k(MMCPU(0xE000), 0x30);
}
INLINE static void chr_fix_451(void) {
	memmap_auto_8k(MMPPU(0x0000), (m451.reg & 0x01));
}
