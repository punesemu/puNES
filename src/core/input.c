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

#include <string.h>
#include "clock.h"
#include "video/gfx.h"
#include "gui.h"
#include "info.h"
#include "conf.h"
#include "vs_system.h"
#include "nsf.h"
#include "input/nes_001.h"
#include "input/nsf_controller.h"
#include "input/nsf_mouse.h"
#include "input/famicom.h"
#include "input/four_score.h"
#include "input/vs.h"
#include "input/standard_controller.h"
#include "input/zapper.h"
#include "input/snes_mouse.h"
#include "input/arkanoid.h"
#include "input/oeka_kids_tablet.h"

#define SET_WR_REG(funct) input_wr_reg = funct
#define SET_RD_REG(id, funct) input_rd_reg[id] = funct

#define SET_WR(id, funct) port_funct[id].input_wr = funct
#define SET_RD(id, funct) port_funct[id].input_rd = funct

#define SET_DECODE_EVENT(id, funct) port_funct[id].input_decode_event = funct
#define SET_ADD_EVENT(id, funct) port_funct[id].input_add_event = funct

_r4016 r4016;
_port port[PORT_MAX];
_port_funct port_funct[PORT_MAX];

BYTE (*input_wr_reg)(BYTE value);
BYTE (*input_rd_reg[2])(BYTE openbus, BYTE nport);

