/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#ifndef IRQA12_H_
#define IRQA12_H_

#include "cpu.h"
#include "ppu.h"

#define _irqA12_clock(function)\
	if (irqA12.cycles > irqA12_min_cpu_cycles_prev_rising_edge) {\
		BYTE *cnt = &irqA12.counter;\
		BYTE *rld = &irqA12.reload;\
		irqA12.cycles = 0;\
		if (irqA12.race.C001 == TRUE) {\
			cnt = &irqA12.race.counter;\
			rld = &irqA12.race.reload;\
		}\
		if (!(*cnt)) {\
			(*cnt) = irqA12.latch;\
			if (!(*cnt) && ((*rld) == TRUE)) {\
				irqA12.save_counter = 1;\
			}\
			(*rld) = FALSE;\
		} else {\
			(*cnt)--;\
		}\
		if (!(*cnt) && irqA12.save_counter && irqA12.enable) {\
			function;\
		}\
		irqA12.save_counter = (*cnt);\
	}
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
	irq.high |= EXT_IRQ;\
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

/*
 * in "Mickey's Safari in Letterland" avviene un rising edge
 * all'inizio dell'istruzione (quando legge l'opcode) nell'istruzione in cui
 * viene scritto al registro $2001 quindi qualche ciclo dopo viene effettuata
 * una scrittura nel registro $2006 che ne fa fare un'altro. Il secondo se avviene
 * a pochi cicli cpu dal primo non deve essere considerato.
 */
enum irqA12_misc_value {
	irqA12_min_cpu_cycles_prev_rising_edge = 18
};

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

	uint32_t cycles;

	struct _race {
		BYTE C001;
		BYTE counter;
		BYTE reload;
	} race;
} _irqA12;

_irqA12 irqA12;
/* questo non e' necessario salvarlo */
BYTE irqA12_delay;

void irqA12_IO(WORD value, WORD value_old);
void irqA12_BS(void);
void irqA12_SB(void);
void irqA12_RS(void);

#endif /* IRQA12_H_ */
