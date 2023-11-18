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

#ifndef IRQA12_H_
#define IRQA12_H_

#include "common.h"

// in "Mickey's Safari in Letterland" avviene un rising edge
// all'inizio dell'istruzione (quando legge l'opcode) nell'istruzione in cui
// viene scritto al registro $2001 quindi qualche ciclo dopo viene effettuata
// una scrittura nel registro $2006 che ne fa fare un'altro. Il secondo se avviene
// a pochi cicli cpu dal primo non deve essere considerato.
enum irqA12_misc_value {
	irqA12_min_cpu_cycles_prev_rising_edge = 18
};

#define _irqA12_clock(function)\
	if (nes[nidx].irqA12.cycles > irqA12_min_cpu_cycles_prev_rising_edge) {\
		BYTE *cnt = &nes[nidx].irqA12.counter;\
		BYTE *rld = &nes[nidx].irqA12.reload;\
		nes[nidx].irqA12.cycles = 0;\
		if (nes[nidx].irqA12.race.C001) {\
			cnt = &nes[nidx].irqA12.race.counter;\
			rld = &nes[nidx].irqA12.race.reload;\
		}\
		if (!(*cnt)) {\
			(*cnt) = nes[nidx].irqA12.latch;\
			if (!(*cnt) && (*rld)) {\
				nes[nidx].irqA12.save_counter = 1;\
			}\
			(*rld) = FALSE;\
		} else {\
			(*cnt)--;\
		}\
		if (!(*cnt) && nes[nidx].irqA12.save_counter && nes[nidx].irqA12.enable) {\
			function;\
		}\
		nes[nidx].irqA12.save_counter = (*cnt);\
	}
#define irqA12_irq_default()\
	/*\
	 * visto che (per la sincronizzazione tra cpu e ppu)\
	 * sono in anticipo di un clock cpu, in caso mi trovo\
	 * in questo momento al penultimo ciclo cpu dell'istruzione\
	 * allora devo trattarlo come l'ultimo ritardando\
	 * l'esecuzione dell'IRQ.\
	 */\
	nes[nidx].irqA12.delay = irqA12_delay;\
	if (nes[nidx].c.cpu.cycles == 2) {\
		nes[nidx].irqA12.delay++;\
	}
#define irqA12_clock() _irqA12_clock(irqA12_irq_default())
#define irqA12_mod(function) _irqA12_clock(function)

typedef struct _irqA12 {
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

/* questo non e' necessario salvarlo */
extern BYTE irqA12_delay;

void irqA12_IO(BYTE nidx, WORD value, WORD value_old);
void irqA12_BS(BYTE nidx);
void irqA12_SB(BYTE nidx);
void irqA12_RS(BYTE nidx);

#endif /* IRQA12_H_ */
