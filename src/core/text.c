/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#if defined (WITH_OPENGL)
#include <time.h>
#endif
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "text.h"
#include "video/gfx.h"
#include "font.h"
#include "tas.h"
#include "input.h"
#include "fds.h"
#include "fps.h"
#include "conf.h"
#include "save_slot.h"
#include "rewind.h"
#include "draw_on_screen.h"

enum txt_internal_misc {
	TXT_MARGIN = 2,
	TXT_SPACING = 3,
	TXT_FADE_SPEED = 4
};

#define port_control(prt, button, ch)\
	if (prt.data[button] == PRESSED) {\
		strcat(ele->text, "[yellow]");\
	} else {\
		strcat(ele->text, "[black]");\
	}\
	strcat(ele->text, ch)

INLINE static void fade_ele(_txt_element *ele, int velocity);
INLINE static void rendering(_txt_element *txt);

static char txt_tags[][10] = {
	"[normal]", "[red]",	"[yellow]", "[green]",
	"[cyan]",   "[brown]",	"[blue]",   "[gray]",
	"[black]",
	"[font8]",  "[font12]",
	"[left]",   "[right]",	"[up]",     "[down]",
	"[select]", "[start]",	"[a]",      "[b]",
	"[floppy]"
};

void text_init(void) {
	_txt_element *ele;
	uint8_t i;

	text_clear = gfx_text_clear;
	text_blit = gfx_text_blit;

	memset(&text, 0, sizeof(text));

	for (i = 0; i < TXT_MAX_LINES; i++) {
		text.info.lines[text.info.index][i] = &text.info.buffers[i];
		text.single.lines[i] = NULL;
	}

	// controllers
	for (i = PORT1; i < PORT_MAX; i++) {
		ele = &text.misc.controllers[i];
		ele->bck = TRUE;
		ele->bck_color = TXT_BLUE;
		ele->font = FONT_8X10;
		ele->factor = 1;
		ele->start_x = TXT_LEFT;
		ele->start_y = TXT_UP;
		ele->w = 8 * font_size[ele->font][0];
		ele->h = font_size[ele->font][1];
		ele->x = i * (ele->w + TXT_SPACING);
		ele->y = 0;
		ele->alpha[0] = 255;
		ele->alpha[1] = 170;
		ele->alpha[2] = 150;
	}

	// floppy
	{
		ele = &text.misc.floppy;
		ele->bck = FALSE;
		ele->font = FONT_12X10;
		ele->factor = 1;
		ele->start_x = TXT_RIGHT;
		ele->start_y = TXT_UP;
		ele->x = 0;
		ele->y = 0;
		ele->w = font_size[ele->font][0];
		ele->h = font_size[ele->font][1];
		ele->alpha[0] = 180;
		ele->alpha[2] = 0;
	}

	// counter frames
	{
		ele = &text.misc.counter_frames;
		ele->bck = TRUE;
		ele->bck_color = TXT_BLACK;
		ele->font = FONT_12X10;
		ele->factor = 1;
		ele->start_x = TXT_LEFT;
		ele->start_y = TXT_DOWN;
		ele->x = 0;
		ele->y = 0;
		ele->w = 0;
		ele->h = font_size[ele->font][1];
		ele->alpha[0] = 255;
		ele->alpha[1] = 170;
		ele->alpha[2] = 170;
	}

	// fps
	{
		ele = &text.misc.fps;
		ele->bck = TRUE;
		ele->bck_color = TXT_BLUE;
		ele->font = FONT_12X10;
		ele->factor = 1;
		ele->start_x = TXT_RIGHT;
		ele->start_y = TXT_DOWN;
		ele->x = 0;
		ele->y = 0;
		ele->w = 2 * font_size[ele->font][0];
		ele->h = font_size[ele->font][1];
		ele->alpha[0] = 220;
		ele->alpha[1] = 220;
		ele->alpha[2] = 220;
	}

	// rewind
	{
		ele = &text.misc.rewind;
		ele->bck = TRUE;
		ele->bck_color = TXT_BLACK;
		ele->font = FONT_12X10;
		ele->factor = 1;
		ele->start_x = TXT_RIGHT;
		ele->start_y = TXT_DOWN;
		ele->x = text.misc.fps.w + TXT_SPACING;
		ele->y = 0;
		ele->w = 0;
		ele->h = font_size[ele->font][1];
		ele->alpha[0] = 220;
		ele->alpha[1] = 220;
		ele->alpha[2] = 220;
	}

	// save slot
	{
		ele = &text.save_slot.slot;
		ele->bck = TRUE;
		ele->bck_color = TXT_BLUE;
		ele->font = FONT_12X10;
		ele->factor = 1;
		ele->start_x = TXT_CENTER;
		ele->start_y = TXT_UP;
		ele->x = 0;
		ele->y = 1 * font_size[ele->font][1];
		ele->w = SAVE_SLOTS * font_size[ele->font][0];
		ele->h = font_size[ele->font][1];
		ele->alpha[0] = 255;
		ele->alpha[1] = 170;
		ele->alpha[2] = 150;
	}
}
void text_add_line(int type, int factor, int font, int alpha, int start_x, int start_y, int x, int y, const char *fmt, ...) {
	uint8_t i, shift_line = !text.info.index;
	_txt_element *ele = NULL;
	va_list ap;

	if (type == TXT_INFO) {
		text.info.count = 1;

		for (i = 0; i < TXT_MAX_LINES; i++) {
			if (text.info.lines[text.info.index][i]->enabled) {
				text_clear(text.info.lines[text.info.index][i]);
			}
			if (i == (TXT_MAX_LINES - 1)) {
				text.info.lines[shift_line][0] = text.info.lines[text.info.index][i];
			} else {
				text.info.lines[shift_line][i + 1] = text.info.lines[text.info.index][i];
				if (text.info.lines[text.info.index][i]->enabled) {
					text.info.count++;
				}
			}
		}

		text.info.index = shift_line;

		ele = text.info.lines[text.info.index][0];
		ele->enabled = TRUE;
		ele->bck = FALSE;
		ele->font = font;
		ele->factor = factor;
		ele->w = 0;
		ele->h = 0;
		if (!text.info.lines[text.info.index][1]->start) {
			ele->start = time(NULL);
		} else {
			ele->start = text.info.lines[text.info.index][1]->start + 1;
		}
		ele->alpha_start_fade = alpha;
		ele->alpha[0] = alpha;
		ele->alpha[1] = 170;
		ele->alpha[2] = 40;
	}

	if (type == TXT_SINGLE) {
		for (i = 0; i < TXT_MAX_LINES; i++) {
			if (text.single.lines[i] == NULL) {
				text.single.lines[i] = (_txt_element *) malloc(sizeof(_txt_element));
				text.single.count++;

				ele = text.single.lines[i];

				memset(ele, 0, sizeof(_txt_element));

				ele->enabled = TRUE;
				ele->bck = FALSE;
				ele->font = font;
				ele->factor = factor;
				ele->start_x = start_x;
				ele->start_y = start_y;
				ele->x = x;
				ele->y = y;
				ele->start = time(NULL);
				ele->alpha_start_fade = alpha;
				ele->alpha[0] = alpha;
				ele->alpha[1] = 170;
				ele->alpha[2] = 40;
				break;
			}
		}
	}

	va_start(ap, fmt);
	vsnprintf(ele->text, TXT_MAX_MSG, fmt, ap);
	va_end(ap);

	int ch_font = ele->font;

	for (i = 0; i < strlen(ele->text); i++) {
		if (ele->text[i] == '[') {
			unsigned int tag, found = FALSE;

			for (tag = 0; tag < LENGTH(txt_tags); tag++) {
				int len = strlen(txt_tags[tag]);

				if (strncmp(ele->text + i, txt_tags[tag], len) == 0) {
					if (tag <= TXT_NORMAL) {
						ch_font = ele->font;
					} else if ((tag >= TXT_FONT_8) && (tag <= TXT_FONT_12)) {
						ch_font = tag - TXT_FONT_8;
					}
					i += (len - 1);
					found = TRUE;
					break;
				}
			}
			if (found) {
				continue;
			}
		}
		ele->w += ele->factor * font_size[ch_font][0];

		if (ele->h < ele->factor * font_size[ch_font][1]) {
			ele->h = ele->factor * font_size[ch_font][1];
		}

		ele->length++;
	}

	if (ele->surface) {
		gfx_text_release_surface(ele);
	}
}
void text_save_slot(BYTE operation) {
	if (cfg->fullscreen == FULLSCR) {
		text.save_slot.operation = operation;
		text.save_slot.slot.start = time(NULL);
		text.save_slot.slot.enabled = TRUE;
		text.save_slot.slot.alpha_start_fade = 255;
		text.save_slot.slot.alpha[0] = text.save_slot.slot.alpha_start_fade;
		text.save_slot.slot.alpha[1] = 170;
		text.save_slot.slot.alpha[2] = 150;
	}
}
void text_rendering(BYTE render) {
	_txt_element *ele;

	text.on_screen = FALSE;

	if (text.info.count) {
		int pos_x = TXT_MARGIN, pos_y = text.h - TXT_MARGIN - (text.misc.counter_frames.h);
		uint8_t i;

		for (i = 0; i < TXT_MAX_LINES; i++) {
			ele = text.info.lines[text.info.index][i];

			if (ele->enabled == TRUE) {
				if (!ele->surface) {
					gfx_text_create_surface(ele);
				}

				fade_ele(ele, 3);
				pos_y -= font_size[ele->font][1] * ele->factor;
				ele->start_x = 0;
				ele->start_y = 0;
				ele->x = pos_x;
				ele->y = pos_y;

				if ((cfg->scale != X1) && render) {
					rendering(ele);
				}

				if (!ele->enabled) {
					text_clear(ele);
					gfx_text_release_surface(ele);
					text.single.count--;
				}
			}
		}
	}

	if (text.single.count) {
		uint8_t i;

		for (i = 0; i < TXT_MAX_LINES; i++) {
			ele = text.single.lines[i];

			if (ele != NULL) {
				if (ele->enabled == TRUE) {
					if (!ele->surface) {
						gfx_text_create_surface(ele);
					}

					fade_ele(ele, 3);
					if ((cfg->scale != X1) && render) {
						rendering(ele);
					}
				} else {
					if (ele->surface) {
						text_clear(ele);
						gfx_text_release_surface(ele);
					}

					free(text.single.lines[i]);
					text.single.lines[i] = NULL;

					text.single.count--;
				}
			}
		}
	}

	// floppy
	if (fds.info.enabled) {
		ele = &text.misc.floppy;

		if ((fds.info.last_operation | fds.drive.disk_ejected)) {
			ele->enabled = TRUE;
			ele->alpha[0] = 180;
			ele->alpha[2] = 0;

			ele->text[0] = 0;

			if (fds.drive.disk_ejected) {
				ele->bck = TRUE;
				ele->bck_color = TXT_RED;
				ele->alpha[0] = 220;
				ele->alpha[2] = 220;
				strcpy(ele->text, "[normal]");
			} else {
				if (fds.info.last_operation == FDS_OP_READ) {
					strcpy(ele->text, "[green]");
				} else {
					strcpy(ele->text, "[red]");
				}
				if (rwnd.action != RWND_ACT_PAUSE) {
					fds.info.last_operation = FDS_OP_NONE;
				}
			}
			strcat(ele->text, "[floppy]");

			if (!ele->surface) {
				gfx_text_create_surface(ele);
			}

			if (render) {
				rendering(ele);
			}
		} else if (ele->enabled == TRUE) {
			ele->enabled = FALSE;

			if (ele->surface) {
				text_clear(ele);
			}
		}
	}

	// controllers
	if (tas.type | cfg->input_display) {
		uint8_t i;

		for (i = PORT1; i < PORT_MAX; i++) {
			ele = &text.misc.controllers[i];

			if (port[i].type != CTRL_STANDARD) {
				if (ele->surface) {
					text_clear(ele);
					gfx_text_release_surface(ele);
				}
				continue;
			}

			ele->text[0] = 0;
			port_control(port[i], UP, "[up]");
			port_control(port[i], DOWN, "[down]");
			port_control(port[i], LEFT, "[left]");
			port_control(port[i], RIGHT, "[right]");
			port_control(port[i], SELECT, "[select]");
			port_control(port[i], START, "[start]");
			port_control(port[i], BUT_A, "[a]");
			port_control(port[i], BUT_B, "[b]");

			if (!ele->surface) {
				gfx_text_create_surface(ele);
			}

			if (render) {
				rendering(ele);
			}
		}
	} else {
		uint8_t i;

		for (i = PORT1; i < PORT_MAX; i++) {
			ele = &text.misc.controllers[i];

			if (ele->surface) {
				text_clear(ele);
				gfx_text_release_surface(ele);
			}
		}
	}

	// counter_frames
	if ((tas.type != NOTAS) || (rwnd.action != RWND_ACT_PLAY)) {
		int w;

		ele = &text.misc.counter_frames;
		ele->enabled = TRUE;

		if (tas.type != NOTAS) {
			if (tas.lag_actual_frame) {
				sprintf(ele->text, "[yellow]%d[normal]/[normal]%d[normal] [red]%d[normal]", tas.frame, tas.total - 1,
					tas.total_lag_frames);
			} else {
				sprintf(ele->text, "[yellow]%d[normal]/[normal]%d[normal] [green]%d[normal]", tas.frame, tas.total - 1,
					tas.total_lag_frames);
			}
		} else {
			sprintf(ele->text, "[yellow]%d[normal]/[normal]%d[normal] (-%s)",
				rewind_snap_cursor(), rewind_count_snaps(), rewind_text_time_backward());
		}

		w = dos_strlen(ele->text) * font_size[ele->font][0];

		if ((w != ele->w) && ele->surface) {
			text_clear(ele);
			gfx_text_release_surface(ele);
		}

		ele->w = w;
		ele->h = font_size[ele->font][1];

		if (!ele->surface) {
			gfx_text_create_surface(ele);
		}

		if (render) {
			rendering(ele);
		}
	} else {
		ele = &text.misc.counter_frames;

		if (ele->enabled == TRUE) {
			ele->enabled = FALSE;

			if (ele->surface) {
				text_clear(ele);
				gfx_text_release_surface(ele);
			}
		}
	}

	// fps
	{
		ele = &text.misc.fps;

		if (cfg->show_fps) {
			int w;

			ele->enabled = TRUE;

			sprintf(ele->text, "%2d", (int)fps.gfx);

			w = dos_strlen(ele->text) * font_size[ele->font][0];

			if ((w != ele->w) && ele->surface) {
				text_clear(ele);
				gfx_text_release_surface(ele);
			}

			ele->w = w;
			ele->h = font_size[ele->font][1];

			if (!ele->surface) {
				gfx_text_create_surface(ele);
			}

			if (render) {
				rendering(ele);
			}
		} else if (ele->enabled == TRUE) {
			ele->enabled = FALSE;

			if (ele->surface) {
				text_clear(ele);
			}
		}
	}

	// rewind
	if (rwnd.action != RWND_ACT_PLAY) {
		BYTE action = rwnd.action;
		int w;

		ele = &text.misc.rewind;
		ele->enabled = TRUE;

		if (cfg->scale > X2) {
			if (ele->font != FONT_12X10) {
				ele->font = FONT_12X10;
			}
		} else {
			if (ele->font != FONT_8X10) {
				ele->font = FONT_8X10;
			}
		}

		if (rwnd.action == RWND_ACT_PAUSE) {
			action = rwnd.action_before_pause;
		}

		switch (action) {
			case RWND_ACT_PAUSE:
			case RWND_ACT_PLAY:
				sprintf(ele->text, "[yellow]stop[normal]");
				break;
			case RWND_ACT_STEP_BACKWARD:
				sprintf(ele->text, "[yellow] -1 [normal]");
				break;
			case RWND_ACT_FAST_BACKWARD:
				if (rwnd.factor.backward < 10) {
					sprintf(ele->text, "[yellow] -%dX[normal]", rwnd.factor.backward);
				} else {
					sprintf(ele->text, "[yellow]-%dX[normal]", rwnd.factor.backward);
				}
				break;
			case RWND_ACT_STEP_FORWARD:
				sprintf(ele->text, "[yellow] +1 [normal]");
				break;
			case RWND_ACT_FAST_FORWARD:
				if (rwnd.factor.backward < 10) {
					sprintf(ele->text, "[yellow] +%dX[normal]", rwnd.factor.forward);
				} else {
					sprintf(ele->text, "[yellow]+%dX[normal]", rwnd.factor.forward);
				}
				break;
		}

		w = dos_strlen(ele->text) * font_size[ele->font][0];

		if ((w != ele->w) && ele->surface) {
			text_clear(ele);
			gfx_text_release_surface(ele);
		}

		ele->x = text.misc.fps.w + TXT_SPACING;
		ele->w = w;
		ele->h = font_size[ele->font][1];

		if (!ele->surface) {
			gfx_text_create_surface(ele);
		}

		if (render) {
			rendering(ele);
		}
	} else {
		ele = &text.misc.rewind;

		if (ele->enabled == TRUE) {
			ele->enabled = FALSE;

			if (ele->surface) {
				text_clear(ele);
				gfx_text_release_surface(ele);
			}
		}
	}

	// save slot
	{
		ele = &text.save_slot.slot;
		char number[5];
		unsigned int i;

		if (ele->enabled == TRUE) {
			ele->text[0] = 0;

			fade_ele(ele, 2);

			for (i = 0; i < SAVE_SLOTS; i++) {
				if (save_slot.slot == i) {
					if (text.save_slot.operation == SAVE_SLOT_SAVE) {
						strcat(ele->text, "[red]");
					} else if (text.save_slot.operation == SAVE_SLOT_READ) {
						strcat(ele->text, "[green]");
					} else {
						strcat(ele->text, "[normal]");
					}
				} else {
					if (save_slot.state[i]) {
						strcat(ele->text, "[cyan]");
					} else {
						strcat(ele->text, "[gray]");
					}
				}
				snprintf(number, sizeof(number), "%d", i);
				strcat(ele->text, number);
			}

			if (!ele->surface) {
				gfx_text_create_surface(ele);
			}

			if ((cfg->fullscreen == FULLSCR) && render) {
				rendering(ele);
			}
		}
	}
}
void text_calculate_real_x_y(_txt_element *ele, int *x, int *y) {
	(*x) = ele->x;
	(*y) = ele->y;
	if (ele->start_x >= TXT_CENTER) {
		if (ele->start_x == TXT_CENTER) {
			(*x) = ((text.w - ele->w) >> 1) + ele->x;
		} else if (ele->start_x == TXT_LEFT) {
			(*x) = TXT_MARGIN + ele->x;
		} else if (ele->start_x == TXT_RIGHT) {
			(*x) = ((text.w - TXT_MARGIN) - ele->w) - ele->x;
		}
		if ((*x) < 0) {
			(*x) = 0;
		}
	}
	if (ele->start_y >= TXT_CENTER) {
		if (ele->start_y == TXT_CENTER) {
			(*y) = ((text.h - (ele->factor * font_size[ele->font][1])) >> 1) + ele->y;
		} else if (ele->start_y == TXT_UP) {
			(*y) = TXT_MARGIN + ele->y;
		} else if (ele->start_y == TXT_DOWN) {
			(*y) = ((text.h - TXT_MARGIN) - font_size[ele->font][1]) - ele->y;
		}
		if ((*y) < 0) {
			(*y) = 0;
		}
	}
}
void text_quit(void) {
	_txt_element *ele;
	int i;

	if (text.info.count) {
		for (i = 0; i < TXT_MAX_LINES; i++) {
			ele = text.info.lines[text.info.index][i];

			if (ele->enabled) {
				gfx_text_release_surface(ele);

				ele->enabled = FALSE;
			}
		}
	}

	if (text.single.count) {

		for (i = 0; i < TXT_MAX_LINES; i++) {
			ele = text.single.lines[i];

			if (ele != NULL) {
				gfx_text_release_surface(ele);

				free(text.single.lines[i]);
				text.single.lines[i] = NULL;

				text.single.count--;
			}
		}
	}

	// floppy
	{
		ele = &text.misc.floppy;

		if (ele->surface) {
			gfx_text_release_surface(ele);
		}
	}

	// controllers
	for (i = PORT1; i < PORT_MAX; i++) {
		ele = &text.misc.controllers[i];

		if (ele->surface) {
			gfx_text_release_surface(ele);
		}
	}

	// rewind
	{
		ele = &text.misc.rewind;

		if (ele->surface) {
			gfx_text_release_surface(ele);
		}
	}

	// fps
	{
		ele = &text.misc.fps;

		if (ele->surface) {
			gfx_text_release_surface(ele);
		}
	}

	// counter frames
	{
		ele = &text.misc.counter_frames;

		if (ele->surface) {
			gfx_text_release_surface(ele);
		}
	}

	// save slot
	{
		ele = &text.save_slot.slot;

		if (ele->surface) {
			gfx_text_release_surface(ele);
		}
	}
}

