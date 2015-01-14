/*
 * save_slot.h
 *
 *  Created on: 11/ago/2011
 *      Author: fhorse
 */

#ifndef SAVE_SLOT_H_
#define SAVE_SLOT_H_

#include <stdio.h>
#include "common.h"

enum save_slot_misc { SAVE_SLOTS = 6 };
enum save_slot_mode { SAVE_SLOT_SAVE, SAVE_SLOT_READ, SAVE_SLOT_COUNT };

#define save_slot_ele(mode, slot, src)\
	if (save_slot_element_struct(mode, slot, (uintptr_t *) &src, sizeof(src), fp, FALSE)) {\
		return (EXIT_ERROR);\
	}
#define save_slot_mem(mode, slot, src, size, preview)\
	if (save_slot_element_struct(mode, slot, (uintptr_t *) src, size, fp, preview)) {\
		return (EXIT_ERROR);\
	}
#define save_slot_int(mode, slot, value)\
	switch(mode) {\
		case SAVE_SLOT_SAVE: {\
			uint32_t uint32 = value;\
			save_slot_ele(mode, slot, uint32);\
			break;\
		}\
		case SAVE_SLOT_READ: {\
			uint32_t uint32 = 0;\
			save_slot_ele(mode, slot, uint32);\
			value = uint32;\
			break;\
		}\
		case SAVE_SLOT_COUNT:\
			save_slot.tot_size[slot] += sizeof(uint32_t);\
			break;\
	}
#define save_slot_pos(mode, slot, start, end)\
	switch(mode) {\
		case SAVE_SLOT_SAVE: {\
			uint32_t bank = 0;\
			bank = end - start;\
			save_slot_ele(mode, slot, bank);\
			break;\
		}\
		case SAVE_SLOT_READ: {\
			uint32_t bank = 0;\
			save_slot_ele(mode, slot, bank);\
			end = start + bank;\
			break;\
		}\
		case SAVE_SLOT_COUNT:\
			save_slot.tot_size[slot] += sizeof(uint32_t);\
			break;\
	}
#define save_slot_square(square, slot)\
	save_slot_ele(mode, slot, square.timer)\
	save_slot_ele(mode, slot, square.frequency);\
	save_slot_ele(mode, slot, square.duty);\
	save_slot_ele(mode, slot, square.envelope.enabled);\
	save_slot_ele(mode, slot, square.envelope.divider);\
	save_slot_ele(mode, slot, square.envelope.counter);\
	save_slot_ele(mode, slot, square.envelope.constant_volume);\
	save_slot_ele(mode, slot, square.envelope.delay);\
	save_slot_ele(mode, slot, square.volume);\
	save_slot_ele(mode, slot, square.sequencer);\
	save_slot_ele(mode, slot, square.sweep.enabled);\
	save_slot_ele(mode, slot, square.sweep.negate);\
	save_slot_ele(mode, slot, square.sweep.divider);\
	save_slot_ele(mode, slot, square.sweep.shift);\
	save_slot_ele(mode, slot, square.sweep.reload);\
	/* ho aggiunto una nuova variabile */\
	if (save_slot.version > 7) {\
		save_slot_ele(mode, slot, square.sweep.silence);\
	}\
	save_slot_ele(mode, slot, square.sweep.delay);\
	save_slot_ele(mode, slot, square.length.value);\
	save_slot_ele(mode, slot, square.length.enabled);\
	save_slot_ele(mode, slot, square.length.halt);\
	save_slot_ele(mode, slot, square.output)

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC struct _save_slot {
	uint32_t version;
	DBWORD slot;
	BYTE state[SAVE_SLOTS];
	DBWORD tot_size[SAVE_SLOTS];
	DBWORD preview[SAVE_SLOTS];
	BYTE preview_start;
} save_slot;

EXTERNC BYTE save_slot_save(void);
EXTERNC BYTE save_slot_load(void);
EXTERNC void save_slot_preview(BYTE slot);
EXTERNC void save_slot_count_load(void);
EXTERNC BYTE save_slot_element_struct(BYTE mode, BYTE slot, uintptr_t *src, DBWORD size, FILE *fp,
		BYTE preview);

#undef EXTERNC

#endif /* SAVE_SLOT_H_ */
