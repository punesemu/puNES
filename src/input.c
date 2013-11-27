/*
 * input.c
 *
 *  Created on: 01/nov/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include "input.h"
#include "clock.h"
#include "ppu.h"
#include "palette.h"
#include "gui.h"
#include "gfx.h"
#include "apu.h"
#include "overscan.h"
#include "fps.h"
#include "tas.h"
#include "cfg_file.h"
#include "param.h"
#if defined SDL
#include "opengl.h"
#endif

static void INLINE input_turbo_buttons_control_standard(_port *port);

void input_init(void) {
	BYTE a;

	r4016.value = 0;

	for (a = PORT1; a < PORT_MAX; a++) {
		switch (port[a].type) {
			case CTRL_DISABLED:
			default:
				input_decode_event[a] = NULL;
				input_add_event[a] = NULL;
				SET_RD_REG(a, input_rd_reg_disabled);
				break;
			case CTRL_STANDARD:
				SET_DECODE_EVENT(a, input_decode_event_standard);
				SET_ADD_EVENT(a, input_add_event_standard);
				SET_RD_REG(a, input_rd_reg_standard);
				break;
			case CTRL_ZAPPER:
				input_decode_event[a] = NULL;
				input_add_event[a] = NULL;
				SET_RD_REG(a, input_rd_reg_zapper);
				break;
		}

		port[a].index = port[a].zapper = 0;

		{
			BYTE b;

			for (b = 0; b < 24; b++) {
				if (b < 8) {
					port[a].data[b] = RELEASED;
				} else {
					port[a].data[b] = PRESSED;
				}
			}
		}
	}
}
void input_check_conflicts(_config_input *settings, _array_pointers_port *array) {
	BYTE a;

	if (settings->check_input_conflicts == FALSE) {
		return;
	}

	/* Standard Controller */
	for (a = PORT1; a < PORT_MAX; a++) {
		BYTE b;
		_port *this = array->port[a];

		if (this->type != CTRL_STANDARD) {
			continue;
		}

		for (b = a; b < PORT_MAX; b++) {
			BYTE type;
			_port *other = array->port[b];

			if (other->type != CTRL_STANDARD) {
				continue;
			}

			for (type = KEYBOARD; type <= JOYSTICK ; type++) {
				BYTE this_button;

				if (type == JOYSTICK) {
					if ((this != other) && (this->joy_id != name_to_jsn("NULL"))
					        && (this->joy_id == other->joy_id)) {
						other->joy_id = name_to_jsn("NULL");
					}
					continue;
				}

				for (this_button = BUT_A; this_button < MAX_STD_PAD_BUTTONS; this_button++) {
					BYTE other_button;

					for (other_button = this_button; other_button < MAX_STD_PAD_BUTTONS;
					        other_button++) {
						if ((this == other) && (this_button == other_button)) {
							continue;
						}

						if (this->input[type][this_button] == other->input[type][other_button]) {
							other->input[type][other_button] = 0;
						}
					}
				}
			}
		}
	}
}

