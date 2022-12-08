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

#include "input/standard_controller.h"
#include "conf.h"
#include "gui.h"
#include "video/gfx.h"
#include "tas.h"

INLINE static void input_turbo_buttons_standard_controller(_port *prt);

void input_wr_standard_controller(const BYTE *value, BYTE nport) {
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
	js_jdev_read_port(&jsp[index], &port[index]);
	input_turbo_buttons_standard_controller(&port[index]);
}
BYTE input_decode_event_standard_controller(BYTE mode, UNUSED(BYTE autorepeat), DBWORD event, BYTE type, _port *prt) {
	BYTE *left = &prt->data[LEFT], *right = &prt->data[RIGHT];
	BYTE *up = &prt->data[UP], *down = &prt->data[DOWN];

	if (tas.type) {
		return (EXIT_OK);
	}

	if ((cfg->screen_rotation | cfg->hflip_screen) && cfg->input_rotation) {
		switch (cfg->screen_rotation) {
			default:
			case ROTATE_0:
				if (cfg->hflip_screen) {
					left = &prt->data[RIGHT];
					right = &prt->data[LEFT];
				}
				break;
			case ROTATE_90:
				left = &prt->data[DOWN];
				right = &prt->data[UP];
				if (cfg->hflip_screen) {
					up = &prt->data[RIGHT];
					down = &prt->data[LEFT];
				} else {
					up = &prt->data[LEFT];
					down = &prt->data[RIGHT];
				}
				break;
			case ROTATE_180:
				if (cfg->hflip_screen) {
					left = &prt->data[LEFT];
					right = &prt->data[RIGHT];
				} else {
					left = &prt->data[RIGHT];
					right = &prt->data[LEFT];
				}
				up = &prt->data[DOWN];
				down = &prt->data[UP];
				break;
			case ROTATE_270:
				left = &prt->data[UP];
				right = &prt->data[DOWN];
				if (cfg->hflip_screen) {
					up = &prt->data[LEFT];
					down = &prt->data[RIGHT];
				} else {
					up = &prt->data[RIGHT];
					down = &prt->data[LEFT];
				}
				break;
		}
	}

	if (event == prt->input[type][BUT_A]) {
		if (!prt->turbo[TURBOA].active) {
			prt->data[BUT_A] = mode;
		}
		return (EXIT_OK);
	} else if (event == prt->input[type][BUT_B]) {
		if (!prt->turbo[TURBOB].active) {
			prt->data[BUT_B] = mode;
		}
		return (EXIT_OK);
	} else if (event == prt->input[type][SELECT]) {
		prt->data[SELECT] = mode;
		return (EXIT_OK);
	} else if (event == prt->input[type][START]) {
		prt->data[START] = mode;
		return (EXIT_OK);
	} else if (event == prt->input[type][UP]) {
		(*up) = mode;
		// non possono essere premuti contemporaneamente
		if (!cfg->input.permit_updown_leftright && (mode == PRESSED)) {
			(*down) = RELEASED;
		}
		return (EXIT_OK);
	} else if (event == prt->input[type][DOWN]) {
		(*down) = mode;
		// non possono essere premuti contemporaneamente
		if (!cfg->input.permit_updown_leftright && (mode == PRESSED)) {
			(*up) = RELEASED;
		}
		return (EXIT_OK);
	} else if (event == prt->input[type][LEFT]) {
		(*left) = mode;
		// non possono essere premuti contemporaneamente
		if (!cfg->input.permit_updown_leftright && (mode == PRESSED)) {
			(*right) = RELEASED;
		}
		return (EXIT_OK);
	} else if (event == prt->input[type][RIGHT]) {
		(*right) = mode;
		// non possono essere premuti contemporaneamente
		if (!cfg->input.permit_updown_leftright && (mode == PRESSED)) {
			(*left) = RELEASED;
		}
		return (EXIT_OK);
	} else if (event == prt->input[type][TRB_A]) {
		prt->turbo[TURBOA].mode = mode;
		if (mode == PRESSED) {
			prt->turbo[TURBOA].active = TRUE;
		}
		return (EXIT_OK);
	} else if (event == prt->input[type][TRB_B]) {
		prt->turbo[TURBOB].mode = mode;
		if (mode == PRESSED) {
			prt->turbo[TURBOB].active = TRUE;
		}
		return (EXIT_OK);
	}
	return (EXIT_ERROR);
}

INLINE static void input_turbo_buttons_standard_controller(_port *prt) {
	if ((prt->turbo[TURBOA].mode == PRESSED) || prt->turbo[TURBOA].active) {
		if (++prt->turbo[TURBOA].counter == prt->turbo[TURBOA].frequency) {
			prt->data[BUT_A] = PRESSED;
			prt->turbo[TURBOA].active = TRUE;
		} else if (prt->turbo[TURBOA].counter > prt->turbo[TURBOA].frequency) {
			prt->data[BUT_A] = RELEASED;
			prt->turbo[TURBOA].active = FALSE;
			prt->turbo[TURBOA].counter = 0;
		}
	}
	if ((prt->turbo[TURBOB].mode == PRESSED) || prt->turbo[TURBOB].active) {
		if (++prt->turbo[TURBOB].counter == prt->turbo[TURBOB].frequency) {
			prt->data[BUT_B] = PRESSED;
			prt->turbo[TURBOB].active = TRUE;
		} else if (prt->turbo[TURBOB].counter > prt->turbo[TURBOB].frequency) {
			prt->data[BUT_B] = RELEASED;
			prt->turbo[TURBOB].active = FALSE;
			prt->turbo[TURBOB].counter = 0;
		}
	}
}
