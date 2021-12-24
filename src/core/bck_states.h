/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#ifndef BCK_STATES_H_
#define BCK_STATES_H_

#include <string.h>
#include "common.h"

enum bck_states_operations_mode {
	BCK_STATES_OP_SAVE_ON_MEM,
	BCK_STATES_OP_READ_FROM_MEM,
	BCK_STATES_OP_COUNT
};

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void bck_states_op_screen(BYTE mode, void *data, size_t *index, size_t *size_buff);
EXTERNC void bck_states_op_keyframe(BYTE mode, void *data, size_t *index, size_t *size_buff);
EXTERNC void bck_states_op_input(BYTE mode, void *data, size_t *index, size_t *size_buff);
EXTERNC void bck_states_op_input_port(BYTE port, BYTE mode, void *data, size_t *index, size_t *size_buff);

#undef EXTERNC

#endif /* BCK_STATES_H_ */
