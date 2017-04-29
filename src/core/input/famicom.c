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

#include "input/famicom.h"
#include "apu.h"
#include "info.h"
#include "conf.h"
#include "input/standard.h"
#include "input/zapper.h"
#include "input/snes_mouse.h"
#include "input/arkanoid.h"

static void INLINE input_rd_famicom_standard(BYTE *value, BYTE nport, BYTE shift);

BYTE input_wr_reg_famicom(BYTE value) {
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
		}
	}

	if (cfg->input.expansion == CTRL_ARKANOID_PADDLE) {
		input_wr_arkanoid(&value, 0);
	}

	// restituisco il nuovo valore del $4016
	return (value);
}
BYTE input_rd_reg_famicom(BYTE openbus, WORD **screen_index, BYTE nport) {
	BYTE i, value = 0, count = 1;

	// se avviene un DMA del DMC all'inizio
	// dell'istruzione di lettura del registro,
	// avverranno due letture.
	if (!info.r4016_dmc_double_read_disabled && (DMC.dma_cycle == 2)) {
		count = 2;
	}

	for (i = 0; i < count; i++) {
		BYTE np = nport;

		value = 0;

		switch (port[np].type) {
			case CTRL_DISABLED:
				break;
			case CTRL_STANDARD:
				input_rd_famicom_standard(&value, np, 0);
				break;
			case CTRL_SNES_MOUSE:
				input_rd_snes_mouse(&value, np, 0);
				break;
		}

		switch (cfg->input.expansion) {
			case CTRL_STANDARD:
				np += 2;
				if (port[np].type == CTRL_DISABLED) {
					break;
				} else {
					input_rd_famicom_standard(&value, np, 1);
				}
				break;
			case CTRL_ZAPPER:
				if (np == PORT2) {
					input_rd_zapper(&value, np, screen_index);
				}
				break;
			case CTRL_ARKANOID_PADDLE:
				input_rd_arkanoid(&value, np);
				break;
		}
	}

	// Famicom $4016:
	// 7  bit  0
	// ---- ----
	// oooo oMFD
	// |||| ||||
	// |||| |||+- Player 1 serial controller data
	// |||| ||+-- If connected to expansion port (and available), player 3 serial controller data (0 otherwise)
	// |||| |+--- Microphone in controller 2 on traditional Famicom, 0 on AV Famicom
	// ++++-+---- Open bus
	if (nport == PORT1) {
		return ((openbus & 0xF8) | value);
	}

	// Famicom $4017:
	// 7  bit  0
	// ---- ----
	// oooX XXFD
	// |||| ||||
	// |||| |||+- Player 2 serial controller data
	// |||| ||+-- If connected to expansion port, player 4 serial controller data (0 otherwise)
	// |||+-+++-- Returns 0 unless something is plugged into the Famicom expansion port
	// +++------- Open bus
	return ((openbus & 0xE0) | value);
}

static void INLINE input_rd_famicom_standard(BYTE *value, BYTE nport, BYTE shift) {
	(*value) |= (port[nport].data[port[nport].index] << shift);

	// Se $4016 e' a 1 leggo solo lo stato del primo pulsante
	// del controller. Quando verra' scritto 0 nel $4016
	// riprendero' a leggere lo stato di tutti i pulsanti.
	if (!r4016.value) {
		if (++port[nport].index >= 24) {
			port[nport].index = 0;
		}
	}
}
