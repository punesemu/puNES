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

#include "irql2f.h"

void irql2f_tick(BYTE nidx) {
	if (nes[nidx].irql2f.frame_x != nes[nidx].p.ppu.frame_x) {
		return;
	}

	if (nes[nidx].p.ppu.screen_y == (SCR_ROWS - 1)) {
		nes[nidx].irql2f.in_frame = FALSE;
		return;
	}

	if (!nes[nidx].irql2f.in_frame) {
		nes[nidx].irql2f.in_frame = IRQL2F_INFRAME;
		nes[nidx].irql2f.counter = 0;
		nes[nidx].irql2f.pending = FALSE;
		// disabilito l'IRQ dell'MMC5
		nes[nidx].c.irq.high &= ~EXT_IRQ;
		return;
	}
	if (++nes[nidx].irql2f.counter == nes[nidx].irql2f.scanline) {
		nes[nidx].irql2f.pending = IRQL2F_PENDING;
		if (nes[nidx].irql2f.enable) {
			nes[nidx].irql2f.delay = 2;
		}
	}
}
