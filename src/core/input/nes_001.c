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

#include "input/nes_001.h"
#include "info.h"
#include "apu.h"

BYTE input_wr_reg_nes_001(BYTE value) {
	// in caso di strobe azzero l'indice
	port_funct[PORT1].input_wr(&value, PORT1);
	port_funct[PORT2].input_wr(&value, PORT2);

	// restituisco il nuovo valore del $4016
	return (value);
}
BYTE input_rd_reg_nes_001(BYTE openbus, BYTE nport) {
	BYTE value = 0;

	port_funct[nport].input_rd(&value, nport, 0);

	// se avviene un DMA del DMC all'inizio
	// dell'istruzione di lettura del registro,
	// avverranno due letture.
	if (!info.r4016_dmc_double_read_disabled && (DMC.dma_cycle == 2)) {
		port_funct[nport].input_rd(&value, nport, 0);
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
