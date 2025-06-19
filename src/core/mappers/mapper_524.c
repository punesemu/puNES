/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

void prg_swap_vrc2and4_524(WORD address, WORD value);
void chr_swap_vrc2and4_524(WORD address, WORD value);

struct _m524 {
	struct _m524_irq {
		BYTE enabled;
		WORD counter;
	} irq;
} m524;

void map_init_524(void) {
	EXTCL_AFTER_MAPPER_INIT(VRC2and4);
	EXTCL_CPU_WR_MEM(524);
	EXTCL_CPU_RD_MEM(VRC2and4);
	EXTCL_SAVE_MAPPER(524);
	EXTCL_CPU_EVERY_CYCLE(524);
	map_internal_struct_init((BYTE *)&m524, sizeof(m524));
	map_internal_struct_init((BYTE *)&vrc2and4, sizeof(vrc2and4));

	if (info.reset >= HARD) {
		memset(&m524, 0x00, sizeof(m524));
	}

	init_VRC2and4(VRC24_VRC2, 0x01, 0x02, TRUE, info.reset);
	VRC2and4_prg_swap = prg_swap_vrc2and4_524;
	VRC2and4_chr_swap = chr_swap_vrc2and4_524;
}
void extcl_cpu_wr_mem_524(BYTE nidx, WORD address, BYTE value) {
	switch (address & 0xF00C) {
		case 0xF008:
			m524.irq.enabled = TRUE;
			return;
		case 0xF00C :
			m524.irq.enabled = FALSE;
			m524.irq.counter = 0;
			nes[nidx].c.irq.high &= ~EXT_IRQ;
			return;
		default:
			extcl_cpu_wr_mem_VRC2and4(nidx, address, value);
			return;
	}
}
BYTE extcl_save_mapper_524(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m524.irq.enabled);
	save_slot_ele(mode, slot, m524.irq.counter);
	return (extcl_save_mapper_VRC2and4(mode, slot, fp));
}
void extcl_cpu_every_cycle_524(BYTE nidx) {
	if (m524.irq.enabled && (++m524.irq.counter & 0x400)) {
		nes[nidx].c.irq.high |= EXT_IRQ;
	}
}

void prg_swap_vrc2and4_524(WORD address, WORD value) {
	prg_swap_VRC2and4_base(address, (value & 0x1F));
}
void chr_swap_vrc2and4_524(WORD address, WORD value) {
	chr_swap_VRC2and4_base(address, (value & 0xFF));
}