BYTE input_rd_reg_disabled(BYTE openbus, WORD **screen_index, _port *port) {
	return (openbus);
	/*return (0x80);*/
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
		/* non possono essere premuti contemporaneamente */
		if ((cfg->input.permit_updown_leftright == FALSE) && (mode == PRESSED)) {
			port->data[DOWN] = RELEASED;
		}
		return (EXIT_OK);
	} else if (event == port->input[type][DOWN]) {
		port->data[DOWN] = mode;
		/* non possono essere premuti contemporaneamente */
		if ((cfg->input.permit_updown_leftright == FALSE) && (mode == PRESSED)) {
			port->data[UP] = RELEASED;
		}
		return (EXIT_OK);
	} else if (event == port->input[type][LEFT]) {
		port->data[LEFT] = mode;
		/* non possono essere premuti contemporaneamente */
		if ((cfg->input.permit_updown_leftright == FALSE) && (mode == PRESSED)) {
			port->data[RIGHT] = RELEASED;
		}
		return (EXIT_OK);
	} else if (event == port->input[type][RIGHT]) {
		port->data[RIGHT] = mode;
		/* non possono essere premuti contemporaneamente */
		if ((cfg->input.permit_updown_leftright == FALSE) && (mode == PRESSED)) {
			port->data[LEFT] = RELEASED;
		}
		return (EXIT_OK);
	} else if (event == port->input[type][TRB_A]) {
		if (mode == PRESSED) {
			/*if (!(port->turbo[TURBOA].active = !port->turbo[TURBOA].active)) {
				port->data[BUT_A] = RELEASED;
				port->turbo[TURBOA].counter = 0;
			}*/
			port->turbo[TURBOA].active = TRUE;
		} else {
			port->turbo[TURBOA].active = FALSE;
			port->data[BUT_A] = RELEASED;
			port->turbo[TURBOA].counter = 0;
		}
		return (EXIT_OK);
	} else if (event == port->input[type][TRB_B]) {
		if (mode == PRESSED) {
			/*if (!(port->turbo[TURBOB].active = !port->turbo[TURBOB].active)) {
				port->data[BUT_B] = RELEASED;
				port->turbo[TURBOB].counter = 0;
			}*/
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
void input_add_event_standard(BYTE index) {
	js_control(&js[index], &port[index]);
	input_turbo_buttons_control_standard(&port[index]);
}
BYTE input_rd_reg_standard(BYTE openbus, WORD **screen_index, _port *port) {
	BYTE value;

	/*
	 * Se $4016 e' a 1 leggo solo lo stato del primo pulsante
	 * del controller. Quando verra' scritto 0 nel $4016
	 * riprendero' a leggere lo stato di tutti i pulsanti.
	 */
	value = port->data[port->index];
	if (!r4016.value) {
		port->index++;
	}
	/*
 	 * se avviene un DMA del DMC all'inizio
 	 * dell'istruzione di lettura del registro,
 	 * avverrano due letture.
 	 */
	if (!info.r4016_dmc_double_read_disabled && (DMC.dma_cycle == 2)) {
		value = port->data[port->index];
		if (!r4016.value) {
			port->index++;
		}
	}

	return(value);
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

BYTE input_rd_reg_zapper(BYTE openbus, WORD **screen_index, _port *port) {
	int x_zapper = -1, y_zapper = -1;
	int x_rect, y_rect;
	int gx = gui.x, gy = gui.y;
	int count = 0;

	port->zapper &= ~0x10;

	if (gui.left_button) {
		port->zapper |= 0x10;
	}

	if (!gui.right_button) {
#if defined SDL
		if (gfx.opengl) {
			int l = (int) opengl.quadcoords.l;
			int b = (int) opengl.quadcoords.b;

			gx -= l;
			gy -= b;
		}
#endif
		x_zapper = gx / gfx.w_pr;
		y_zapper = gy / gfx.h_pr;

		if (overscan.enabled) {
			x_zapper += overscan.left;
			y_zapper += overscan.up;
#if defined SDL
			/*
			 * il filtro NTSC necessita di un'aggiustatina sia con
			 * l'overscan abilitato che senza.
			 */
			if (cfg->filter == NTSC_FILTER) {
				x_zapper += 1;
			}
#endif
		} else {
#if defined SDL
			if (cfg->filter == NTSC_FILTER) {
				x_zapper -= 1;
			}
#endif
		}
	}

	//fprintf(stderr, "x : %d (%d)    %d (%d)   \r", x_zapper, gui.x, y_zapper, gui.y);

	if ((x_zapper <= 0) || (x_zapper >= SCR_ROWS) || (y_zapper <= 0) || (y_zapper >= SCR_LINES)) {
		return (port->zapper |= 0x08);
	}

	if (!r2002.vblank && r2001.visible && (ppu.frame_y > machine.vint_lines)
	        && (ppu.screen_y < SCR_LINES)) {
		for (y_rect = (y_zapper - 8); y_rect < (y_zapper + 8); y_rect++) {
			if (y_rect < 0) {
				continue;
			}
			if (y_rect <= (ppu.screen_y - 18)) {
				continue;
			}
			if (y_rect >= ppu.screen_y) {
				break;
			}
			for (x_rect = (x_zapper - 8); x_rect < (x_zapper + 8); x_rect++) {
				if (x_rect < 0) {
					continue;
				}
				if (x_rect > 255) {
					break;
				}
				{
					int brightness;
					_color_RGB color = palette_RGB[screen_index[y_rect][x_rect]];

					brightness = (color.r * 0.299) + (color.g * 0.587) + (color.b * 0.114);
					if (brightness > 0x80) {
						count++;
					}
				}
			}
		}
	}

	port->zapper &= ~0x08;

	if (count < 0x40) {
		port->zapper |= 0x08;
	}

	return (port->zapper);
}
BYTE input_zapper_is_connected(_port *array) {
	BYTE i;

	for (i = PORT1; i < PORT_MAX; i++) {
		if (array[i].type == CTRL_ZAPPER) {
			return (TRUE);
		}
	}

	return (FALSE);
}


