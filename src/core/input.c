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

#include <string.h>
#include "input.h"
#include "clock.h"
#include "gfx.h"
#include "info.h"
#include "conf.h"
#include "vs_system.h"
#include "input/famicom.h"
#include "input/four_score.h"
#include "input/standard.h"
#include "input/zapper.h"
#include "input/snes_mouse.h"
#include "input/arkanoid.h"

#define SET_DECODE_EVENT(id, funct) input_decode_event[id] = funct
#define SET_ADD_EVENT(id, funct) input_add_event[id] = funct
#define SET_WR_REG(funct) input_wr_reg = funct
#define SET_RD_REG(id, funct) input_rd_reg[id] = funct

void input_init(BYTE set_cursor) {
	BYTE a;

	r4016.value = 0;

	input_init_four_score();
	input_init_zapper();
	input_init_snes_mouse();
	input_init_arkanoid();

	if (vs_system.enabled == TRUE) {
		SET_WR_REG(input_wr_reg_standard_vs);
	} else {
		switch (cfg->input.controller_mode) {
			case CTRL_MODE_NES:
				SET_WR_REG(input_wr_reg_standard);
				SET_RD_REG(PORT1, input_rd_reg_standard);
				SET_RD_REG(PORT2, input_rd_reg_standard);
				break;
			case CTRL_MODE_FAMICOM:
				SET_WR_REG(input_wr_reg_famicom);
				SET_RD_REG(PORT1, input_rd_reg_famicom);
				SET_RD_REG(PORT2, input_rd_reg_famicom);
				break;
			case CTRL_MODE_FOUR_SCORE:
				SET_WR_REG(input_wr_reg_four_score);
				SET_RD_REG(PORT1, input_rd_reg_four_score);
				SET_RD_REG(PORT2, input_rd_reg_four_score);
				break;
		}
	}

	for (a = PORT1; a < PORT_MAX; a++) {
		if (vs_system.enabled == TRUE) {
			if (info.extra_from_db & VSZAPPER) {
				if (a == PORT1) {
					SET_RD_REG(a, input_rd_reg_zapper_vs);
					input_decode_event[a] = NULL;
					input_add_event[a] = NULL;
				} else if (a == PORT2) {
					SET_RD_REG(a, input_rd_reg_standard_vs);
					SET_DECODE_EVENT(a, input_decode_event_standard);
					SET_ADD_EVENT(a, input_add_event_standard);
				}
			} else {
				if (a <= PORT2) {
					SET_RD_REG(a, input_rd_reg_standard_vs);
					SET_DECODE_EVENT(a, input_decode_event_standard);
					SET_ADD_EVENT(a, input_add_event_standard);
				}
			}
		} else {
			switch (port[a].type) {
				case CTRL_STANDARD:
					SET_DECODE_EVENT(a, input_decode_event_standard);
					SET_ADD_EVENT(a, input_add_event_standard);
					break;
				case CTRL_ARKANOID_PADDLE:
					input_decode_event[a] = NULL;
					input_add_event[a] = NULL;
					break;
				case CTRL_DISABLED:
				case CTRL_ZAPPER:
				case CTRL_SNES_MOUSE:
				default:
					input_decode_event[a] = NULL;
					input_add_event[a] = NULL;
					break;
			}
		}

		port[a].index = 0;

		{
			BYTE b, state = RELEASED;

			if (((port[a].type_pad == CTRL_PAD_AUTO) && (machine.type != DENDY))
					|| (port[a].type_pad == CTRL_PAD_ORIGINAL)) {
				state = PRESSED;
			}

			for (b = 0; b < LENGTH(port[a].data); b++) {
				if (b < 8) {
					port[a].data[b] = RELEASED;
				} else {
					port[a].data[b] = state;
				}
			}
		}
	}

	if (set_cursor == TRUE) {
		gfx_cursor_set();
	}
}

