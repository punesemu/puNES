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

#include <string.h>
#include "input/nsf_mouse.h"
#include "input/mouse.h"
#include "input/standard_controller.h"
#include "nsf.h"

struct _nsf_mouse {
	BYTE pressed;
} nsf_mouse;

void input_init_nsf_mouse(void) {
	memset(&nsf_mouse, 0x00, sizeof(nsf_mouse));
}
void input_add_event_nsf_mouse(UNUSED(BYTE index)) {
	int x, y;

	if (gmouse.left) {
		if (nsf_mouse.pressed) {
			return;
		} else {
			input_read_mouse_coords(&x, &y);
			nsf_controls_mouse_in_gui(x, y);
			nsf_mouse.pressed = TRUE;
		}
		return;
	}

	if (nsf_mouse.pressed) {
		_port *prt = &port[PORT1];

		input_data_set_standard_controller(START, RELEASED, prt);
		input_data_set_standard_controller(SELECT, RELEASED, prt);
		input_data_set_standard_controller(LEFT, RELEASED, prt);
		input_data_set_standard_controller(RIGHT, RELEASED, prt);
		input_data_set_standard_controller(BUT_A, RELEASED, prt);
		input_data_set_standard_controller(BUT_B, RELEASED, prt);
		nsf_mouse.pressed = FALSE;
	}
}
