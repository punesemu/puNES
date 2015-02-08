/*
 * input.c
 *
 *  Created on: 01/nov/2011
 *      Author: fhorse
 */

#include "input.h"
#include "clock.h"
#include "info.h"
#include "ppu.h"
#include "palette.h"
#include "gui.h"
#include "gfx.h"
#include "apu.h"
#include "overscan.h"
#include "fps.h"
#include "tas.h"
#include "conf.h"
#if defined (SDL)
#include "opengl.h"
#endif

static void INLINE input_turbo_buttons_control_standard(_port *port);

void input_init(void) {
	BYTE a;

	r4016.value = 0;

	for (a = PORT1; a <= PORT2; a++) {
		memset(&four_score[a], 0x00, sizeof(_four_score));
	}

	switch (cfg->input.controller_mode) {
		case CTRL_MODE_NES:
		case CTRL_MODE_FAMICOM:
			SET_WR_REG(input_wr_reg_standard);
			break;
		case CTRL_MODE_FOUR_SCORE:
			SET_WR_REG(input_wr_reg_four_score);
			break;
	}

	for (a = PORT1; a < PORT_MAX; a++) {
		switch (port[a].type) {
			case CTRL_DISABLED:
			default:
				input_decode_event[a] = NULL;
				input_add_event[a] = NULL;
				if (a <= PORT2) {
					SET_RD_REG(a, input_rd_reg_disabled);
				}
				break;
			case CTRL_STANDARD:
				SET_DECODE_EVENT(a, input_decode_event_standard);
				SET_ADD_EVENT(a, input_add_event_standard);
				if (a <= PORT2) {
					switch (cfg->input.controller_mode) {
						case CTRL_MODE_NES:
							SET_RD_REG(a, input_rd_reg_standard);
							break;
						case CTRL_MODE_FAMICOM:
							SET_RD_REG(a, input_rd_reg_famicon_expansion);
							break;
						case CTRL_MODE_FOUR_SCORE:
							SET_RD_REG(a, input_rd_reg_four_score);
							break;
					}
				}
				break;
			case CTRL_ZAPPER:
				input_decode_event[a] = NULL;
				input_add_event[a] = NULL;
				if (a <= PORT2) {
					SET_RD_REG(a, input_rd_reg_zapper);
				}
				break;
		}

		port[a].index = port[a].zapper = 0;

		{
			BYTE b, state = RELEASED;

			if (((port[a].type_pad == CTRL_PAD_AUTO) && (machine.type != DENDY))
					|| (port[a].type_pad == CTRL_PAD_ORIGINAL)) {
				state = PRESSED;
			}

			for (b = 0; b < 24; b++) {
				if (b < 8) {
					port[a].data[b] = RELEASED;
				} else {
					port[a].data[b] = state;
				}
			}
		}
	}
}

