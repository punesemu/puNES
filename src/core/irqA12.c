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

#include "ppu_inline.h"
#include "irqA12.h"

BYTE irqA12_delay;

void irqA12_IO(BYTE cidx, WORD value, WORD value_old) {
	if (!(value_old & 0x1000) && (value & 0x1000)) {
		if (nes[cidx].irqA12.cycles > irqA12_min_cpu_cycles_prev_rising_edge) {
			nes[cidx].irqA12.cycles = 0;
			if (!extcl_irq_A12_clock) {
				if (!nes[cidx].irqA12.counter) {
					nes[cidx].irqA12.counter = nes[cidx].irqA12.latch;
					if (!nes[cidx].irqA12.counter && nes[cidx].irqA12.reload) {
						nes[cidx].irqA12.save_counter = 1;
					}
					nes[cidx].irqA12.reload = FALSE;
				} else {
					nes[cidx].irqA12.counter--;
				}
				if (!nes[cidx].irqA12.counter && nes[cidx].irqA12.save_counter && nes[cidx].irqA12.enable) {
					nes[cidx].c.irq.high |= EXT_IRQ;
				}
				nes[cidx].irqA12.save_counter = nes[cidx].irqA12.counter;
			} else {
				extcl_irq_A12_clock(cidx);
			}
		}
	}
}
void irqA12_BS(BYTE cidx) {
	BYTE n_spr;

	if (nes[cidx].irqA12.a12BS || ((nes[cidx].p.ppu.frame_x & 0x0007) != 0x0003)) {
		return;
	}

	n_spr = (nes[cidx].p.ppu.frame_x & 0x0038) >> 3;

	if (!n_spr) {
		nes[cidx].irqA12.s_adr_old = nes[cidx].p.ppu.bck_adr;
	}

	if ((!nes[cidx].p.spr_ev.count_plus) && (nes[cidx].p.r2000.size_spr == 16)) {
		nes[cidx].p.ppu.spr_adr = 0x1000;
	} else {
		ppu_spr_adr(n_spr)
	}

	if (!(nes[cidx].irqA12.s_adr_old & 0x1000) && (nes[cidx].p.ppu.spr_adr & 0x1000)) {
		if (!extcl_irq_A12_clock) {
			irqA12_clock()
		} else {
			extcl_irq_A12_clock(cidx);
		}
		nes[cidx].irqA12.a12BS = TRUE;
	}
	nes[cidx].irqA12.s_adr_old = nes[cidx].p.ppu.spr_adr;
}
void irqA12_SB(BYTE cidx) {
	if (nes[cidx].irqA12.a12SB || ((nes[cidx].p.ppu.frame_x & 0x0007) != 0x0003)) {
		return;
	}

	if (nes[cidx].p.ppu.frame_x == 323) {
		ppu_spr_adr(7)
		nes[cidx].irqA12.b_adr_old = nes[cidx].p.ppu.spr_adr;
	}

	ppu_bck_adr(nes[cidx].p.r2000.bpt_adr, nes[cidx].p.r2006.value);

	if (!(nes[cidx].irqA12.b_adr_old & 0x1000) && (nes[cidx].p.ppu.bck_adr & 0x1000)) {
		if (!extcl_irq_A12_clock) {
			irqA12_clock()
		} else {
			extcl_irq_A12_clock(cidx);
		}
		nes[cidx].irqA12.a12SB = TRUE;
	}
	nes[cidx].irqA12.b_adr_old = nes[cidx].p.ppu.bck_adr;
}
void irqA12_RS(BYTE cidx) {
	if (nes[cidx].p.ppu.frame_x == 256) {
		nes[cidx].irqA12.a12BS = FALSE;
		return;
	}
	if (nes[cidx].p.ppu.frame_x == 323) {
		nes[cidx].irqA12.a12SB = FALSE;
		return;
	}
}
