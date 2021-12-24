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

#include "input/vs.h"
#include "info.h"
#include "conf.h"
#include "vs_system.h"

INLINE static void _input_rd_reg_vs(BYTE *value, BYTE nport);

BYTE input_wr_reg_vs(BYTE value) {
	vs_system.shared_mem = value & 0x02;

	// in caso di strobe azzero l'indice
	port_funct[PORT1].input_wr(&value, PORT1);
	port_funct[PORT2].input_wr(&value, PORT2);

	// restituisco il nuovo valore del $4016
	return (value);
}
BYTE input_rd_reg_vs_r4016(UNUSED(BYTE openbus), BYTE nport) {
	BYTE value = 0;

	_input_rd_reg_vs(&value, !nport);

	// port $4016
	// 7  bit  0
	// ---- ----
	// xCCD DSxB
	//  ||| || |
	//  ||| || +- Buttons for player 2 (A, B, 1, 3, Up, Down, Left, Right)
	//  ||| |+--- Service button (commonly inserts a credit)
	//  ||+-+---- DIP switches "2" and "1", respectively
	//  ++------- Coin inserted (read below)
	return ((vs_system.coins.right ? 0x40 : 0x00) |
			(vs_system.coins.left ? 0x20 : 0x00) |
			((cfg->dipswitch & 0x03) << 3) |
			(vs_system.coins.service ? 0x04 : 0x00) |
			(value & 0x01));
}
BYTE input_rd_reg_vs_r4017(UNUSED(BYTE openbus), BYTE nport) {
	BYTE value = 0;

	vs_system.watchdog.timer = 0;
	_input_rd_reg_vs(&value, !nport);

	// port $4017
	// 7  bit  0
	// ---- ----
	// DDDD DDxB
	// |||| || |
	// |||| || +- Buttons for player 1 (A, B, 2, 4, Up, Down, Left, Right)
	// ++++-++--- More DIP switches (7 down to 2)
	return ((cfg->dipswitch & 0xFC) | (value & 0x01));
}

INLINE static void _input_rd_reg_vs(BYTE *value, BYTE nport) {
	port_funct[nport].input_rd(value, nport, 0);

	// se avviene un DMA del DMC all'inizio
	// dell'istruzione di lettura del registro,
	// avverranno due letture.
	if (!info.r4016_dmc_double_read_disabled && (DMC.dma_cycle == 2)) {
		port_funct[nport].input_rd(value, nport, 0);
	}
}
