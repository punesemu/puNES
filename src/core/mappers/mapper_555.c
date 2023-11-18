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
#include "mappers.h"
#include "save_slot.h"
#include "clock.h"
#include "gui.h"

void prg_swap_mmc3_555(WORD address, WORD value);
void wram_fix_mmc3_555(void);
void chr_swap_mmc3_555(WORD address, WORD value);

struct _m555 {
	BYTE reg[2];
	struct _counter_m555 {
		BYTE disabled;
		uint32_t timer;
	} counter;
} m555;

void map_init_555(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(555);
	EXTCL_CPU_RD_MEM(555);
	EXTCL_SAVE_MAPPER(555);
	EXTCL_CPU_EVERY_CYCLE(555);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m555;
	mapper.internal_struct_size[0] = sizeof(m555);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&nes[0].irqA12, 0x00, sizeof(nes[0].irqA12));
	}

	memset(&m555, 0x00, sizeof(m555));

	init_MMC3(HARD);
	MMC3_prg_swap = prg_swap_mmc3_555;
	MMC3_wram_fix = wram_fix_mmc3_555;
	MMC3_chr_swap = chr_swap_mmc3_555;

	info.mapper.extend_wr = TRUE;

	nes[0].irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_555(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		if (address & 0x0800) {
			m555.reg[(address & 0x400) >> 10] = value;
			MMC3_prg_fix();
			MMC3_chr_fix();
			return;
		}
		wram_wr(nidx, address, value);
		return;
	}
	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(nidx, address, value);
	}
}
BYTE extcl_cpu_rd_mem_555(UNUSED(BYTE nidx), WORD address, UNUSED(BYTE openbus)) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		if (address & 0x0800) {
			return ((m555.counter.disabled ? 0x80 : 0x00) | 0x5C);
		}
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_555(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m555.reg);
	save_slot_ele(mode, slot, m555.counter.disabled);
	save_slot_ele(mode, slot, m555.counter.timer);
	return (extcl_save_mapper_MMC3(mode, slot, fp));
}
void extcl_cpu_every_cycle_555(BYTE nidx) {
	extcl_cpu_every_cycle_MMC3(nidx);
	if (!(m555.reg[0] & 0x08)) {
		m555.counter.timer = 0;
		m555.counter.disabled = FALSE;
	} else {
		uint32_t timer = (dipswitch.value | 0x10) << 25;
		uint32_t cpu_hz = (uint32_t)machine.cpu_hz;

		if (++m555.counter.timer == timer) {
			m555.counter.disabled = TRUE;
		}
		if (!m555.counter.disabled && ((m555.counter.timer % cpu_hz) == 0)) {
			uint32_t seconds = (timer - m555.counter.timer) / cpu_hz;
			uTCHAR buffer[50] = { 0 };

			if (usnprintf(buffer, usizeof(buffer), uL("Time left: %02d:%02d"),
					seconds / 60, seconds % 60) <= (int)usizeof(buffer)) {
				gui_overlay_info_append_subtitle(buffer);
			}
		}
	}
}

void prg_swap_mmc3_555(WORD address, WORD value) {
	WORD base = (m555.reg[0] & 0x04) << 3;
	WORD mask = ((m555.reg[0] & 0x03) << 3) | 0x07;

	prg_swap_MMC3_base(address, (base | (value & mask)));
}
void wram_fix_mmc3_555(void) {
	memmap_auto_4k(0, MMCPU(0x5000), 2);
	wram_fix_MMC3_base();
}
void chr_swap_mmc3_555(WORD address, WORD value) {
	WORD base = (m555.reg[0] & 0x04) << 5;

	if ((m555.reg[0] & 0x06) == 0x02) {
		if ((value & 0x40) && (vram_size(0))) {
			memmap_vram_1k(0, MMPPU(address), (base | (value & 0x07)));
		} else {
			memmap_chrrom_1k(0, MMPPU(address), (base | value));
		}
	} else {
		chr_swap_MMC3_base(address, (base | (value & 0x7F)));
	}
}
