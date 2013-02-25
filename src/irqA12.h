/*
 * irqA12.h
 *
 *  Created on: 19/lug/2011
 *      Author: fhorse
 */

#ifndef IRQA12_H_
#define IRQA12_H_

#include "common.h"
#include "cpu.h"
#include "ppu.h"

#define _irqA12_clock(function)\
	if (!irqA12.counter) {\
		irqA12.counter = irqA12.latch;\
		if (!irqA12.counter && irqA12.reload) {\
			irqA12.save_counter = 1;\
		}\
		irqA12.reload = FALSE;\
	} else {\
		irqA12.counter--;\
	}\
	if (!irqA12.counter && irqA12.save_counter && irqA12.enable) {\
		function;\
	}\
	irqA12.save_counter = irqA12.counter
#define irqA12_irq_default()\
	/*\
	 * visto che (per la sincronizzazione tra cpu e ppu)\
	 * sono in anticipo di un clock cpu, in caso mi trovo\
	 * in questo momento al penultimo ciclo cpu dell'istruzione\
	 * allora devo trattarlo come l'ultimo ritardando\
	 * l'esecuzione dell'IRQ.\
	 */\
	irqA12.delay = irqA12_delay;\
	if (cpu.cycles == 2) {\
		irqA12.delay++;\
	}
/* modificato il 23/04/2012
#define irqA12_irq_default()\
	irq.high |= EXTIRQ;\
	if (irqA12.delay) {\
		if (cpu.cycles <= irqA12.delay) {\
			irq.delay = TRUE;\
		}\
	} else if (cpu.cycles == 2) {\
		irq.delay = TRUE;\
	}
*/
#define irqA12_clock() _irqA12_clock(irqA12_irq_default())
#define irqA12_mod(function) _irqA12_clock(function)

typedef struct {
	BYTE present;
	BYTE delay;
	BYTE counter;
	BYTE latch;
	BYTE reload;
	BYTE enable;
	BYTE save_counter;
	BYTE a12BS;
	BYTE a12SB;
	WORD b_adr_old;
	WORD s_adr_old;
} _irqA12;

_irqA12 irqA12;
/* questo non e' necessario salvarlo */
BYTE irqA12_delay;

void irqA12_IO(WORD value_old);
void irqA12_BS(void);
void irqA12_SB(void);
void irqA12_RS(void);

#endif /* IRQA12_H_ */
