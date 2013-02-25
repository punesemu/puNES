/*
 * irqA12.c
 *
 *  Created on: 19/lug/2011
 *      Author: fhorse
 */

#include "memmap.h"
#include "ppuinline.h"
#include "irqA12.h"

void irqA12_IO(WORD value_old) {
	if (!(value_old & 0x1000) && (r2006.value & 0x1000)) {
		if (!extcl_irq_A12_clock) {
			if (!irqA12.counter) {
				irqA12.counter = irqA12.latch;
				if (!irqA12.counter && irqA12.reload) {
					irqA12.save_counter = 1;
				}
				irqA12.reload = 0;
			} else {
				irqA12.counter--;
			}
			if (!irqA12.counter && irqA12.save_counter && irqA12.enable) {
				irq.high |= EXTIRQ;
			}
			irqA12.save_counter = irqA12.counter;
		} else {
			/*
			 * utilizzato dalle mappers :
			 * Tengen
			 */
			extcl_irq_A12_clock();
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
		irqA12.s_adr_old = ppu.bckAdr;
	}

	if ((!sprEv.countPlus) && (r2000.sizeSPR == 16)) {
		ppu.sprAdr = 0x1000;
	} else {
		ppuSprAdr(nSpr);
	}

	if (!(irqA12.s_adr_old & 0x1000) && (ppu.sprAdr & 0x1000)) {
		if (!extcl_irq_A12_clock) {
			irqA12_clock();
		} else {
			/*
			 * utilizzato dalle mappers :
			 * Tengen
			 */
			extcl_irq_A12_clock();
		}
		irqA12.a12BS = TRUE;
	}
	irqA12.s_adr_old = ppu.sprAdr;
}
void irqA12_SB(void) {
	if (irqA12.a12SB || ((ppu.frameX & 0x0007) != 0x0003)) {
		return;
	}

	if (ppu.frameX == 323) {
		ppuSprAdr(7);
		irqA12.b_adr_old = ppu.sprAdr;
	}

	ppuBckAdr();

	if (!(irqA12.b_adr_old & 0x1000) && (ppu.bckAdr & 0x1000)) {
		if (!extcl_irq_A12_clock) {
			irqA12_clock();
		} else {
			/*
			 * utilizzato dalle mappers :
			 * Tengen
			 */
			extcl_irq_A12_clock();
		}
		irqA12.a12SB = TRUE;
	}
	irqA12.b_adr_old = ppu.bckAdr;
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
