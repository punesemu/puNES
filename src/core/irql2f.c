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

_irql2f irql2f;

void irql2f_tick(void) {
	if (irql2f.frame_x != ppu.frame_x) {
		return;
	}

	if (ppu.screen_y == (SCR_ROWS - 1)) {
		irql2f.in_frame = FALSE;
		return;
	}

	if (!irql2f.in_frame) {
		irql2f.in_frame = IRQL2F_INFRAME;
		irql2f.counter = 0;
		irql2f.pending = FALSE;
		/* disabilito l'IRQ dell'MMC5 */
		irq.high &= ~EXT_IRQ;
		return;
	}
	if (++irql2f.counter == irql2f.scanline) {
		irql2f.pending = IRQL2F_PENDING;
		if (irql2f.enable) {
			irq.high |= EXT_IRQ;
		}
	}
}
