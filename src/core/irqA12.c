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

void irqA12_IO(BYTE nidx, WORD value, WORD value_old) {
	if (!(value_old & 0x1000) && (value & 0x1000)) {
		if (nes[nidx].irqA12.cycles > irqA12_min_cpu_cycles_prev_rising_edge) {
			nes[nidx].irqA12.cycles = 0;
			if (!extcl_irq_A12_clock) {
				if (!nes[nidx].irqA12.counter) {
					nes[nidx].irqA12.counter = nes[nidx].irqA12.latch;
					if (!nes[nidx].irqA12.counter && nes[nidx].irqA12.reload) {
						nes[nidx].irqA12.save_counter = 1;
					}
					nes[nidx].irqA12.reload = FALSE;
				} else {
					nes[nidx].irqA12.counter--;
				}
				if (!nes[nidx].irqA12.counter && nes[nidx].irqA12.save_counter && nes[nidx].irqA12.enable) {
					nes[nidx].c.irq.high |= EXT_IRQ;
				}
				nes[nidx].irqA12.save_counter = nes[nidx].irqA12.counter;
			} else {
				extcl_irq_A12_clock(nidx);
			}
		}
	}
}
void irqA12_BS(BYTE nidx) {
	BYTE n_spr;

	if (nes[nidx].irqA12.a12BS || ((nes[nidx].p.ppu.frame_x & 0x0007) != 0x0003)) {
		return;
	}

	n_spr = (nes[nidx].p.ppu.frame_x & 0x0038) >> 3;

	if (!n_spr) {
		nes[nidx].irqA12.s_adr_old = nes[nidx].p.ppu.bck_adr;
	}

	if ((!nes[nidx].p.spr_ev.count_plus) && (nes[nidx].p.r2000.size_spr == 16)) {
		nes[nidx].p.ppu.spr_adr = 0x1000;
	} else {
		ppu_spr_adr(n_spr)
	}

	if (!(nes[nidx].irqA12.s_adr_old & 0x1000) && (nes[nidx].p.ppu.spr_adr & 0x1000)) {
		if (!extcl_irq_A12_clock) {
			irqA12_clock()
		} else {
			extcl_irq_A12_clock(nidx);
		}
		nes[nidx].irqA12.a12BS = TRUE;
	}
	nes[nidx].irqA12.s_adr_old = nes[nidx].p.ppu.spr_adr;
}
void irqA12_SB(BYTE nidx) {
	if (nes[nidx].irqA12.a12SB || ((nes[nidx].p.ppu.frame_x & 0x0007) != 0x0003)) {
		return;
	}

	if (nes[nidx].p.ppu.frame_x == 323) {
		ppu_spr_adr(7)
		nes[nidx].irqA12.b_adr_old = nes[nidx].p.ppu.spr_adr;
	}

	ppu_bck_adr(nes[nidx].p.r2000.bpt_adr, nes[nidx].p.r2006.value);

	if (!(nes[nidx].irqA12.b_adr_old & 0x1000) && (nes[nidx].p.ppu.bck_adr & 0x1000)) {
		if (!extcl_irq_A12_clock) {
			irqA12_clock()
		} else {
			extcl_irq_A12_clock(nidx);
		}
		nes[nidx].irqA12.a12SB = TRUE;
	}
	nes[nidx].irqA12.b_adr_old = nes[nidx].p.ppu.bck_adr;
}
void irqA12_RS(BYTE nidx) {
	if (nes[nidx].p.ppu.frame_x == 256) {
		nes[nidx].irqA12.a12BS = FALSE;
		return;
	}
	if (nes[nidx].p.ppu.frame_x == 323) {
		nes[nidx].irqA12.a12SB = FALSE;
		return;
	}
}
