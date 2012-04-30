/*
 * irqA12.h
 *
 *  Created on: 19/lug/2011
 *      Author: fhorse
 */

#ifndef IRQA12_H_
#define IRQA12_H_

#include "common.h"
#include "cpu6502.h"
#include "ppu.h"

#define _irqA12Clock(function)\
	if (!irqA12.counter) {\
		irqA12.counter = irqA12.latch;\
		if (!irqA12.counter && irqA12.reload) {\
			irqA12.saveCounter = 1;\
		}\
		irqA12.reload = FALSE;\
	} else {\
		irqA12.counter--;\
	}\
	if (!irqA12.counter && irqA12.saveCounter && irqA12.enable) {\
		function;\
	}\
	irqA12.saveCounter = irqA12.counter
#define irqA12IRQDefault()\
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
#define irqA12IRQDefault()\
	irq.high |= EXTIRQ;\
	if (irqA12.delay) {\
		if (cpu.cycles <= irqA12.delay) {\
			irq.delay = TRUE;\
		}\
	} else if (cpu.cycles == 2) {\
		irq.delay = TRUE;\
	}
*/
#define irqA12Clock() _irqA12Clock(irqA12IRQDefault())
#define irqA12Mod(function) _irqA12Clock(function)

typedef struct {
	BYTE present;
	BYTE delay;
	BYTE counter;
	BYTE latch;
	BYTE reload;
	BYTE enable;
	BYTE saveCounter;
	BYTE a12BS;
	BYTE a12SB;
	WORD bAdrOld;
	WORD sAdrOld;
} _irqA12;

_irqA12 irqA12;
/* questo non e' necessario salvarlo */
BYTE irqA12_delay;

void irqA12_IO(WORD valueOld);
void irqA12_BS(void);
void irqA12_SB(void);
void irqA12_RS(void);

#endif /* IRQA12_H_ */
