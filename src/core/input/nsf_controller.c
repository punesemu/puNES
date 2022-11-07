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

#include "input/nsf_controller.h"
#include "gui.h"
#include "nsf.h"

void input_add_event_nsf_controller(BYTE index) {
	js_jdev_read_port(&js[index], &port[index]);
}
BYTE input_decode_event_nsf_controller(BYTE mode, BYTE autorepeat, DBWORD event, BYTE type, _port *prt) {
	if (autorepeat == TRUE) {
		return (EXIT_OK);
	}

	if (event == prt->input[type][BUT_A]) {
		if (!prt->turbo[TURBOA].active) {
			prt->data[BUT_A] = mode;
		}
		nsf.timers.button[BUT_A] = 0;
		return (EXIT_OK);
	} else if (event == prt->input[type][BUT_B]) {
		if (!prt->turbo[TURBOB].active) {
			prt->data[BUT_B] = mode;
		}
		nsf.timers.button[BUT_B] = 0;
		return (EXIT_OK);
	} else if (event == prt->input[type][SELECT]) {
		prt->data[SELECT] = mode;
		nsf.timers.button[SELECT] = 0;
		return (EXIT_OK);
	} else if (event == prt->input[type][START]) {
		prt->data[START] = mode;
		nsf.timers.button[START] = 0;
		return (EXIT_OK);
	} else if (event == prt->input[type][UP]) {
		prt->data[UP] = mode;
		nsf.timers.button[UP] = 0;
		return (EXIT_OK);
	} else if (event == prt->input[type][DOWN]) {
		prt->data[DOWN] = mode;
		nsf.timers.button[DOWN] = 0;
		return (EXIT_OK);
	} else if (event == prt->input[type][LEFT]) {
		if (mode == PRESSED) {
			prt->data[RIGHT] = RELEASED;
		}
		prt->data[LEFT] = mode;
		nsf.timers.button[LEFT] = 0;
		return (EXIT_OK);
	} else if (event == prt->input[type][RIGHT]) {
		prt->data[RIGHT] = mode;
		if (mode == PRESSED) {
			prt->data[LEFT] = RELEASED;
		}
		nsf.timers.button[RIGHT] = 0;
		return (EXIT_OK);
	}
	return (EXIT_ERROR);
}
