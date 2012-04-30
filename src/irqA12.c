/*
 * irqA12.c
 *
 *  Created on: 19/lug/2011
 *      Author: fhorse
 */

#include "memmap.h"
#include "ppuinline.h"
#include "irqA12.h"

void irqA12_IO(WORD valueOld) {
	if (!(valueOld & 0x1000) && (r2006.value & 0x1000)) {
		if (!extclIrqA12Clock) {
			if (!irqA12.counter) {
				irqA12.counter = irqA12.latch;
				if (!irqA12.counter && irqA12.reload) {
					irqA12.saveCounter = 1;
				}
				irqA12.reload = 0;
			} else {
				irqA12.counter--;
			}
			if (!irqA12.counter && irqA12.saveCounter && irqA12.enable) {
				irq.high |= EXTIRQ;
			}
			irqA12.saveCounter = irqA12.counter;
		} else {
			/*
			 * utilizzato dalle mappers :
			 * Tengen
			 */
			extclIrqA12Clock();
		}
	}
}
void irqA12_BS(void) {
	BYTE nSpr;

	if (irqA12.a12BS || ((ppu.frameX & 0x0007) != 0x0003)) {
		return;
	}

	nSpr = (ppu.frameX & 0x0038) >> 3;

	if (!nSpr) {
		irqA12.sAdrOld = ppu.bckAdr;
	}

	if ((!sprEv.countPlus) && (r2000.sizeSPR == 16)) {
		ppu.sprAdr = 0x1000;
	} else {
		ppuSprAdr(nSpr);
	}

	if (!(irqA12.sAdrOld & 0x1000) && (ppu.sprAdr & 0x1000)) {
		if (!extclIrqA12Clock) {
			irqA12Clock();
		} else {
			/*
			 * utilizzato dalle mappers :
			 * Tengen
			 */
			extclIrqA12Clock();
		}
		irqA12.a12BS = TRUE;
	}
	irqA12.sAdrOld = ppu.sprAdr;
}
void irqA12_SB(void) {
	if (irqA12.a12SB || ((ppu.frameX & 0x0007) != 0x0003)) {
		return;
	}

	if (ppu.frameX == 323) {
		ppuSprAdr(7);
		irqA12.bAdrOld = ppu.sprAdr;
	}

	ppuBckAdr();

	if (!(irqA12.bAdrOld & 0x1000) && (ppu.bckAdr & 0x1000)) {
		if (!extclIrqA12Clock) {
			irqA12Clock();
		} else {
			/*
			 * utilizzato dalle mappers :
			 * Tengen
			 */
			extclIrqA12Clock();
		}
		irqA12.a12SB = TRUE;
	}
	irqA12.bAdrOld = ppu.bckAdr;
}
void irqA12_RS(void) {
	if (ppu.frameX == 256) {
		irqA12.a12BS = FALSE;
		return;
	}
	if (ppu.frameX == 323) {
		irqA12.a12SB = FALSE;
		return;
	}
}