BYTE input_rd_reg_disabled(BYTE openbus, WORD **screen_index, BYTE nport) {
	return (openbus);
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
BYTE input_wr_reg_standard(BYTE value) {
	/* in caso di strobe azzero l'indice */
	if (r4016.value && !(value & 0x01)) {
		BYTE i;

		for (i = PORT1; i < PORT_MAX; i++) {
			port[i].index = 0;
		}
	}

	/* restituisco il nuovo valore del $4016 */
	return(value & 0x01);
}
BYTE input_rd_reg_standard(BYTE openbus, WORD **screen_index, BYTE nport) {
	BYTE value;

#define _input_rd()\
	value = port[nport].data[port[nport].index];\
	if (!r4016.value) {\
		port[nport].index++;\
	}
	/*
	 * Se $4016 e' a 1 leggo solo lo stato del primo pulsante
	 * del controller. Quando verra' scritto 0 nel $4016
	 * riprendero' a leggere lo stato di tutti i pulsanti.
	 */
	_input_rd()
	/*
 	 * se avviene un DMA del DMC all'inizio
 	 * dell'istruzione di lettura del registro,
 	 * avverranno due letture.
 	 */
	if (!info.r4016_dmc_double_read_disabled && (DMC.dma_cycle == 2)) {
		_input_rd()
	}
#undef _input_rd

	/*
	 * NES only, both $4016 and $4017:
	 * 7  bit  0
	 * ---- ----
	 * OOOx xxxD
	 * |||| ||||
	 * |||| |||+- Serial controller data
	 * |||+-+++-- Always 0
	 * +++------- Open bus
	 */
	return((openbus & 0xE0) | value);
}

BYTE input_rd_reg_famicon_expansion(BYTE openbus, WORD **screen_index, BYTE nport) {
	BYTE value, nport2 = nport + 2;

#define _input_rd()\
	value = port[nport].data[port[nport].index];\
	value |= (port[nport2].data[port[nport2].index] << 1);\
	if (!r4016.value) {\
		port[nport].index++;\
		port[nport2].index++;\
	}
	/*
	 * Se $4016 e' a 1 leggo solo lo stato del primo pulsante
	 * del controller. Quando verra' scritto 0 nel $4016
	 * riprendero' a leggere lo stato di tutti i pulsanti.
	 */
	_input_rd()
	/*
 	 * se avviene un DMA del DMC all'inizio
 	 * dell'istruzione di lettura del registro,
 	 * avverrano due letture.
 	 */
	if (!info.r4016_dmc_double_read_disabled && (DMC.dma_cycle == 2)) {
		_input_rd()
	}
#undef _input_rd

	/*
	 * Famicom $4016:
	 * 7  bit  0
	 * ---- ----
	 * OOOx xMFD
	 * |||| ||||
	 * |||| |||+- Player 1 serial controller data
	 * |||| ||+-- If connected to expansion port, player 3 serial controller data (0 otherwise)
	 * |||| |+--- Microphone in controller 2 on traditional Famicom, 0 on AV Famicom
	 * |||+-+---- Open bus on traditional Famicom, all 0s on AV Famicom
	 * +++------- Open bus
	 *
	 * Famicom $4017:
	 * 7  bit  0
	 * ---- ----
	 * OOOx xxFD
	 * |||| ||||
	 * |||| |||+- Player 2 serial controller data
	 * |||| ||+-- If connected to expansion port, player 4 serial controller data (0 otherwise)
	 * |||+-+++-- Returns 0 unless something is plugged into the Famicom expansion port
	 * +++------- Open bus
	 */
	/* emulo un traditional Famicom */
	/* visto che per ora non emulo alcun microfono metto tutto a 0 */
	return((openbus & 0xF8) | value);
}

BYTE input_wr_reg_four_score(BYTE value) {
	value &= 0x01;

	if (!value) {
		four_score[PORT1].count = 0;
		four_score[PORT2].count = 0;
	}

	return(value);
}
BYTE input_rd_reg_four_score(BYTE openbus, WORD **screen_index, BYTE nport) {
	BYTE value = 0, nport2 = nport + 2;

#define _input_rd(np)\
	value = port[np].data[four_score[nport].count & 0x07];

	if (four_score[nport].count < 8) {
		_input_rd(nport)
	} else if (four_score[nport].count < 16) {
		_input_rd(nport2)
	} else if (four_score[nport].count < 20) {
		value = (four_score[nport].count - 18) ^ nport;
	}

	++four_score[nport].count;

	/*
	 * Se $4016 e' a 1 leggo solo lo stato del primo pulsante
	 * del controller. Quando verra' scritto 0 nel $4016
	 * riprendero' a leggere lo stato di tutti i pulsanti.
	 */
	//_input_rd(nport)
	/*
 	 * se avviene un DMA del DMC all'inizio
 	 * dell'istruzione di lettura del registro,
 	 * avverrano due letture.
 	 */
	//if (!info.r4016_dmc_double_read_disabled && (DMC.dma_cycle == 2)) {
	//	_input_rd(nport)
	//}
#undef _input_rd

	return(value);
}

BYTE input_rd_reg_zapper(BYTE openbus, WORD **screen_index, BYTE nport) {
	int x_zapper = -1, y_zapper = -1;
	int x_rect, y_rect;
	int gx = mouse.x, gy = mouse.y;
	int count = 0;

	port[nport].zapper &= ~0x10;

	if (mouse.left) {
		port[nport].zapper |= 0x10;
	}

	if (!mouse.right) {
#if defined (SDL)
		if (gfx.opengl) {
			int l = (int) opengl.quadcoords.l;
			int b = (int) opengl.quadcoords.b;

			gx -= l;
			gy -= b;
		}
#elif defined (D3D9)
		{
			int l = (int) gfx.quadcoords.l;
			int t = (int) gfx.quadcoords.t;

			gx -= l;
			gy -= t;
		}
#endif
		x_zapper = gx / gfx.w_pr;
		y_zapper = gy / gfx.h_pr;

		if (overscan.enabled) {
			x_zapper += overscan.borders->left;
			y_zapper += overscan.borders->up;
			/*
			 * il filtro NTSC necessita di un'aggiustatina sia con
			 * l'overscan abilitato che senza.
			 */
			if (cfg->filter == NTSC_FILTER) {
				x_zapper += 1;
			}
		} else {
			if (cfg->filter == NTSC_FILTER) {
				x_zapper -= 1;
			}
		}
	}

	//fprintf(stderr, "x : %d (%d)    %d (%d)   \r", x_zapper, gui.x, y_zapper, gui.y);

	if ((x_zapper <= 0) || (x_zapper >= SCR_ROWS) || (y_zapper <= 0) || (y_zapper >= SCR_LINES)) {
		return (port[nport].zapper |= 0x08);
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

	port[nport].zapper &= ~0x08;

	if (count < 0x40) {
		port[nport].zapper |= 0x08;
	}

	return (port[nport].zapper);
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


