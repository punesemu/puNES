/*
 * savestate.h
 *
 *  Created on: 11/ago/2011
 *      Author: fhorse
 */

#ifndef SAVESTATE_H_
#define SAVESTATE_H_

#include <stdio.h>
#include "common.h"

#define SSAVAILABLE 6

enum { SSSAVE, SSREAD, SSCOUNT };

#define savestateEle(mode, slot, src)\
	if (savestateElementStruct(mode, slot, (uintptr_t *) &src, sizeof(src), fp, FALSE)) {\
		return (EXIT_ERROR);\
	}
#define savestateMem(mode, slot, src, size, preview)\
	if (savestateElementStruct(mode, slot, (uintptr_t *) src, size, fp, preview)) {\
		return (EXIT_ERROR);\
	}
#define savestateInt(mode, slot, value)\
	switch(mode) {\
		case SSSAVE: {\
			uint32_t uint32 = value;\
			savestateEle(mode, slot, uint32);\
			break;\
		}\
		case SSREAD: {\
			uint32_t uint32 = 0;\
			savestateEle(mode, slot, uint32);\
			value = uint32;\
			break;\
		}\
		case SSCOUNT:\
			savestate.totSize[slot] += sizeof(uint32_t);\
			break;\
	}
#define savestatePos(mode, slot, start, end)\
	switch(mode) {\
		case SSSAVE: {\
			uint32_t bank = 0;\
			bank = end - start;\
			savestateEle(mode, slot, bank);\
			break;\
		}\
		case SSREAD: {\
			uint32_t bank = 0;\
			savestateEle(mode, slot, bank);\
			end = start + bank;\
			break;\
		}\
		case SSCOUNT:\
			savestate.totSize[slot] += sizeof(uint32_t);\
			break;\
	}
#define savestateSquare(square, slot)\
	savestateEle(mode, slot, square.timer)\
	savestateEle(mode, slot, square.frequency);\
	savestateEle(mode, slot, square.duty);\
	savestateEle(mode, slot, square.envelope.enabled);\
	savestateEle(mode, slot, square.envelope.divider);\
	savestateEle(mode, slot, square.envelope.counter);\
	savestateEle(mode, slot, square.envelope.constant_volume);\
	savestateEle(mode, slot, square.envelope.delay);\
	savestateEle(mode, slot, square.volume);\
	savestateEle(mode, slot, square.sequencer);\
	savestateEle(mode, slot, square.sweep.enabled);\
	savestateEle(mode, slot, square.sweep.negate);\
	savestateEle(mode, slot, square.sweep.divider);\
	savestateEle(mode, slot, square.sweep.shift);\
	savestateEle(mode, slot, square.sweep.reload);\
	/* ho aggiunto una nuova variabile */\
	if (savestate.version > 7) {\
		savestateEle(mode, slot, square.sweep.silence);\
	}\
	savestateEle(mode, slot, square.sweep.delay);\
	savestateEle(mode, slot, square.length.value);\
	savestateEle(mode, slot, square.length.enabled);\
	savestateEle(mode, slot, square.length.halt);\
	savestateEle(mode, slot, square.output)

struct _savestate {
	uint32_t version;
	DBWORD slot;
	BYTE slotState[SSAVAILABLE];
	DBWORD totSize[SSAVAILABLE];
	DBWORD preview[SSAVAILABLE];
	BYTE previewStart;
} savestate;

BYTE savestateSave(void);
BYTE savestateLoad(void);
void savestatePreview(BYTE slot);
void savestateCountLoad(void);
BYTE savestateElementStruct(BYTE mode, BYTE slot, uintptr_t *src, DBWORD size, FILE *fp,
        BYTE preview);

#endif /* SAVESTATE_H_ */
