/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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
#include "info.h"
#include "apu.h"

INLINE static void _input_rd_reg_famicom(BYTE *value, BYTE nport);

BYTE input_wr_reg_famicom(BYTE value) {
	// in caso di strobe azzero l'indice
	port_funct[PORT1].input_wr(&value, PORT1);
	port_funct[PORT2].input_wr(&value, PORT2);
	port_funct[PORT3].input_wr(&value, PORT3);
	port_funct[PORT4].input_wr(&value, PORT4);

	// restituisco il nuovo valore del $4016
	return (value);
}
BYTE input_rd_reg_famicom_r4016(BYTE openbus, BYTE nport) {
	BYTE value = 0;

	_input_rd_reg_famicom(&value, nport);

	// Famicom $4016:
	// 7  bit  0
	// ---- ----
	// oooo oMFD
	// |||| ||||
	// |||| |||+- Player 1 serial controller data
	// |||| ||+-- If connected to expansion port (and available), player 3 serial controller data (0 otherwise)
	// |||| |+--- Microphone in controller 2 on traditional Famicom, 0 on AV Famicom
	// ++++-+---- Open bus
	return ((openbus & 0xF8) | value);
}
BYTE input_rd_reg_famicom_r4017(BYTE openbus, BYTE nport) {
	BYTE value = 0;

	_input_rd_reg_famicom(&value, nport);

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

INLINE static void _input_rd_reg_famicom(BYTE *value, BYTE nport) {
	(*value) = 0;
	port_funct[nport].input_rd(value, nport, 0);
	port_funct[nport + 2].input_rd(value, nport + 2, 1);

	// se avviene un DMA del DMC all'inizio
	// dell'istruzione di lettura del registro,
	// avverranno due letture.
	if (!info.r4016_dmc_double_read_disabled && (DMC.dma_cycle == 2)) {
		(*value) = 0;
		port_funct[nport].input_rd(value, nport, 0);
		port_funct[nport + 2].input_rd(value, nport + 2, 1);
	}
}
