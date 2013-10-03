/*
 * irqA12.c
 *
 *  Created on: 19/lug/2011
 *      Author: fhorse
 */

#include "mem_map.h"
#include "ppu_inline.h"
#include "irqA12.h"

void irqA12_IO(WORD value_old) {
	if (!(value_old & 0x1000) && (r2006.value & 0x1000)) {
		if (!extcl_irq_A12_clock) {
			if (!irqA12.counter) {
				irqA12.counter = irqA12.latch;
				if (!irqA12.counter && (irqA12.reload == TRUE)) {
					irqA12.save_counter = 1;
				}
				irqA12.reload = FALSE;
			} else {
				irqA12.counter--;
			}
			if (!irqA12.counter && irqA12.save_counter && irqA12.enable) {
				irq.high |= EXT_IRQ;
			}
			irqA12.save_counter = irqA12.counter;
		} else {
			/*
			 * utilizzato dalle mappers :
			 * mapper222
			 * Futuremedia
			 * Tengen
			 */
			extcl_irq_A12_clock();
		}
	}
}
void irqA12_BS(void) {
	BYTE n_spr;

	if (irqA12.a12BS || ((ppu.frame_x & 0x0007) != 0x0003)) {
		return;
	}

	n_spr = (ppu.frame_x & 0x0038) >> 3;

	if (!n_spr) {
		irqA12.s_adr_old = ppu.bck_adr;
	}

	if ((!spr_ev.count_plus) && (r2000.size_spr == 16)) {
		ppu.spr_adr = 0x1000;
	} else {
		ppu_spr_adr(n_spr);
	}

	if (!(irqA12.s_adr_old & 0x1000) && (ppu.spr_adr & 0x1000)) {
		if (!extcl_irq_A12_clock) {
			irqA12_clock();
		} else {
			/*
			 * utilizzato dalle mappers :
			 * mapper222
			 * Futuremedia
			 * Tengen
			 */
			extcl_irq_A12_clock();
		}
		irqA12.a12BS = TRUE;
	}
	irqA12.s_adr_old = ppu.spr_adr;
}
void irqA12_SB(void) {
	if (irqA12.a12SB || ((ppu.frame_x & 0x0007) != 0x0003)) {
		return;
	}

	if (ppu.frame_x == 323) {
		ppu_spr_adr(7);
		irqA12.b_adr_old = ppu.spr_adr;
	}

	ppu_bck_adr();

	if (!(irqA12.b_adr_old & 0x1000) && (ppu.bck_adr & 0x1000)) {
		if (!extcl_irq_A12_clock) {
			irqA12_clock();
		} else {
			/*
			 * utilizzato dalle mappers :
			 * mapper222
			 * Futuremedia
			 * Tengen
			 */
			extcl_irq_A12_clock();
		}
		irqA12.a12SB = TRUE;
	}
	irqA12.b_adr_old = ppu.bck_adr;
}
void irqA12_RS(void) {
	if (ppu.frame_x == 256) {
		irqA12.a12BS = FALSE;
		return;
	}
	if (ppu.frame_x == 323) {
		irqA12.a12SB = FALSE;
		return;
	}
}