INLINE static void fade_ele(_txt_element *ele, int velocity) {
	static int diff;

	if (ele->alpha[0] == ele->alpha_start_fade) {
		diff = time(NULL) - ele->start;

		if (diff >= velocity) {
			ele->alpha[0] -= TXT_FADE_SPEED;
		}
	} else {
		if ((ele->alpha[0] -= TXT_FADE_SPEED) < TXT_FADE_SPEED) {
			ele->enabled = FALSE;
			ele->start = 0;
		}
		if (ele->alpha[1] > ele->alpha[0]) {
			ele->alpha[1] = ele->alpha[0];
		}
		if (ele->alpha[2] > ele->alpha[1]) {
			ele->alpha[2] = ele->alpha[1];
		}
	}
}
INLINE static void rendering(_txt_element *ele) {
	unsigned int i = 0;
	int font_x = 0, font_y = 0, ch_font = ele->font;
	uint32_t color[3], max_pixels = (text.w - 16), pixels = 0;
	_txt_rect surface_rect, font;

	text.on_screen = TRUE;

	font.x = 0;
	font.w = ele->factor * font_size[ch_font][0];

	{
		int x, y;

		text_calculate_real_x_y(ele, &x, &y);

		surface_rect.x = x;
		surface_rect.y = y;
		surface_rect.w = ele->w;
		surface_rect.h = ele->h;
	}

	color[0] = (ele->alpha[0] << 24) | txt_table[TXT_NORMAL];
	color[1] = (ele->alpha[1] << 24) | txt_table[TXT_BLACK];
	if (!ele->bck_color) {
		color[2] = (ele->alpha[2] << 24) | txt_table[TXT_BLACK];
	} else {
		color[2] = (ele->alpha[2] << 24) | txt_table[ele->bck_color];
	}

	ele->index = 0;

	for (pixels = ele->x; pixels < max_pixels;) {
		char ch = ' ';

		if (i < strlen(ele->text)) {
			ch = ele->text[i];
		} else {
			break;
		}

		if (ch == '[') {
			unsigned int tag, found = FALSE;

			for (tag = 0; tag < LENGTH(txt_tags); tag++) {
				int len = strlen(txt_tags[tag]);

				if (strncmp(ele->text + i, txt_tags[tag], len) == 0) {
					if (tag <= TXT_BLACK) {
						color[0] = (ele->alpha[0] << 24) | txt_table[tag];
						i += len;
						found = TRUE;
					} else if (tag <= TXT_FONT_12) {
						ch_font = tag - TXT_FONT_8;
						font.w = ele->factor * font_size[ch_font][0];
						i += len;
						found = TRUE;
					} else if (tag <= TXT_FLOPPY) {
						ch = tag - TXT_BUTTON_LEFT;
						i += len - 1;
						found = FALSE;
					}
					break;
				}
			}

			if (found) {
				continue;
			}
		}

		if ((pixels += font.w) >= max_pixels) {
			break;
		}

		switch (ch) {
/* riga 0 */
			case ' ':
				font_x = 0  * font_size[ch_font][0];
				font_y = 0  * font_size[ch_font][1];
				break;
			case '!':
				font_x = 1  * font_size[ch_font][0];
				font_y = 0  * font_size[ch_font][1];
				break;
			case '"':
				font_x = 2  * font_size[ch_font][0];
				font_y = 0  * font_size[ch_font][1];
				break;
			case '#':
				font_x = 3  * font_size[ch_font][0];
				font_y = 0  * font_size[ch_font][1];
				break;
			case '$':
				font_x = 4  * font_size[ch_font][0];
				font_y = 0  * font_size[ch_font][1];
				break;
			case '%':
				font_x = 5  * font_size[ch_font][0];
				font_y = 0  * font_size[ch_font][1];
				break;
			case '&':
				font_x = 6  * font_size[ch_font][0];
				font_y = 0  * font_size[ch_font][1];
				break;
			case 0x27: // '
				font_x = 7  * font_size[ch_font][0];
				font_y = 0  * font_size[ch_font][1];
				break;
/* riga 1 */
			case '(':
				font_x = 0  * font_size[ch_font][0];
				font_y = 1  * font_size[ch_font][1];
				break;
			case ')':
				font_x = 1  * font_size[ch_font][0];
				font_y = 1  * font_size[ch_font][1];
				break;
			case '*':
				font_x = 2  * font_size[ch_font][0];
				font_y = 1  * font_size[ch_font][1];
				break;
			case '+':
				font_x = 3  * font_size[ch_font][0];
				font_y = 1  * font_size[ch_font][1];
				break;
			case ',':
				font_x = 4  * font_size[ch_font][0];
				font_y = 1  * font_size[ch_font][1];
				break;
			case '-':
				font_x = 5  * font_size[ch_font][0];
				font_y = 1  * font_size[ch_font][1];
				break;
			case '.':
				font_x = 6  * font_size[ch_font][0];
				font_y = 1  * font_size[ch_font][1];
				break;
			case '/':
				font_x = 7  * font_size[ch_font][0];
				font_y = 1  * font_size[ch_font][1];
				break;
/* riga 2 */
			case '0':
				font_x = 0  * font_size[ch_font][0];
				font_y = 2  * font_size[ch_font][1];
				break;
			case '1':
				font_x = 1  * font_size[ch_font][0];
				font_y = 2  * font_size[ch_font][1];
				break;
			case '2':
				font_x = 2  * font_size[ch_font][0];
				font_y = 2  * font_size[ch_font][1];
				break;
			case '3':
				font_x = 3  * font_size[ch_font][0];
				font_y = 2  * font_size[ch_font][1];
				break;
			case '4':
				font_x = 4  * font_size[ch_font][0];
				font_y = 2  * font_size[ch_font][1];
				break;
			case '5':
				font_x = 5  * font_size[ch_font][0];
				font_y = 2  * font_size[ch_font][1];
				break;
			case '6':
				font_x = 6  * font_size[ch_font][0];
				font_y = 2  * font_size[ch_font][1];
				break;
			case '7':
				font_x = 7  * font_size[ch_font][0];
				font_y = 2  * font_size[ch_font][1];
				break;
/* riga 3 */
			case '8':
				font_x = 0  * font_size[ch_font][0];
				font_y = 3  * font_size[ch_font][1];
				break;
			case '9':
				font_x = 1  * font_size[ch_font][0];
				font_y = 3  * font_size[ch_font][1];
				break;
			case ':':
				font_x = 2  * font_size[ch_font][0];
				font_y = 3  * font_size[ch_font][1];
				break;
			case ';':
				font_x = 3  * font_size[ch_font][0];
				font_y = 3  * font_size[ch_font][1];
				break;
			case '<':
				font_x = 4  * font_size[ch_font][0];
				font_y = 3  * font_size[ch_font][1];
				break;
			case '=':
				font_x = 5  * font_size[ch_font][0];
				font_y = 3  * font_size[ch_font][1];
				break;
			case '>':
				font_x = 6  * font_size[ch_font][0];
				font_y = 3  * font_size[ch_font][1];
				break;
			case '?':
				font_x = 7  * font_size[ch_font][0];
				font_y = 3  * font_size[ch_font][1];
				break;
/* riga 4 */
			case '@':
				font_x = 0  * font_size[ch_font][0];
				font_y = 4  * font_size[ch_font][1];
				break;
			case 'A':
				font_x = 1  * font_size[ch_font][0];
				font_y = 4  * font_size[ch_font][1];
				break;
			case 'B':
				font_x = 2  * font_size[ch_font][0];
				font_y = 4  * font_size[ch_font][1];
				break;
			case 'C':
				font_x = 3  * font_size[ch_font][0];
				font_y = 4  * font_size[ch_font][1];
				break;
			case 'D':
				font_x = 4  * font_size[ch_font][0];
				font_y = 4  * font_size[ch_font][1];
				break;
			case 'E':
				font_x = 5  * font_size[ch_font][0];
				font_y = 4  * font_size[ch_font][1];
				break;
			case 'F':
				font_x = 6  * font_size[ch_font][0];
				font_y = 4  * font_size[ch_font][1];
				break;
			case 'G':
				font_x = 7  * font_size[ch_font][0];
				font_y = 4  * font_size[ch_font][1];
				break;
/* riga 5 */
			case 'H':
				font_x = 0  * font_size[ch_font][0];
				font_y = 5  * font_size[ch_font][1];
				break;
			case 'I':
				font_x = 1  * font_size[ch_font][0];
				font_y = 5  * font_size[ch_font][1];
				break;
			case 'J':
				font_x = 2  * font_size[ch_font][0];
				font_y = 5  * font_size[ch_font][1];
				break;
			case 'K':
				font_x = 3  * font_size[ch_font][0];
				font_y = 5  * font_size[ch_font][1];
				break;
			case 'L':
				font_x = 4  * font_size[ch_font][0];
				font_y = 5  * font_size[ch_font][1];
				break;
			case 'M':
				font_x = 5  * font_size[ch_font][0];
				font_y = 5  * font_size[ch_font][1];
				break;
			case 'N':
				font_x = 6  * font_size[ch_font][0];
				font_y = 5  * font_size[ch_font][1];
				break;
			case 'O':
				font_x = 7  * font_size[ch_font][0];
				font_y = 5  * font_size[ch_font][1];
				break;
/* riga 6 */
			case 'P':
				font_x = 0  * font_size[ch_font][0];
				font_y = 6  * font_size[ch_font][1];
				break;
			case 'Q':
				font_x = 1  * font_size[ch_font][0];
				font_y = 6  * font_size[ch_font][1];
				break;
			case 'R':
				font_x = 2  * font_size[ch_font][0];
				font_y = 6  * font_size[ch_font][1];
				break;
			case 'S':
				font_x = 3  * font_size[ch_font][0];
				font_y = 6  * font_size[ch_font][1];
				break;
			case 'T':
				font_x = 4  * font_size[ch_font][0];
				font_y = 6  * font_size[ch_font][1];
				break;
			case 'U':
				font_x = 5  * font_size[ch_font][0];
				font_y = 6  * font_size[ch_font][1];
				break;
			case 'V':
				font_x = 6  * font_size[ch_font][0];
				font_y = 6  * font_size[ch_font][1];
				break;
			case 'W':
				font_x = 7  * font_size[ch_font][0];
				font_y = 6  * font_size[ch_font][1];
				break;
/* riga 7 */
			case 'X':
				font_x = 0  * font_size[ch_font][0];
				font_y = 7  * font_size[ch_font][1];
				break;
			case 'Y':
				font_x = 1  * font_size[ch_font][0];
				font_y = 7  * font_size[ch_font][1];
				break;
			case 'Z':
				font_x = 2  * font_size[ch_font][0];
				font_y = 7  * font_size[ch_font][1];
				break;
			case '[':
				font_x = 3  * font_size[ch_font][0];
				font_y = 7  * font_size[ch_font][1];
				break;
			case '\\':
				font_x = 4  * font_size[ch_font][0];
				font_y = 7  * font_size[ch_font][1];
				break;
			case ']':
				font_x = 5  * font_size[ch_font][0];
				font_y = 7  * font_size[ch_font][1];
				break;
			case '^':
				font_x = 6  * font_size[ch_font][0];
				font_y = 7  * font_size[ch_font][1];
				break;
			case '_':
				font_x = 7  * font_size[ch_font][0];
				font_y = 7  * font_size[ch_font][1];
				break;
/* riga 8 */
			case '`':
				font_x = 0  * font_size[ch_font][0];
				font_y = 8  * font_size[ch_font][1];
				break;
			case 'a':
				font_x = 1  * font_size[ch_font][0];
				font_y = 8  * font_size[ch_font][1];
				break;
			case 'b':
				font_x = 2  * font_size[ch_font][0];
				font_y = 8  * font_size[ch_font][1];
				break;
			case 'c':
				font_x = 3  * font_size[ch_font][0];
				font_y = 8  * font_size[ch_font][1];
				break;
			case 'd':
				font_x = 4  * font_size[ch_font][0];
				font_y = 8  * font_size[ch_font][1];
				break;
			case 'e':
				font_x = 5  * font_size[ch_font][0];
				font_y = 8  * font_size[ch_font][1];
				break;
			case 'f':
				font_x = 6  * font_size[ch_font][0];
				font_y = 8  * font_size[ch_font][1];
				break;
			case 'g':
				font_x = 7  * font_size[ch_font][0];
				font_y = 8  * font_size[ch_font][1];
				break;
/* riga 9 */
			case 'h':
				font_x = 0  * font_size[ch_font][0];
				font_y = 9  * font_size[ch_font][1];
				break;
			case 'i':
				font_x = 1  * font_size[ch_font][0];
				font_y = 9  * font_size[ch_font][1];
				break;
			case 'j':
				font_x = 2  * font_size[ch_font][0];
				font_y = 9  * font_size[ch_font][1];
				break;
			case 'k':
				font_x = 3  * font_size[ch_font][0];
				font_y = 9  * font_size[ch_font][1];
				break;
			case 'l':
				font_x = 4  * font_size[ch_font][0];
				font_y = 9  * font_size[ch_font][1];
				break;
			case 'm':
				font_x = 5  * font_size[ch_font][0];
				font_y = 9  * font_size[ch_font][1];
				break;
			case 'n':
				font_x = 6  * font_size[ch_font][0];
				font_y = 9  * font_size[ch_font][1];
				break;
			case 'o':
				font_x = 7  * font_size[ch_font][0];
				font_y = 9  * font_size[ch_font][1];
				break;
/* riga 10 */
			case 'p':
				font_x = 0  * font_size[ch_font][0];
				font_y = 10 * font_size[ch_font][1];
				break;
			case 'q':
				font_x = 1  * font_size[ch_font][0];
				font_y = 10 * font_size[ch_font][1];
				break;
			case 'r':
				font_x = 2  * font_size[ch_font][0];
				font_y = 10 * font_size[ch_font][1];
				break;
			case 's':
				font_x = 3  * font_size[ch_font][0];
				font_y = 10 * font_size[ch_font][1];
				break;
			case 't':
				font_x = 4  * font_size[ch_font][0];
				font_y = 10 * font_size[ch_font][1];
				break;
			case 'u':
				font_x = 5  * font_size[ch_font][0];
				font_y = 10 * font_size[ch_font][1];
				break;
			case 'v':
				font_x = 6  * font_size[ch_font][0];
				font_y = 10 * font_size[ch_font][1];
				break;
			case 'w':
				font_x = 7  * font_size[ch_font][0];
				font_y = 10 * font_size[ch_font][1];
				break;
/* riga 11 */
			case 'x':
				font_x = 0  * font_size[ch_font][0];
				font_y = 11 * font_size[ch_font][1];
				break;
			case 'y':
				font_x = 1  * font_size[ch_font][0];
				font_y = 11 * font_size[ch_font][1];
				break;
			case 'z':
				font_x = 2  * font_size[ch_font][0];
				font_y = 11 * font_size[ch_font][1];
				break;
			case '{':
				font_x = 3  * font_size[ch_font][0];
				font_y = 11 * font_size[ch_font][1];
				break;
			case '|':
				font_x = 4  * font_size[ch_font][0];
				font_y = 11 * font_size[ch_font][1];
				break;
			case '}':
				font_x = 5  * font_size[ch_font][0];
				font_y = 11 * font_size[ch_font][1];
				break;
			case '~':
				font_x = 6  * font_size[ch_font][0];
				font_y = 11 * font_size[ch_font][1];
				break;
			case 8:
				font_x = 7  * font_size[ch_font][0];
				font_y = 11 * font_size[ch_font][1];
				break;
/* riga 12 */
			case 0:
				font_x = 0  * font_size[ch_font][0];
				font_y = 12  * font_size[ch_font][1];
				break;
			case 1:
				font_x = 1  * font_size[ch_font][0];
				font_y = 12  * font_size[ch_font][1];
				break;
			case 2:
				font_x = 2  * font_size[ch_font][0];
				font_y = 12  * font_size[ch_font][1];
				break;
			case 3:
				font_x = 3  * font_size[ch_font][0];
				font_y = 12  * font_size[ch_font][1];
				break;
			case 4:
			case 5:
				font_x = 4  * font_size[ch_font][0];
				font_y = 12  * font_size[ch_font][1];
				break;
			case 6:
			case 7:
				font_x = 5  * font_size[ch_font][0];
				font_y = 12  * font_size[ch_font][1];
				break;
		}
		{
			int x, y;
			_txt_rect rect;

			rect.w = ele->factor;
			rect.h = ele->factor;

			for (y = 0; y < font_size[ch_font][1]; y++) {
				char *list;

				if (ch_font == 0) {
					list = font_8x10[font_y] + font_x;
				} else {
					list = font_12x10[font_y] + font_x;
				}

				rect.y = y * ele->factor;

				for (x = 0; x < font_size[ch_font][0]; x++) {
					rect.x = font.x + (x * ele->factor);
					if (list[x] == '@') {
						gfx_text_rect_fill(ele, &rect, color[0]);
					} else if (list[x] == '+') {
						gfx_text_rect_fill(ele, &rect, color[1]);
					} else if (list[x] == '.') {
						gfx_text_rect_fill(ele, &rect, color[2]);
					} else {
						if (!ele->bck) {
							gfx_text_rect_fill(ele, &rect, 0);
						} else {
							gfx_text_rect_fill(ele, &rect, color[2]);
						}
					}
				}
				font_y++;
			}
		}
		font.x += font.w;

		ele->index++;
		i++;
	}

	text_blit(ele, &surface_rect);
}
