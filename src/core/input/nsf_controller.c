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

#include "input/nsf_controller.h"
#include "gui.h"
#include "nsf.h"
#include "input/standard_controller.h"

void input_add_event_nsf_controller(BYTE index) {
	js_jdev_read_port(&jsp[index], &port[index]);
}
BYTE input_decode_event_nsf_controller(BYTE mode, BYTE autorepeat, DBWORD event, BYTE type, _port *prt) {
	if (autorepeat) {
		return (EXIT_OK);
	}

	if (event == prt->input[type][BUT_A]) {
		if (!prt->turbo[TURBOA].active) {
			input_data_set_standard_controller(BUT_A, mode, prt);
		}
		nsf.timers.button[BUT_A] = 0;
		return (EXIT_OK);
	} else if (event == prt->input[type][BUT_B]) {
		if (!prt->turbo[TURBOB].active) {
			input_data_set_standard_controller(BUT_B, mode, prt);
		}
		nsf.timers.button[BUT_B] = 0;
		return (EXIT_OK);
	} else if (event == prt->input[type][SELECT]) {
		input_data_set_standard_controller(SELECT, mode, prt);
		nsf.timers.button[SELECT] = 0;
		return (EXIT_OK);
	} else if (event == prt->input[type][START]) {
		input_data_set_standard_controller(START, mode, prt);
		nsf.timers.button[START] = 0;
		return (EXIT_OK);
	} else if (event == prt->input[type][UP]) {
		input_data_set_standard_controller(UP, mode, prt);
		nsf.timers.button[UP] = 0;
		return (EXIT_OK);
	} else if (event == prt->input[type][DOWN]) {
		input_data_set_standard_controller(DOWN, mode, prt);
		nsf.timers.button[DOWN] = 0;
		return (EXIT_OK);
	} else if (event == prt->input[type][LEFT]) {
		input_data_set_standard_controller(LEFT, mode, prt);
		if (mode == PRESSED) {
			input_data_set_standard_controller(RIGHT, RELEASED, prt);
		}
		nsf.timers.button[LEFT] = 0;
		return (EXIT_OK);
	} else if (event == prt->input[type][RIGHT]) {
		input_data_set_standard_controller(RIGHT, mode, prt);
		if (mode == PRESSED) {
			input_data_set_standard_controller(LEFT, RELEASED, prt);
		}
		nsf.timers.button[RIGHT] = 0;
		return (EXIT_OK);
	}
	return (EXIT_ERROR);
}
