/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

#include "input/standard.h"
#include "info.h"
#include "conf.h"
#include "tas.h"
#include "apu.h"
#include "gui.h"
#include "vs_system.h"
#include "input/zapper.h"
#include "input/snes_mouse.h"
#include "input/arkanoid.h"

static void INLINE input_rd_standard(BYTE *value, BYTE nport);
static void INLINE input_turbo_buttons_control_standard(_port *port);

BYTE input_wr_reg_standard(BYTE value) {
	BYTE i;

	// in caso di strobe azzero l'indice
	for (i = PORT1; i < PORT_MAX; i++) {
		switch (port[i].type) {
			case CTRL_DISABLED:
				break;
			case CTRL_STANDARD:
				input_wr_standard(&value, i);
				break;
			case CTRL_SNES_MOUSE:
				input_wr_snes_mouse(&value, i);
				break;
			case CTRL_ARKANOID_PADDLE:
				input_wr_arkanoid(&value, i);
				break;
		}
	}

	// restituisco il nuovo valore del $4016
	return (value);
}
BYTE input_rd_reg_standard(BYTE openbus, WORD **screen_index, BYTE nport) {
	BYTE a, value = 0, count = 1;

	// se avviene un DMA del DMC all'inizio
	// dell'istruzione di lettura del registro,
	// avverranno due letture.
	if (!info.r4016_dmc_double_read_disabled && (DMC.dma_cycle == 2)) {
		count++;
	}

	for (a = 0; a < count; a++) {
		switch (port[nport].type) {
			case CTRL_DISABLED:
				break;
			case CTRL_STANDARD:
				input_rd_standard(&value, nport);
				break;
			case CTRL_ZAPPER:
				input_rd_zapper(&value, nport, screen_index);
				break;
			case CTRL_SNES_MOUSE:
				input_rd_snes_mouse(&value, nport, 0);
				break;
			case CTRL_ARKANOID_PADDLE:
				input_rd_arkanoid(&value, nport);
				break;
		}
	}

	// NES-001 (front-loading NES) $4016 and $4017, and NES-101 (top-loading NES) $4016 and $4017
	// 7  bit  0
	// ---- ----
	// oooX XZXD
	// |||| ||||
	// |||| |||+- Serial controller data
	// |||+-+-+-- Always 0
	// |||   +--- Open bus on NES-101 $4016; 0 otherwise
	// +++------- Open bus
	return ((openbus & 0xE0) | value);
}
void input_wr_standard(BYTE *value, BYTE nport) {
	if ((r4016.value & 0x01) || ((*value) & 0x01)) {
		port[nport].index = 0;
	}
}

BYTE input_wr_reg_standard_vs(BYTE value) {
	BYTE i;

	vs_system.shared_mem = value & 0x02;

	// in caso di strobe azzero l'indice
	for (i = PORT1; i < PORT_MAX; i++) {
		input_wr_standard(&value, i);
	}

	// restituisco il nuovo valore del $4016
	return (value);
}
BYTE input_rd_reg_standard_vs(BYTE openbus, WORD **screen_index, BYTE nport) {
	BYTE i, value = 0, count = 1, np = !nport;

	// se avviene un DMA del DMC all'inizio
	// dell'istruzione di lettura del registro,
	// avverranno due letture.
	if (!info.r4016_dmc_double_read_disabled && (DMC.dma_cycle == 2)) {
		count++;
	}

	for (i = 0; i < count; i++) {
		// Se $4016 e' a 1 leggo solo lo stato del primo pulsante
		// del controller. Quando verra' scritto 0 nel $4016
		// riprendero' a leggere lo stato di tutti i pulsanti.
		input_rd_standard(&value, np);
	}

	vs_system_r4016_r4017(nport)
}

void input_add_event_standard(BYTE index) {
	js_control(&js[index], &port[index]);
	input_turbo_buttons_control_standard(&port[index]);
}
BYTE input_decode_event_standard(BYTE mode, DBWORD event, BYTE type, _port *port) {
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

static void INLINE input_rd_standard(BYTE *value, BYTE nport) {
	(*value) = port[nport].data[port[nport].index];

	// Se $4016 e' a 1 leggo solo lo stato del primo pulsante
	// del controller. Quando verra' scritto 0 nel $4016
	// riprendero' a leggere lo stato di tutti i pulsanti.
	if (!r4016.value) {
		if (++port[nport].index >= 24) {
			port[nport].index = 0;
		}
	}
}
static void INLINE input_turbo_buttons_control_standard(_port *port) {
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
