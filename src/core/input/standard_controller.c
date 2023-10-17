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

#include "input/standard_controller.h"
#include "info.h"
#include "conf.h"
#include "nes.h"
#include "gui.h"
#include "vs_system.h"
#include "video/gfx.h"
#include "tas.h"

INLINE static void input_turbo_buttons_standard_controller(_port *prt);
INLINE static void input_reverse_button_standard_controller(BYTE **b0, BYTE **b1);

void input_wr_standard_controller(BYTE nidx, const BYTE *value, BYTE nport) {
	if ((nes[nidx].c.input.r4016 & 0x01) || ((*value) & 0x01)) {
		nes[nidx].c.input.pindex[nport] = 0;
	}
}
void input_rd_standard_controller(BYTE nidx, BYTE *value, BYTE nport, BYTE shift) {
	(*value) = port[nport].data[nes[nidx].c.input.pindex[nport]] << shift;

	// Se $4016 e' a 1 leggo solo lo stato del primo pulsante
	// del controller. Quando verra' scritto 0 nel $4016
	// riprendero' a leggere lo stato di tutti i pulsanti.
	if (!(nes[nidx].c.input.r4016 & 0x01) && (++nes[nidx].c.input.pindex[nport] >= sizeof(port[nport].data))) {
		nes[nidx].c.input.pindex[nport] = 0;
	}
}

void input_add_event_standard_controller(BYTE index) {
	js_jdev_read_port(&jsp[index], &port[index]);
	input_turbo_buttons_standard_controller(&port[index]);
}
BYTE input_decode_event_standard_controller(BYTE mode, UNUSED(BYTE autorepeat), DBWORD event, BYTE type, _port *prt) {
	BYTE *left = &prt->data[LEFT], *right = &prt->data[RIGHT];
	BYTE *up = &prt->data[UP], *down = &prt->data[DOWN];
	BYTE *select = &prt->data[SELECT], *start = &prt->data[START];

	if (tas.type) {
		return (EXIT_OK);
	}

	if ((cfg->screen_rotation | cfg->hflip_screen) && cfg->input_rotation) {
		switch (cfg->screen_rotation) {
			default:
			case ROTATE_0:
				if (cfg->hflip_screen) {
					input_reverse_button_standard_controller(&left, &right);
				}
				break;
			case ROTATE_90:
				input_reverse_button_standard_controller(&left, &down);
				input_reverse_button_standard_controller(&right, &up);
				if (!cfg->hflip_screen) {
					input_reverse_button_standard_controller(&up, &down);
				}
				break;
			case ROTATE_180:
				input_reverse_button_standard_controller(&up, &down);
				if (!cfg->hflip_screen) {
					input_reverse_button_standard_controller(&left, &right);
				}
				break;
			case ROTATE_270:
				input_reverse_button_standard_controller(&left, &up);
				input_reverse_button_standard_controller(&right, &down);
				if (!cfg->hflip_screen) {
					input_reverse_button_standard_controller(&up, &down);
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
		(*select) = mode;
		return (EXIT_OK);
	} else if (event == prt->input[type][START]) {
		(*start) = mode;
		return (EXIT_OK);
	} else if (event == prt->input[type][UP]) {
		(*up) = ((*down) == PRESSED) && !cfg->input.permit_updown_leftright ? RELEASED : mode;
		return (EXIT_OK);
	} else if (event == prt->input[type][DOWN]) {
		(*down) = ((*up) == PRESSED) && !cfg->input.permit_updown_leftright ? RELEASED : mode;
		return (EXIT_OK);
	} else if (event == prt->input[type][LEFT]) {
		(*left) = ((*right) == PRESSED) && !cfg->input.permit_updown_leftright ? RELEASED : mode;
		return (EXIT_OK);
	} else if (event == prt->input[type][RIGHT]) {
		(*right) = ((*left) == PRESSED) && !cfg->input.permit_updown_leftright ? RELEASED : mode;
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

void input_rd_standard_controller_vs(BYTE nidx, BYTE *value, BYTE nport, BYTE shift) {
	BYTE index = nes[nidx].c.input.pindex[nport];
	BYTE protection = FALSE;
	BYTE np = nport;

	if (index == START) {
		index = SELECT;
		protection = vs_system.special_mode.type == VS_SM_Ice_Climber;
	} else if (index == SELECT) {
		index = START;
	} else if (info.mapper.expansion == EXP_VS_1P_R4017) {
		np ^= 0x01;
	}
	(*value) = (protection ? PRESSED : port[np].data[index]) << shift;

	// Se $4016 e' a 1 leggo solo lo stato del primo pulsante
	// del controller. Quando verra' scritto 0 nel $4016
	// riprendero' a leggere lo stato di tutti i pulsanti.
	if (!(nes[nidx].c.input.r4016 & 0x01) && (++nes[nidx].c.input.pindex[nport] >= sizeof(port[nport].data))) {
		nes[nidx].c.input.pindex[nport] = 0;
	}
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
INLINE static void input_reverse_button_standard_controller(BYTE **b0, BYTE **b1) {
	BYTE *tmp = (*b0);

	(*b0) = (*b1);
	(*b1) = tmp;
}