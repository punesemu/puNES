/*
 * text.c
 *
 *  Created on: 02/mar/2013
 *      Author: fhorse
 */

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "text.h"
#include "font.h"
#include "cfg_file.h"
#include "gfx.h"
#include "tas.h"
#include "input.h"
#include "fds.h"

enum txt_fade { FADE_SPEED = 4 };

#define port_control(prt, button, ch)\
	if (prt.data[button] == PRESSED) {\
		strcat(ele->text, "[yellow]");\
	} else {\
		strcat(ele->text, "[black]");\
	}\
	strcat(ele->text, ch)

static char txt_tags[][10] = {
	"[normal]", "[red]",	"[yellow]",	"[green]",
	"[cyan]"  ,	"[brown]",	"[blue]"  ,	"[black]",
	"[font8]" , "[font12]",	"[left]"  ,	"[right]",
	"[up]"    ,	"[down]",	"[select]",	"[start]",
	"[a]"     ,	"[b]"   ,	"[floppy]"
};

void text_init(void) {
	uint8_t i;

	//text_clear = sdl_text_clear;
	//text_blit = sdl_text_blit;

	memset(&text, 0, sizeof(text));

	for (i = 0; i < TXT_MAX_LINES; i++) {
		text.info.lines[text.info.index][i] = &text.info.buffers[i];
		text.single.lines[i] = NULL;
	}

	{
		_txt_element *ele;

		ele = &text.tas.counter_frames;
		ele->bck = TRUE;
		ele->bck_color = TXT_BLACK;
		ele->font = FONT_8X10;
		ele->factor = 1;
		ele->alpha[0] = 255;
		ele->alpha[1] = 170;
		ele->alpha[2] = 70;
		ele->start_x = TXT_LEFT;
		ele->start_y = TXT_UP;
		ele->x = 0;
		ele->y = 1 * font_size[ele->font][1];

		for (i = 0; i < 4; i++) {
			ele = &text.tas.controllers[i];
			ele->bck = TRUE;
			ele->bck_color = TXT_BLUE;
			ele->font = FONT_8X10;
			ele->factor = 1;
			ele->alpha[0] = 255;
			ele->alpha[1] = 170;
			ele->alpha[2] = 150;
			ele->start_x = TXT_LEFT;
			ele->start_y = TXT_UP;
			ele->x = (i * (8 + 1)) * font_size[ele->font][0];
			ele->y = 0;
			ele->w = 8 * font_size[ele->font][0];
			ele->h = font_size[ele->font][1];
		}
	}
}
void text_add_line(int type, int factor, int font, int alpha, int start_x, int start_y, int x,
        int y, const char *fmt, ...) {
	uint8_t i, new = !text.info.index;
	_txt_element *ele = NULL;
	va_list ap;

	if (type == TXT_INFO) {
		for (i = 0; i < TXT_MAX_LINES; i++) {
			if (text.info.lines[text.info.index][i]->enabled) {
				//text_clear(text.info.lines[text.info.index][i]);
			}
			if (i == (TXT_MAX_LINES - 1)) {
				text.info.lines[new][0] = text.info.lines[text.info.index][i];
			} else {
				text.info.lines[new][i + 1] = text.info.lines[text.info.index][i];
			}
		}

		text.info.index = new;

		ele = text.info.lines[text.info.index][0];
		ele->enabled = TRUE;
		ele->bck = FALSE;
		ele->font = font;
		ele->factor = factor;
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
				text.single.lines[i] = malloc(sizeof(_txt_element));
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
			int tag, found = FALSE;

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
		//SDL_FreeSurface(ele->surface);
		ele->surface = NULL;
	}
}
void text_rendering(BYTE render) {
	text.on_screen = FALSE;

	{
		int pos_x = 8, pos_y = text.h - 8;
		uint8_t i;

		for (i = 0; i < TXT_MAX_LINES; i++) {
			_txt_element *ele = text.info.lines[text.info.index][i];

			if (ele->enabled == TRUE) {
				if (!ele->surface) {
					gfx_text_create_surface(ele->surface, ele->w, ele->h);
					gfx_text_create_surface(ele->blank, ele->w, ele->h);
					//ele->surface = gfx_create_RGB_surface(text.surface, ele->w, ele->h);
					//ele->blank = gfx_create_RGB_surface(text.surface, ele->w, ele->h);
				}

				if (ele->alpha[0] == ele->alpha_start_fade) {
					int diff = time(NULL) - ele->start;

					if (diff >= 6) {
						ele->alpha[0] -= FADE_SPEED;
					}
				} else {
					if ((ele->alpha[0] -= FADE_SPEED) < FADE_SPEED) {
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
				pos_y -= font_size[ele->font][1] * ele->factor;
				ele->start_x = 0;
				ele->start_y = 0;
				ele->x = pos_x;
				ele->y = pos_y;

				if ((cfg->scale != X1) && render){
					//rendering(ele);
				}

				if (!ele->enabled) {
					//SDL_FreeSurface(ele->surface);
					//SDL_FreeSurface(ele->blank);
					ele->surface = NULL;
					ele->blank = NULL;
				}
			}
		}
	}

	if (text.single.count) {
		uint8_t i;

		for (i = 0; i < TXT_MAX_LINES; i++) {
			_txt_element *ele = text.single.lines[i];

			if (ele != NULL) {
				if (ele->enabled == TRUE) {
					if (!ele->surface) {
						//ele->surface = gfx_create_RGB_surface(text.surface, ele->w, ele->h);
					}

					if (ele->alpha[0] == ele->alpha_start_fade) {
						int diff = time(NULL) - ele->start;

						if (diff >= 6) {
							ele->alpha[0] -= FADE_SPEED;
						}
					} else {
						if ((ele->alpha[0] -= FADE_SPEED) < FADE_SPEED) {
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
					if ((cfg->scale != X1) && render){
						//rendering(ele);
					}
				} else {
					//SDL_FreeSurface(ele->surface);
					ele->surface = NULL;

					free(text.single.lines[i]);
					text.single.lines[i] = NULL;

					text.single.count--;
				}
			}
		}
	}

	if (tas.type) {
		_txt_element *ele = &text.tas.counter_frames;

		/* counter frames */
		{
			int old_w = ele->w;

			int length;
			if (tas.lag_frame) {
				sprintf(ele->text, "%d/%d [red]%d[normal]", tas.frame, tas.total,
				        tas.total_lag_frames);
				length = strlen(ele->text) - 13;
			} else {
				sprintf(ele->text, "%d/%d [green]%d[normal]", tas.frame, tas.total,
				        tas.total_lag_frames);
				length = strlen(ele->text) - 15;
			}

			ele->w = length * font_size[ele->font][0];
			ele->h = font_size[ele->font][1];

			if ((old_w != ele->w) && ele->surface) {
				//SDL_FreeSurface(ele->surface);
				ele->surface = NULL;
			}

			if (!ele->surface) {
				//ele->surface = gfx_create_RGB_surface(text.surface, ele->w, ele->h);
			}

			if (render) {
				//rendering(ele);
			}
		}

		{
			ele = &text.tas.controllers[0];

			ele->text[0] = 0;
			port_control(port1, UP, "[up]");
			port_control(port1, DOWN, "[down]");
			port_control(port1, LEFT, "[left]");
			port_control(port1, RIGHT, "[right]");
			port_control(port1, SELECT, "[select]");
			port_control(port1, START, "[start]");
			port_control(port1, BUT_A, "[a]");
			port_control(port1, BUT_B, "[b]");

			if (!ele->surface) {
				//ele->surface = gfx_create_RGB_surface(text.surface, ele->w, ele->h);
			}

			if (render) {
				//rendering(ele);
			}
		}
	}

	if (fds.info.enabled) {
		_txt_element *ele = &text.fds.floppy;

		if ((fds.info.last_operation | fds.drive.disk_ejected)) {
			ele->enabled = TRUE;
			ele->bck = FALSE;
			ele->font = FONT_12X10;
			ele->factor = 1;
			ele->start_x = TXT_RIGHT;
			ele->start_y = TXT_DOWN;
			ele->x = 0;
			ele->y = 0;
			ele->w = font_size[ele->font][0];
			ele->h = font_size[ele->font][1];
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
				fds.info.last_operation = FDS_OP_NONE;
			}
			strcat(ele->text, "[floppy]");

			if (!ele->surface) {
				//ele->surface = gfx_create_RGB_surface(text.surface, ele->w, ele->h);
			}

			if (render) {
				//rendering(ele);
			}
		}
	}
}
void text_quit(void) {
	return;
}
