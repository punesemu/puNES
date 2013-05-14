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

static void INLINE rendering(_txt_element *txt);

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
					gfx_text_release_surface(ele->surface);
					gfx_text_release_surface(ele->blank);
					//SDL_FreeSurface(ele->surface);
					//SDL_FreeSurface(ele->blank);
					//ele->surface = NULL;
					//ele->blank = NULL;
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
						gfx_text_create_surface(ele->surface, ele->w, ele->h);
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
					gfx_text_release_surface(ele->surface);
					//SDL_FreeSurface(ele->surface);
					//ele->surface = NULL;

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
				gfx_text_release_surface(ele->surface);
				//SDL_FreeSurface(ele->surface);
				//ele->surface = NULL;
			}

			if (!ele->surface) {
				gfx_text_create_surface(ele->surface, ele->w, ele->h);
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
				gfx_text_create_surface(ele->surface, ele->w, ele->h);
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
				gfx_text_create_surface(ele->surface, ele->w, ele->h);
				//ele->surface = gfx_create_RGB_surface(text.surface, ele->w, ele->h);
			}

			if (render) {
				//rendering(ele);
			}
		}
	}
}
void text_quit(void) {
	if (text.info.count) {
		uint8_t i;

		for (i = 0; i < TXT_MAX_LINES; i++) {
			_txt_element *ele = text.info.lines[text.info.index][i];

			if (ele->enabled) {
				gfx_text_release_surface(ele->surface);
				//SDL_FreeSurface(ele->surface);
				//ele->surface = NULL;

				ele->enabled = FALSE;
			}
		}
	}

	if (text.single.count) {
		uint8_t i;

		for (i = 0; i < TXT_MAX_LINES; i++) {
			_txt_element *ele = text.single.lines[i];

			if (ele != NULL) {
				gfx_text_release_surface(ele->surface);
				//SDL_FreeSurface(ele->surface);
				//ele->surface = NULL;

				free(text.single.lines[i]);
				text.single.lines[i] = NULL;

				text.single.count--;
			}
		}
	}

	{
		_txt_element *ele = &text.tas.counter_frames;
		uint8_t i;

		if (ele->surface) {
			gfx_text_release_surface(ele->surface);
			//SDL_FreeSurface(ele->surface);
			//ele->surface = NULL;
		}

		for (i = 0; i < 4; i++) {
			_txt_element *ele = text.tas.controllers;

			if (ele->surface) {
				gfx_text_release_surface(ele->surface);
				//SDL_FreeSurface(ele->surface);
				//ele->surface = NULL;
			}
		}
	}

	{
		_txt_element *ele = &text.fds.floppy;

		if (ele->surface) {
			gfx_text_release_surface(ele->surface);
			//SDL_FreeSurface(ele->surface);
			//ele->surface = NULL;
		}
	}
}








static void INLINE rendering(_txt_element *ele) {
	int i = 0, font_x = 0, font_y = 0, ch_font = ele->font;
	uint32_t color[3], max_pixels = (text.w - 16), pixels = 0;
	_rect surface_rect, font;

	text.on_screen = TRUE;

	font.x = 0;
	font.w = ele->factor * font_size[ch_font][0];

	surface_rect.x = ele->x;
	surface_rect.y = ele->y;
	surface_rect.w = ele->w;
	surface_rect.h = ele->h;

	if (ele->start_x >= TXT_CENTER) {
		if (ele->start_x == TXT_CENTER) {
			surface_rect.x = ((text.w - ele->w) >> 1) + ele->x;
		} else if (ele->start_x == TXT_LEFT) {
			surface_rect.x = 8 + ele->x;
		} else if (ele->start_x == TXT_RIGHT) {
			surface_rect.x = ((text.w - 8) - ele->w) + ele->x;
		}
		if (surface_rect.x < 0) {
			surface_rect.x = 0;
		}
	}

	if (ele->start_y >= TXT_CENTER) {
		if (ele->start_y == TXT_CENTER) {
			surface_rect.y = ((text.h - (ele->factor * font_size[ch_font][1])) >> 1)
			        + ele->y;
		} else if (ele->start_y == TXT_UP) {
			surface_rect.y = 8 + ele->y;
		} else if (ele->start_y == TXT_DOWN) {
			surface_rect.y = ((text.h - 8) - font_size[ch_font][1]) + ele->y;
		}
		if (surface_rect.y < 0) {
			surface_rect.y = 0;
		}
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
			int tag, found = FALSE;

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
			_rect rect;

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
						gfx_text_rect_fill(ele->surface, &rect, color[0]);
						//SDL_FillRect(ele->surface, &rect, color[0]);
					} else if (list[x] == '+') {
						gfx_text_rect_fill(ele->surface, &rect, color[1]);
						//SDL_FillRect(ele->surface, &rect, color[1]);
					} else if (list[x] == '.') {
						gfx_text_rect_fill(ele->surface, &rect, color[2]);
						//SDL_FillRect(ele->surface, &rect, color[2]);
					} else {
						if (!ele->bck) {
							gfx_text_rect_fill(ele->surface, &rect, 0);
							//SDL_FillRect(ele->surface, &rect, 0);
						} else {
							gfx_text_rect_fill(ele->surface, &rect, color[2]);
							//SDL_FillRect(ele->surface, &rect, color[2]);
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

	//text_blit(ele, &surface_rect);
}
