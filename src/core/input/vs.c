/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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
#include "input/nes_001.h"
#include "input/four_score.h"
#include "conf.h"
#include "vs_system.h"
#include "nes.h"

BYTE input_wr_reg_vs(BYTE nidx, BYTE value) {
	if (vs_system.special_mode.type < VS_DS_Normal) {
		vs_system.shared_mem = !(value & 0x02);
		return (input_wr_reg_nes_001(nidx, value));
	} else {
		if (nidx == 0) {
			vs_system.shared_mem = !(value & 0x02);
		}
		if (!(value & 0x02)) {
			nes[!nidx].c.irq.high |= EXT_IRQ;
		} else {
			nes[!nidx].c.irq.high &= ~EXT_IRQ;
		}
		return (input_wr_reg_four_score(nidx, value));
	}
}
BYTE input_rd_reg_vs_r4016(BYTE nidx, UNUSED(BYTE openbus), BYTE nport) {
	BYTE value = 0;

	// port $4016
	// 7  bit  0
	// ---- ----
	// PCCD DS0B
	// |||| ||||
	// |||| |||+- Buttons for right stick (A, B, 1, 3, Up, Down, Left, Right)
	// |||| ||+-- always 0 (from floating input on 74LS240)
	// |||| |+--- Service button (commonly inserts a credit)
	// |||+-+---- DIP switches "2" and "1", respectively
	// |++------- Coin inserted (read below)
	// +--------- 0: Game is running on the primary CPU (it controls which CPU has access to shared RAM)
	//            1: Game is running on the secondary CPU (it must prevent watchdog timer timeout)
	if (vs_system.special_mode.type < VS_DS_Normal) {
		value = input_rd_reg_nes_001(nidx, 0x00, nport);
		value |= (vs_system.coins.left || vs_system.coins.right ? 0x20 : 0x00);
	} else {
		value = input_rd_reg_four_score_vs(nidx, 0x00, nport);
		value |= (nidx == 1 ? vs_system.coins.right ? 0x40 : 0x00 : 0x00);
		value |= (nidx == 0 ? vs_system.coins.left ? 0x20 : 0x00 : 0x00);
	}
	return ((nidx << 7) |
		(((cfg->dipswitch >> (nidx << 3)) & 0x03) << 3) |
		(vs_system.coins.service ? 0x04 : 0x00) |
		(value & 0x61));
}
BYTE input_rd_reg_vs_r4017(BYTE nidx, UNUSED(BYTE openbus), BYTE nport) {
	BYTE value = 0;

	vs_system.watchdog.timer = 0;
	// port $4017
	// 7  bit  0
	// ---- ----
	// DDDD DD0B
	// |||| ||||
	// |||| |||+- Buttons for left stick (A, B, 2, 4, Up, Down, Left, Right)
	// |||| ||+-- always 0 (from floating input on 74LS240)
	// ++++-++--- More DIP switches ("8" down to "3")
	if (vs_system.special_mode.type < VS_DS_Normal) {
		value = input_rd_reg_nes_001(nidx, 0x00, nport);
	} else {
		value = input_rd_reg_four_score_vs(nidx, 0x00, nport);
	}
	return (((cfg->dipswitch >> (nidx << 3)) & 0xFC) | (value & 0x01));
}
