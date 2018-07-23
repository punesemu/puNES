/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#include "input/standard_controller.h"
#include "conf.h"
#include "gui.h"
#include "tas.h"

static void INLINE input_turbo_buttons_standard_controller(_port *port);

void input_wr_standard_controller(BYTE *value, BYTE nport) {
	if ((r4016.value & 0x01) || ((*value) & 0x01)) {
		port[nport].index = 0;
	}
}
void input_rd_standard_controller(BYTE *value, BYTE nport, BYTE shift) {
	(*value) = port[nport].data[port[nport].index] << shift;

	// Se $4016 e' a 1 leggo solo lo stato del primo pulsante
	// del controller. Quando verra' scritto 0 nel $4016
	// riprendero' a leggere lo stato di tutti i pulsanti.
	if (!(r4016.value & 0x01) && (++port[nport].index >= sizeof(port[nport].data))) {
		port[nport].index = 0;
	}
}

void input_add_event_standard_controller(BYTE index) {
	js_control(&js[index], &port[index]);
	input_turbo_buttons_standard_controller(&port[index]);
}
BYTE input_decode_event_standard_controller(BYTE mode, BYTE autorepeat, DBWORD event, BYTE type,
		_port *port) {
	if (tas.type) {
		return (EXIT_OK);
	} else if (event == port->input[type][BUT_A]) {
		if (!port->turbo[TURBOA].active) {
			port->data[BUT_A] = mode;
		}
		return (EXIT_OK);
	} else if (event == port->input[type][BUT_B]) {
		if (!port->turbo[TURBOB].active) {
			port->data[BUT_B] = mode;
		}
		return (EXIT_OK);
	} else if (event == port->input[type][SELECT]) {
		port->data[SELECT] = mode;
		return (EXIT_OK);
	} else if (event == port->input[type][START]) {
		port->data[START] = mode;
		return (EXIT_OK);
	} else if (event == port->input[type][UP]) {
		port->data[UP] = mode;
		// non possono essere premuti contemporaneamente
		if ((cfg->input.permit_updown_leftright == FALSE) && (mode == PRESSED)) {
			port->data[DOWN] = RELEASED;
		}
		return (EXIT_OK);
	} else if (event == port->input[type][DOWN]) {
		port->data[DOWN] = mode;
		// non possono essere premuti contemporaneamente
		if ((cfg->input.permit_updown_leftright == FALSE) && (mode == PRESSED)) {
			port->data[UP] = RELEASED;
		}
		return (EXIT_OK);
	} else if (event == port->input[type][LEFT]) {
		port->data[LEFT] = mode;
		// non possono essere premuti contemporaneamente
		if ((cfg->input.permit_updown_leftright == FALSE) && (mode == PRESSED)) {
			port->data[RIGHT] = RELEASED;
		}
		return (EXIT_OK);
	} else if (event == port->input[type][RIGHT]) {
		port->data[RIGHT] = mode;
		// non possono essere premuti contemporaneamente
		if ((cfg->input.permit_updown_leftright == FALSE) && (mode == PRESSED)) {
			port->data[LEFT] = RELEASED;
		}
		return (EXIT_OK);
	} else if (event == port->input[type][TRB_A]) {
		if (mode == PRESSED) {
			//if (!(port->turbo[TURBOA].active = !port->turbo[TURBOA].active)) {
			//	port->data[BUT_A] = RELEASED;
			//	port->turbo[TURBOA].counter = 0;
			//}
			port->turbo[TURBOA].active = TRUE;
		} else {
			port->turbo[TURBOA].active = FALSE;
			port->data[BUT_A] = RELEASED;
			port->turbo[TURBOA].counter = 0;
		}
		return (EXIT_OK);
	} else if (event == port->input[type][TRB_B]) {
		if (mode == PRESSED) {
			//if (!(port->turbo[TURBOB].active = !port->turbo[TURBOB].active)) {
			//	port->data[BUT_B] = RELEASED;
			//	port->turbo[TURBOB].counter = 0;
			//}
			port->turbo[TURBOB].active = TRUE;
		} else {
			port->turbo[TURBOB].active = FALSE;
			port->data[BUT_B] = RELEASED;
			port->turbo[TURBOB].counter = 0;
		}
		return (EXIT_OK);
	}
	return (EXIT_ERROR);
}

static void INLINE input_turbo_buttons_standard_controller(_port *port) {
	if (port->turbo[TURBOA].active) {
		if (++port->turbo[TURBOA].counter == port->turbo[TURBOA].frequency) {
			port->data[BUT_A] = PRESSED;
		} else if (port->turbo[TURBOA].counter > port->turbo[TURBOA].frequency) {
			port->data[BUT_A] = RELEASED;
			port->turbo[TURBOA].counter = 0;
		}
	}
	if (port->turbo[TURBOB].active) {
		if (++port->turbo[TURBOB].counter == port->turbo[TURBOB].frequency) {
			port->data[BUT_B] = PRESSED;
		} else if (port->turbo[TURBOB].counter > port->turbo[TURBOB].frequency) {
			port->data[BUT_B] = RELEASED;
			port->turbo[TURBOB].counter = 0;
		}
	}
}

