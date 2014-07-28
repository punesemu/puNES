/*
 * irql2f.c
 *
 *  Created on: 02/set/2011
 *      Author: fhorse
 */

#include "irql2f.h"

void irql2f_tick(void) {
	if (irql2f.frame_x != ppu.frame_x) {
		return;
	}

	if (ppu.screen_y == (SCR_LINES - 1)) {
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
	return;
}
