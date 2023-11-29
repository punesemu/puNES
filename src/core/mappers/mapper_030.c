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
#include "ines.h"
#include "save_slot.h"
#include "SST39SF040.h"
#include "gui.h"

INLINE static void prg_fix_030(void);
INLINE static void chr_fix_030(void);
INLINE static void mirroring_fix_030(void);

struct _m030 {
	WORD reg;
} m030;
struct _m030tmp {
	BYTE *sst39sf040;
} m030tmp;

void map_init_030(void) {
	EXTCL_AFTER_MAPPER_INIT(030);
	EXTCL_CPU_WR_MEM(030);
	EXTCL_SAVE_MAPPER(030);
	map_internal_struct_init((BYTE *)&m030, sizeof(m030));

	if (info.reset >= HARD) {
		memset(&m030, 0x00, sizeof(m030));
	}

	if ((info.format != NES_2_0) && (vram_size(0) <= S8K)) {
		vram_set_ram_size(0, S32K);
	}

	if (ines.flags[FL6] & 0x02) {
		EXTCL_CPU_RD_MEM(030);
		EXTCL_CPU_EVERY_CYCLE(030);
		EXTCL_BATTERY_IO(030);

		if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
			m030tmp.sst39sf040 = (BYTE *)malloc(prgrom_size());
			memcpy(m030tmp.sst39sf040, prgrom_pnt(), prgrom_size());
			sst39sf040_init(m030tmp.sst39sf040, prgrom_size(), 0xBF, 0xB7, 0x5555, 0x2AAA, 4096);
		}
		info.mapper.force_battery_io = TRUE;
		info.mapper.extend_rd = TRUE;
	} else {
		m030tmp.sst39sf040 = NULL;
	}
}
void extcl_after_mapper_init_030(void) {
	prg_fix_030();
	chr_fix_030();
	mirroring_fix_030();
}
void extcl_cpu_wr_mem_030(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0xC000) || !(ines.flags[FL6] & 0x02)) {
		if (!m030tmp.sst39sf040) {
			// bus conflict
			value &= prgrom_rd(nidx, address);
		}
		m030.reg = value;
		prg_fix_030();
		chr_fix_030();
		mirroring_fix_030();
	} else {
		sst39sf040_write(nidx, address, value);
	}
}
BYTE extcl_cpu_rd_mem_030(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	switch (address & 0xF000) {
		case 0x4000:
		case 0x5000:
		case 0x6000:
		case 0x7000:
			return (wram_rd(nidx, address));
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
			return (sst39sf040_read(nidx, address));
		default:
			return (prgrom_rd(nidx, address));
	}
}
BYTE extcl_save_mapper_030(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m030.reg);
	return (sst39sf040_save_mapper(mode, slot, fp));
}
void extcl_cpu_every_cycle_030(BYTE nidx) {
	sst39sf040_tick(nidx);
}
void extcl_battery_io_030(BYTE mode, FILE *fp) {
	if (mode == WR_BAT) {
		if (m030tmp.sst39sf040 && (fwrite(m030tmp.sst39sf040, prgrom_size(), 1, fp) < 1)) {
			log_error(uL("mapper_030;error on write flash chip"));
		}
	} else {
		if (m030tmp.sst39sf040 && (fread(m030tmp.sst39sf040, prgrom_size(), 1, fp) < 1)) {
			log_error(uL("mapper_030;error on read flash chip"));
		}
	}
}
INLINE static void prg_fix_030(void) {
	memmap_auto_16k(0, MMCPU(0x8000), m030.reg);
	memmap_auto_16k(0, MMCPU(0xC000), 0xFF);
}
INLINE static void chr_fix_030(void) {
	memmap_auto_8k(0, MMPPU(0x0000), (m030.reg >> 5));
}
INLINE static void mirroring_fix_030(void) {
	if (info.mapper.submapper == 1) {
		if (m030.reg & 0x80) {
			mirroring_V(0);
		} else {
			mirroring_H(0);
		}
		return;
	} else {
		switch ((ines.flags[FL6] & 0x01) | ((ines.flags[FL6] & 0x08) >> 2)) {
			case 0:
				mirroring_H(0);
				return;
			case 1:
				mirroring_V(0);
				return;
			case 2:
				if (m030.reg & 0x80) {
					mirroring_SCR1(0);
				} else {
					mirroring_SCR0(0);
				}
				return;
			case 3:
				// 4-Screen, cartridge VRAM
				memmap_nmt_chrrom_8k(0, MMPPU(0x2000), 3);
				return;
			default:
				return;
		}
	}
}
