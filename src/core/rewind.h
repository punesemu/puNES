/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#ifndef REWIND_H_
#define REWIND_H_

#include "common.h"

enum rewind_options {
	RWND_0_MINUTES,
	RWND_2_MINUTES,
	RWND_5_MINUTES,
	RWND_15_MINUTES,
	RWND_30_MINUTES,
	RWND_60_MINUTES,
	RWND_UNLIMITED_MINUTES,
};
enum rewind_directions {
	RWND_BACKWARD,
	RWND_FORWARD
};
enum rewind_action {
	RWND_ACT_PLAY,
	RWND_ACT_PAUSE,
	RWND_ACT_STEP_BACKWARD,
	RWND_ACT_FAST_BACKWARD,
	RWND_ACT_STEP_FORWARD,
	RWND_ACT_FAST_FORWARD
};

typedef struct _rewind {
	BYTE active;
	BYTE direction;
	BYTE action;
	BYTE action_before_pause;

	struct _rewind_factor {
		int backward;
		int forward;
	} factor;
} _rewind;

extern _rewind rwnd;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE rewind_init(void);
EXTERNC void rewind_quit(void);

EXTERNC void rewind_snapshoot(void);
EXTERNC void rewind_frames(int32_t frames_to_rewind);

EXTERNC void rewind_save_state_snap(BYTE mode);

EXTERNC void rewind_init_operation(void);
EXTERNC void rewind_close_operation(void);

EXTERNC BYTE rewind_is_first_snap(void);
EXTERNC BYTE rewind_is_last_snap(void);

EXTERNC int32_t rewind_max_buffered_snaps(void);
EXTERNC int32_t rewind_count_snaps(void);
EXTERNC int32_t rewind_snap_cursor(void);
EXTERNC int32_t rewind_calculate_snap_cursor(int factor, BYTE direction);

#undef EXTERNC

#endif /* REWIND_H_ */
