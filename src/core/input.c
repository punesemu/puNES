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

#include <string.h>
#include "clock.h"
#include "video/gfx.h"
#include "gui.h"
#include "info.h"
#include "nes.h"
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
#include "input/family_basic_keyboard.h"
#include "input/subor_keyboard.h"

INLINE static void input_init_generic_keyboard(void);

#define SET_WR_REG(funct) input_wr_reg = funct
#define SET_RD_REG(id, funct) input_rd_reg[id] = funct

#define SET_WR(id, funct) port_funct[id].input_wr = funct
#define SET_RD(id, funct) port_funct[id].input_rd = funct

#define SET_DECODE_EVENT(id, funct) port_funct[id].input_decode_event = funct
#define SET_ADD_EVENT(id, funct) port_funct[id].input_add_event = funct

_port port[PORT_MAX];
_port_funct port_funct[PORT_MAX];
_nes_keyboard nes_keyboard;
_generic_keyboard generic_keyboard;
_mic mic;
_ctrl_input_permit_updown_leftright cipu[PORT_MAX];
_ctrl_input_permit_updown_leftright cipl[PORT_MAX];

BYTE (*input_wr_reg)(BYTE nidx, BYTE value);
BYTE (*input_rd_reg[2])(BYTE nidx, BYTE openbus, BYTE nport);

