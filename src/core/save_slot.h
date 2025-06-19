/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

#ifndef SAVE_SLOT_H_
#define SAVE_SLOT_H_

#include <stdio.h>
#include "common.h"
#include "info.h"

enum save_slot_misc {
	SAVE_SLOTS = 12,
	SAVE_SLOTS_TOTAL = SAVE_SLOTS + 1, // include anche lo slot per il file
	SAVE_SLOT_FILE = SAVE_SLOTS
};
enum save_slot_mode { SAVE_SLOT_SAVE, SAVE_SLOT_READ, SAVE_SLOT_COUNT, SAVE_SLOT_INCDEC };

#define _save_slot_ele(mode, slot, src, size, preview)\
	if (save_slot_element((mode), (slot), (src), (size_t)(size), fp, (preview))) return (EXIT_ERROR)
#define save_slot_ele(mode, slot, src)\
	_save_slot_ele((mode), (slot), (void *)&(src), sizeof(src), FALSE)
#define save_slot_int(mode, slot, value)\
	_save_slot_ele((mode), (slot), (void *)&(value), sizeof(value), FALSE)
#define save_slot_mem(mode, slot, src, size, preview)\
	_save_slot_ele((mode), (slot), (void *)(src), (size_t)(size), (preview))
#define save_slot_pos(mode, slot, start, end)\
	if (save_slot_position((mode), (slot), (void *)(start), (void *)(end), fp)) return (EXIT_ERROR)
#define save_slot_square(square, slot)\
	save_slot_ele(mode, slot, (square).timer);\
	save_slot_ele(mode, slot, (square).frequency);\
	save_slot_ele(mode, slot, (square).duty);\
	save_slot_ele(mode, slot, (square).envelope.enabled);\
	save_slot_ele(mode, slot, (square).envelope.divider);\
	save_slot_ele(mode, slot, (square).envelope.counter);\
	save_slot_ele(mode, slot, (square).envelope.constant_volume);\
	save_slot_ele(mode, slot, (square).envelope.delay);\
	save_slot_ele(mode, slot, (square).volume);\
	save_slot_ele(mode, slot, (square).sequencer);\
	save_slot_ele(mode, slot, (square).sweep.enabled);\
	save_slot_ele(mode, slot, (square).sweep.negate);\
	save_slot_ele(mode, slot, (square).sweep.divider);\
	save_slot_ele(mode, slot, (square).sweep.shift);\
	save_slot_ele(mode, slot, (square).sweep.reload);\
	save_slot_ele(mode, slot, (square).sweep.silence);\
	save_slot_ele(mode, slot, (square).sweep.delay);\
	save_slot_ele(mode, slot, (square).length.value);\
	save_slot_ele(mode, slot, (square).length.enabled);\
	save_slot_ele(mode, slot, (square).length.halt);\
	save_slot_ele(mode, slot, (square).output)

typedef struct _save_slot {
	uint32_t version;
	DBWORD slot_in_use;
	uint32_t crc32;
	uint32_t size;
	struct _save_slots_info {
		BYTE state;
		uint32_t tot_size;
	} slot[SAVE_SLOTS_TOTAL];
} _save_slot;

extern _save_slot save_slot;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE save_slot_save(BYTE slot);
EXTERNC BYTE save_slot_load(BYTE slot);
EXTERNC void save_slot_count_load(void);
EXTERNC BYTE save_slot_element(BYTE mode, BYTE slot, void *src, size_t size, FILE *fp, BYTE preview);
EXTERNC BYTE save_slot_position(BYTE mode, BYTE slot, void *start, void *end, FILE *fp);
EXTERNC BYTE save_slot_operation(BYTE mode, BYTE slot, FILE *fp);

#undef EXTERNC

#endif /* SAVE_SLOT_H_ */
