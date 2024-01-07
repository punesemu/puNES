/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#ifndef INPUT_STANDARD_CONTROLLER_H_
#define INPUT_STANDARD_CONTROLLER_H_

#include "input.h"

typedef struct _input_lfud_standard_controller {
	BYTE left;
	BYTE right;
	BYTE up;
	BYTE down;
} _input_lfud_standard_controller;
typedef struct _permit_axes_standard_controller {
	BYTE axis;
	BYTE delay;
} _permit_axes_standard_controller;
typedef struct _permit_updown_leftright_standard_controller {
	_permit_axes_standard_controller up_or_down[PORT_MAX];
	_permit_axes_standard_controller left_or_right[PORT_MAX];
} _permit_updown_leftright_standard_controller;

extern _permit_updown_leftright_standard_controller permit;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void input_wr_standard_controller(BYTE nidx, const BYTE *value, BYTE nport);
EXTERNC void input_rd_standard_controller(BYTE nidx, BYTE *value, BYTE nport, BYTE shift);

EXTERNC void input_add_event_standard_controller(BYTE index);
EXTERNC BYTE input_decode_event_standard_controller(BYTE mode, BYTE autorepeat, DBWORD event, BYTE type, _port *prt);

EXTERNC void input_rd_standard_controller_vs(BYTE nidx, BYTE *value, BYTE nport, BYTE shift);

EXTERNC void input_data_set_standard_controller(BYTE index, BYTE value, _port *prt);
EXTERNC void input_rotate_standard_controller(_input_lfud_standard_controller *lfud);
EXTERNC void input_updown_leftright_standard_controller(BYTE index, BYTE nport);

#undef EXTERNC

#endif /* INPUT_STANDARD_CONTROLLER_H_ */