void input_init(BYTE set_cursor) {
	int a = 0;

	for (int nesidx = 0; nesidx < info.number_of_nes; nesidx++) {
		nes[nesidx].c.input.r4016 = 0;
	}

	info.zapper_is_present = FALSE;

	input_init_four_score();
	input_init_zapper();
	input_init_snes_mouse();
	input_init_arkanoid();
	input_init_oeka_kids_tablet();
	input_init_nsf_mouse();
	input_init_generic_keyboard();

	memset(&mic, 0x00, sizeof(mic));

	if (nsf.enabled) {
		SET_WR_REG(NULL);
		SET_RD_REG(PORT1, NULL);
		SET_RD_REG(PORT2, NULL);
	} else if (vs_system.enabled) {
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

		memset(&cipu[a], 0x00, sizeof(_ctrl_input_permit_updown_leftright));
		memset(&cipl[a], 0x00, sizeof(_ctrl_input_permit_updown_leftright));

		// NSF
		if (nsf.enabled) {
			switch (a) {
				case PORT1:
					SET_DECODE_EVENT(a, input_decode_event_nsf_controller);
					SET_ADD_EVENT(a, input_add_event_nsf_controller);
					break;
				case PORT2:
					SET_ADD_EVENT(a, input_add_event_nsf_mouse);
					break;
				default:
					break;
			}
		// VS SYSTEM
		} else if (vs_system.enabled) {
			if (info.mapper.expansion == EXP_VS_ZAPPER) {
				switch (a) {
					case PORT1:
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
						SET_RD(a, input_rd_standard_controller_vs);
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

		for (int nesidx = 0; nesidx < NES_CHIPS_MAX; nesidx++) {
			nes[nesidx].c.input.pindex[a] = 0;
		}

		{
			BYTE state = RELEASED;

			if (((port[a].type_pad == CTRL_PAD_AUTO) && (machine.type != DENDY)) || (port[a].type_pad == CTRL_PAD_ORIGINAL)) {
				state = PRESSED;
			}

			for (unsigned int b = 0; b < LENGTH(port[a].data); b++) {
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
			case CTRL_FAMILY_BASIC_KEYBOARD:
				nes_keyboard.enabled = TRUE;
				SET_WR(PORT3, input_wr_family_basic_keyboard);
				SET_RD(PORT3, input_rd_family_basic_keyboard);
				SET_RD(PORT4, input_rd_family_basic_keyboard);
				SET_ADD_EVENT(PORT3, input_add_event_family_basic_keyboard);
				break;
			case CTRL_SUBOR_KEYBOARD:
				nes_keyboard.enabled = TRUE;
				SET_WR(PORT3, input_wr_subor_keyboard);
				SET_RD(PORT3, input_rd_subor_keyboard);
				SET_RD(PORT4, input_rd_subor_keyboard);
				SET_ADD_EVENT(PORT3, input_add_event_subor_keyboard);
				break;
			default:
				break;
		}
	}

	if (set_cursor) {
		gfx_cursor_set();
	}
	gui_nes_keyboard();
	gui_overlay_update();
}

void input_wr_disabled(UNUSED(BYTE nidx), UNUSED(const BYTE *value), UNUSED(BYTE nport)) {}
void input_rd_disabled(UNUSED(BYTE nidx), UNUSED(BYTE *value), UNUSED(BYTE nport),	UNUSED(BYTE shift)) {}

BYTE input_permit_updown_leftright(BYTE index, BYTE nport) {
	_ctrl_input_permit_updown_leftright *cip = NULL;
	static BYTE delay = 5;
	BYTE value[2] = { 0 };
	BYTE axe[2] = { 0 };

	if (cfg->input.permit_updown_leftright) {
		return (port[nport].data[index]);
	}
	if ((index == LEFT) || (index == RIGHT)) {
		axe[0] = LEFT;
		axe[1] = RIGHT;
		cip = &cipl[nport];
	} else if ((index == UP) || (index == DOWN)) {
		axe[0] = UP;
		axe[1] = DOWN;
		cip = &cipu[nport];
	} else {
		return (port[nport].data[index]);
	}
	value[0] = port[nport].data[axe[0]];
	value[1] = port[nport].data[axe[1]];
	if (!cip->delay) {
		if (cip->axe != FALSE) {
			if (((cip->axe == axe[0]) && !port[nport].data[axe[0]]) ||
				((cip->axe == axe[1]) && !port[nport].data[axe[1]])) {
				cip->delay = delay;
			}
		} else {
			if (!(port[nport].data[axe[0]] | port[nport].data[axe[1]])) {
				cip->axe = FALSE;
			} else if (port[nport].data[axe[0]] & port[nport].data[axe[1]]) {
				if (cip->axe == FALSE) {
					cip->axe = axe[0];
				}
			} else if (port[nport].data[axe[0]]) {
				cip->axe = axe[0];
			} else if (port[nport].data[axe[1]]) {
				cip->axe = axe[1];
			} else {
				cip->axe = FALSE;
			}
		}
	} else {
		cip->delay--;
		if (!cip->delay) {
			cip->axe = FALSE;
		}
	}
	if (cip->axe == axe[0]) {
		value[1] = RELEASED;
	} else if (cip->axe == axe[1]) {
		value[0] = RELEASED;
	}
	return (index == axe[0] ? value[0] : value[1]);
}
BYTE input_draw_target(void) {
	int i = 0;

	if (vs_system.enabled) {
		if ((info.mapper.expansion == EXP_VS_ZAPPER) && !cfg->input.hide_zapper_cursor) {
			return (TRUE);
		}
		return (FALSE);
	}

	if (cfg->input.controller_mode == CTRL_MODE_FAMICOM) {
		switch (cfg->input.expansion) {
			case CTRL_ZAPPER:
				if (!cfg->input.hide_zapper_cursor) {
					return (TRUE);
				}
				return (FALSE);
			case CTRL_OEKA_KIDS_TABLET:
				return (TRUE);
		}
	} else {
		for (i = PORT1; i < PORT_MAX; i++) {
			if ((port[i].type == CTRL_ZAPPER) && !cfg->input.hide_zapper_cursor) {
				return (TRUE);
			}
		}
	}

	return (FALSE);
}

INLINE static void input_init_generic_keyboard(void) {
	nes_keyboard.enabled = FALSE;
	memset(&generic_keyboard, 0x00, sizeof(generic_keyboard));
}
