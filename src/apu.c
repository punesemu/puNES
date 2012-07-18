/*
 * apu.c
 *
 *  Created on: 06/set/2010
 *      Author: fhorse
 */

#include <string.h>
#include "apu.h"
#include "ppu.h"
#include "cpu6502.h"
#include "clock.h"
#include "sdlsnd.h"
#include "memmap.h"
#include "fds.h"
#include "audio_filter.h"

BYTE sflag = 0;

void apuTick(SWORD cyclesCPU, BYTE *hwtick) {
	/* sottraggo il numero di cicli eseguiti */
	apu.cycles -= cyclesCPU;
	/*
	 * questo flag sara' a TRUE solo nel ciclo
	 * in cui viene eseguito il length counter.
	 */
	apu.length_clocked = FALSE;
	/*
	 * se e' settato il delay del $4017, essendo
	 * questo il ciclo successivo, valorizzo il
	 * registro.
	 */
	if (r4017.delay) {
		r4017.delay = FALSE;
		r4017jitter();
	}
	/* quando apuCycles e' a 0 devo eseguire uno step */
	if (!apu.cycles) {
		switch (apu.step) {
			case 0:
				/*
				 * nel mode 1 devo eseguire il
				 * length counter e lo sweep.
				 */
				if (apu.mode == APU48HZ) {
					lengthClock()
					sweepClock()
				}
				envelopeClock()
				/* triangle's linear counter */
				linearClock()
				/* passo al prossimo step */
				apuChangeStep(++apu.step);
				break;
			case 1:
				/* nel mode 0 devo eseguire il length counter */
				if (apu.mode == APU60HZ) {
					lengthClock()
					sweepClock()
				}
				envelopeClock()
				/* triangle's linear counter */
				linearClock()
				/* passo al prossimo step */
				apuChangeStep(++apu.step);
				break;
			case 2:
				/*
				 * nel mode 1 devo eseguire il
				 * length counter e lo sweep.
				 */
				if (apu.mode == APU48HZ) {
					lengthClock()
					sweepClock()
				}
				envelopeClock()
				/* triangle's linear counter */
				linearClock()
				/* passo al prossimo step */
				apuChangeStep(++apu.step);
				break;
			case 3:
				/*
				 * gli step 3, 4 e 5 settano il bit 6 del $4015
				 * ma solo nel 4 genero un IRQ.
				 */
				if (apu.mode == APU60HZ) {
					/*
					 * se e' a 0 il bit 6 del $4017 (interrupt
					 * inhibit flag) allora devo generare un IRQ.
					 */
					if (!(r4017.value & 0x40)) {
						/* setto il bit 6 del $4015 */
						r4015.value |= 0x40;
					}
				} else {
					/* nel mode 1 devo eseguire l'envelope */
					envelopeClock()
					/* triangle's linear counter */
					linearClock()
				}
				/* passo al prossimo step */
				apuChangeStep(++apu.step);
				break;
			case 4:
				/*
				 * gli step 3, 4 e 5 settano il bit 6 del $4015
				 * ma solo nel 4 genero un IRQ.
				 */
				if (apu.mode == APU60HZ) {
					lengthClock()
					sweepClock()
					envelopeClock()
					/* triangle's linear counter */
					linearClock()
					/*
					 * se e' a 0 il bit 6 del $4017 (interrupt
					 * inhibit flag) allora devo generare un IRQ.
					 */
					if (!(r4017.value & 0x40)) {
						/* setto il bit 6 del $4015 */
						r4015.value |= 0x40;
						/* abilito l'IRQ del frame counter */
						irq.high |= APUIRQ;
					}
				}
				/* passo al prossimo step */
				apuChangeStep(++apu.step);
				break;
			case 5:
				/*
				 * gli step 3, 4 e 5 settano il bit 6 del $4015
				 * ma solo nel 4 genero un IRQ.
				 */
				if (apu.mode == APU60HZ) {
					/*
					 * se e' a 0 il bit 6 del $4017 (interrupt
					 * inhibit flag) allora devo generare un IRQ.
					 */
					if (!(r4017.value & 0x40)) {
						/* setto il bit 6 del $4015 */
						r4015.value |= 0x40;
					}
					apu.step++;
				} else {
					/* nel mode 1 devo ricominciare il ciclo */
					apu.step = 0;
				}
				/* passo al prossimo step */
				apuChangeStep(apu.step);
				break;
			case 6:
				/* da qui ci passo solo nel mode 0 */
				envelopeClock()
				/* triangle's linear counter */
				linearClock()
				/* questo e' il passaggio finale del mode 0 */
				apu.step = 1;
				/* passo al prossimo step */
				apuChangeStep(apu.step);
				break;
		}
	}

	/*
	 * eseguo un ticket per ogni canale
	 * valorizzandone l'output.
	 */
	squareTick(S1)
	squareTick(S2)
	triangleTick()
	noiseTick()
	dmcTick();
	if (extclApuTick) {
		/*
		 * utilizzato dalle mappers :
		 * FDS
		 * MMC5
		 * Namcot
		 * Sunsoft
		 * VRC6
		 */
		extclApuTick();
	}

	/* tick filtro audio */
	audio_filter_apu_tick();

	sndWrite();
}

void apuTurnON(void) {
	if (info.reset >= HARD) {
		memset(&apu, 0x00, sizeof(apu));
		memset(&r4015, 0x00, sizeof(r4015));
		memset(&r4017, 0x00, sizeof(r4017));
		/* azzero tutte le variabili interne dei canali */
		memset(&S1, 0x00, sizeof(S1));
		memset(&S2, 0x00, sizeof(S2));
		memset(&TR, 0x00, sizeof(TR));
		memset(&NS, 0x00, sizeof(NS));
		memset(&DMC, 0x00, sizeof(DMC));
		/* al reset e' sempre settato a 60Hz */
		apu.mode = APU60HZ;
		if (machine.type == NTSC) {
			apu.type = 0;
		} else if (machine.type == DENDY) {
			apu.type = 2;
		} else {
			apu.type = 1;
		}
		apuChangeStep(apu.step);
		/* valori iniziali dei vari canali */
		S1.frequency = 1;
		S2.frequency = 1;
		TR.frequency = 1;
		TR.sequencer = 7;
		NS.frequency = 1;
		NS.shift = 1;
		DMC.frequency = 1;
		DMC.empty = TRUE;
		DMC.silence = TRUE;
		DMC.counterOut = 8;
	} else {
		r4017.delay = FALSE;
		r4017jitter();
		r4015.value = 0;
		S1.length.enabled = 0;
		S1.length.value = 0;
		S2.length.enabled = 0;
		S2.length.value = 0;
		TR.length.enabled = 0;
		TR.length.value = 0;
		TR.sequencer = 7;
		NS.length.enabled = 0;
		NS.length.value = 0;
		DMC.remain = 0;
		DMC.silence = TRUE;
		DMC.frequency = 1;
		DMC.counterOut = 8;
	}
}
