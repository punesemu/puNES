/*
 * irql2f.c
 *
 *  Created on: 02/set/2011
 *      Author: fhorse
 */

#include "irql2f.h"

void irql2f_tick(void) {
	if (irql2f.frameX != ppu.frameX) {
		return;
	}

	if (ppu.screenY == (SCRLINES - 1)) {
		irql2f.inFrame = FALSE;
		return;
	}

	if (!irql2f.inFrame) {
		irql2f.inFrame = IRQL2FINFRAME;
		irql2f.counter = 0;
		irql2f.pending = FALSE;
		/* disabilito l'IRQ dell'MMC5 */
		irq.high &= ~EXTIRQ;
		return;
	}
	if (++irql2f.counter == irql2f.scanline) {
		irql2f.pending = IRQL2FPENDING;
		if (irql2f.enable) {
			irq.high |= EXTIRQ;
		}
	}
	return;
}
