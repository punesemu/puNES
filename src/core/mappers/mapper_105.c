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

void prg_fix_mmc1_105(void);
void prg_swap_mmc1_105(WORD address, WORD value);
void chr_swap_mmc1_105(WORD address, WORD value);

struct _m105 {
	struct _counter_m105 {
		BYTE disabled;
		uint32_t timer;
	} counter;
} m105;

void map_init_105(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC1);
	EXTCL_CPU_WR_MEM(MMC1);
	EXTCL_SAVE_MAPPER(105);
	EXTCL_CPU_EVERY_CYCLE(105);
	map_internal_struct_init((BYTE *)&m105, sizeof(m105));
	map_internal_struct_init((BYTE *)&mmc1, sizeof(mmc1));

	memset(&m105, 0x00, sizeof(m105));

	init_MMC1(MMC1B, HARD);
	MMC1_prg_fix = prg_fix_mmc1_105;
	MMC1_prg_swap = prg_swap_mmc1_105;
	MMC1_chr_swap = chr_swap_mmc1_105;
}
BYTE extcl_save_mapper_105(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m105.counter.timer);
	save_slot_ele(mode, slot, m105.counter.disabled);
	return (EXIT_OK);
}
void extcl_cpu_every_cycle_105(BYTE nidx) {
	if (mmc1.reg[1] & 0x10) {
		m105.counter.timer = 0;
		m105.counter.disabled = FALSE;
		nes[nidx].c.irq.high &= ~EXT_IRQ;
	} else {
		uint32_t timer = (dipswitch.value | 0x10) << 25;
		uint32_t cpu_hz = (uint32_t)machine.cpu_hz;

		if (++m105.counter.timer == timer) {
			m105.counter.disabled = TRUE;
			nes[nidx].c.irq.high |= EXT_IRQ;
		}
		if (!m105.counter.disabled && ((m105.counter.timer % cpu_hz) == 0)) {
			uint32_t seconds = (timer - m105.counter.timer) / cpu_hz;
			uTCHAR buffer[50] = { 0 };

			if (usnprintf(buffer, usizeof(buffer), uL("Time left: %02d:%02d"),
					seconds / 60, seconds % 60) <= (int)usizeof(buffer)) {
				gui_overlay_info_append_subtitle(buffer);
			}
		}
	}
}

void prg_fix_mmc1_105(void) {
	if (mmc1.reg[1] & 0x08) {
		prg_fix_MMC1_base();
		return;
	}
	memmap_auto_32k(0, MMCPU(0x8000), ((mmc1.reg[1] & 0x06) >> 1));
}
void prg_swap_mmc1_105(WORD address, WORD value) {
	prg_swap_MMC1_base(address, (0x08 | (value & 0x07)));
}
void chr_swap_mmc1_105(WORD address, WORD value) {
	chr_swap_MMC1_base(address, (value & 0x01));
}
