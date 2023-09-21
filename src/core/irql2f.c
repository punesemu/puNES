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

void irql2f_tick(BYTE cidx) {
	if (nes[cidx].irql2f.frame_x != nes[cidx].p.ppu.frame_x) {
		return;
	}

	if (nes[cidx].p.ppu.screen_y == (SCR_ROWS - 1)) {
		nes[cidx].irql2f.in_frame = FALSE;
		return;
	}

	if (!nes[cidx].irql2f.in_frame) {
		nes[cidx].irql2f.in_frame = IRQL2F_INFRAME;
		nes[cidx].irql2f.counter = 0;
		nes[cidx].irql2f.pending = FALSE;
		// disabilito l'IRQ dell'MMC5
		nes[cidx].c.irq.high &= ~EXT_IRQ;
		return;
	}
	if (++nes[cidx].irql2f.counter == nes[cidx].irql2f.scanline) {
		nes[cidx].irql2f.pending = IRQL2F_PENDING;
		if (nes[cidx].irql2f.enable) {
			nes[cidx].irql2f.delay = 2;
		}
	}
}