void input_init(BYTE set_cursor) {
	BYTE a;

	r4016.value = 0;

	info.zapper_is_present = FALSE;

	input_init_four_score();
	input_init_zapper();
	input_init_snes_mouse();
	input_init_arkanoid();
	input_init_oeka_kids_tablet();
	input_init_nsf_mouse();

	if (nsf.enabled == TRUE) {
		SET_WR_REG(NULL);
		SET_RD_REG(PORT1, NULL);
		SET_RD_REG(PORT2, NULL);
	} else if (vs_system.enabled == TRUE) {
		SET_WR_REG(input_wr_reg_vs);
		SET_RD_REG(PORT1, input_rd_reg_vs_r4016);
		SET_RD_REG(PORT2, input_rd_reg_vs_r4017);
	} else {
		switch (cfg->input.controller_mode) {
			default:
			case CTRL_MODE_NES:
				SET_WR_REG(input_wr_reg_nes_001);
				SET_RD_REG(PORT1, input_rd_reg_nes_001);
				SET_RD_REG(PORT2, input_rd_reg_nes_001);
				break;
			case CTRL_MODE_FAMICOM:
				SET_WR_REG(input_wr_reg_famicom);
				SET_RD_REG(PORT1, input_rd_reg_famicom_r4016);
				SET_RD_REG(PORT2, input_rd_reg_famicom_r4017);
				break;
			case CTRL_MODE_FOUR_SCORE:
				SET_WR_REG(input_wr_reg_four_score);
				SET_RD_REG(PORT1, input_rd_reg_four_score);
				SET_RD_REG(PORT2, input_rd_reg_four_score);
				break;
		}
	}

	for (a = PORT1; a < PORT_MAX; a++) {
		SET_WR(a, input_wr_disabled);
		SET_RD(a, input_rd_disabled);
		SET_DECODE_EVENT(a, NULL);
		SET_ADD_EVENT(a, NULL);

		// NSF
		if (nsf.enabled == TRUE) {
			switch (a) {
				case PORT1:
					SET_DECODE_EVENT(a, input_decode_event_nsf_controller);
					SET_ADD_EVENT(a, input_add_event_nsf_controller);
					break;
				case PORT2:
					SET_ADD_EVENT(a, input_add_event_nsf_mouse);
					break;
			}
		// VS SYSTEM
		} else if (vs_system.enabled == TRUE) {
			if (info.extra_from_db & VSZAPPER) {
				switch (a) {
					case PORT1:
						SET_WR(a, input_wr_standard_controller);
						SET_RD(a, input_rd_standard_controller);
						SET_DECODE_EVENT(a, input_decode_event_standard_controller);
						SET_ADD_EVENT(a, input_add_event_standard_controller);
						break;
					case PORT2:
						SET_WR(a, input_wr_standard_controller);
						SET_RD(a, input_rd_zapper_vs);
						info.zapper_is_present = TRUE;
						break;
					default:
						break;
				}
			} else {
				switch (a) {
					case PORT1:
					case PORT2:
						SET_WR(a, input_wr_standard_controller);
						SET_RD(a, input_rd_standard_controller);
						SET_DECODE_EVENT(a, input_decode_event_standard_controller);
						SET_ADD_EVENT(a, input_add_event_standard_controller);
						break;
					default:
						break;
				}
			}
		} else {
			// Famicom
			if (cfg->input.controller_mode == CTRL_MODE_FAMICOM) {
				switch (port[a].type) {
					case CTRL_STANDARD:
						SET_WR(a, input_wr_standard_controller);
						SET_RD(a, input_rd_standard_controller);
						SET_DECODE_EVENT(a, input_decode_event_standard_controller);
						SET_ADD_EVENT(a, input_add_event_standard_controller);
						break;
					case CTRL_SNES_MOUSE:
						SET_WR(a, input_wr_snes_mouse);
						SET_RD(a, input_rd_snes_mouse);
						break;
					default:
						break;
				}
			} else {
				// NES-001 e Four Score
				switch (port[a].type) {
					case CTRL_STANDARD:
						SET_WR(a, input_wr_standard_controller);
						SET_RD(a, input_rd_standard_controller);
						SET_DECODE_EVENT(a, input_decode_event_standard_controller);
						SET_ADD_EVENT(a, input_add_event_standard_controller);
						break;
					case CTRL_ZAPPER:
						SET_RD(a, input_rd_zapper);
						info.zapper_is_present = TRUE;
						break;
					case CTRL_ARKANOID_PADDLE:
						SET_WR(a, input_wr_arkanoid);
						SET_RD(a, input_rd_arkanoid);
						break;
					case CTRL_SNES_MOUSE:
						SET_WR(a, input_wr_snes_mouse);
						SET_RD(a, input_rd_snes_mouse);
						break;
					default:
						break;
				}
			}
		}

		port[a].index = 0;

		{
			BYTE b, state = RELEASED;

			if (((port[a].type_pad == CTRL_PAD_AUTO) && (machine.type != DENDY)) || (port[a].type_pad == CTRL_PAD_ORIGINAL)) {
				state = PRESSED;
			}

			for (b = 0; b < LENGTH(port[a].data); b++) {
				if (b < 8) {
					if (info.reset >= HARD) {
						port[a].data[b] = RELEASED;
					}
				} else {
					port[a].data[b] = state;
				}
			}
		}
	}

	// Famicom expansion port
	if (cfg->input.controller_mode == CTRL_MODE_FAMICOM) {
		for (a = PORT3; a < PORT_MAX; a++) {
			SET_WR(a, input_wr_disabled);
			SET_RD(a, input_rd_disabled);
			SET_DECODE_EVENT(a, NULL);
			SET_ADD_EVENT(a, NULL);
		}
		switch (cfg->input.expansion) {
			case CTRL_STANDARD:
				for (a = PORT3; a < PORT_MAX; a++) {
					if (port[a].type != CTRL_DISABLED) {
						SET_WR(a, input_wr_standard_controller);
						SET_RD(a, input_rd_standard_controller);
						SET_DECODE_EVENT(a, input_decode_event_standard_controller);
						SET_ADD_EVENT(a, input_add_event_standard_controller);
					}
				}
				break;
			case CTRL_ZAPPER:
				SET_RD(PORT4, input_rd_zapper);
				info.zapper_is_present = TRUE;
				break;
			case CTRL_ARKANOID_PADDLE:
				SET_WR(PORT3, input_wr_arkanoid);
				SET_RD(PORT3, input_rd_arkanoid);
				SET_RD(PORT4, input_rd_arkanoid);
				break;
			case CTRL_OEKA_KIDS_TABLET:
				SET_WR(PORT4, input_wr_oeka_kids_tablet);
				SET_RD(PORT4, input_rd_oeka_kids_tablet);
				break;
			default:
				break;
		}
	}

	if (set_cursor == TRUE) {
		gfx_cursor_set();
	}

	gui_overlay_update();
}

void input_wr_disabled(UNUSED(BYTE *value), UNUSED(BYTE nport)) {}
void input_rd_disabled(UNUSED(BYTE *value), UNUSED(BYTE nport),	UNUSED(BYTE shift)) {}

BYTE input_draw_target(void) {
	BYTE i;

	if (vs_system.enabled == TRUE) {
		if ((info.extra_from_db & VSZAPPER) && (cfg->input.hide_zapper_cursor == FALSE)) {
			return (TRUE);
		}
		return (FALSE);
	}

	if (cfg->input.controller_mode == CTRL_MODE_FAMICOM) {
		switch (cfg->input.expansion) {
			case CTRL_ZAPPER:
				if (cfg->input.hide_zapper_cursor == FALSE) {
					return (TRUE);
				}
				return (FALSE);
			case CTRL_OEKA_KIDS_TABLET:
				return (TRUE);
		}
	} else {
		for (i = PORT1; i < PORT_MAX; i++) {
			if ((port[i].type == CTRL_ZAPPER) && (cfg->input.hide_zapper_cursor == FALSE)) {
				return (TRUE);
			}
		}
	}

	return (FALSE);
}
