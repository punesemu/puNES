/*
 * sdltext.c
 *
 *  Created on: 04/feb/2012
 *      Author: fhorse
 */

#include <time.h>
#include <SDL.h>
#include "sdltext.h"
#include "sdlgfx.h"
#include "font.h"
#include "tas.h"
#include "ppu.h"
#include "input.h"
#include "fds.h"

#define TXT_MAX_MSG 256
#define TXT_MAX_LINES 10
#define create_tmp_surface()\
	if (tmp) {\
		SDL_FreeSurface(tmp);\
	}\
	tmp_rect.w = txt->factor * font_size[ch_font][0];\
	tmp_rect.h = txt->factor * font_size[ch_font][1];\
	tmp = SDL_DisplayFormatAlpha(SDL_CreateRGBSurface(surface->flags, tmp_rect.w, tmp_rect.h,\
			surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask,\
			surface->format->Bmask, surface->format->Amask))
#define port_control(prt, button, ch)\
	if (prt.data[button] == PRESSED) {\
		strcat(ele->text, "[yellow]");\
	} else {\
		strcat(ele->text, "[black]");\
	}\
	strcat(ele->text, ch)

enum txt_tgs {
	TXT_NORMAL,
	TXT_RED,
	TXT_YELLOW,
	TXT_GREEN,
	TXT_CYAN,
	TXT_BROWN,
	TXT_BLUE,
	TXT_BLACK,
	TXT_FONT_8,
	TXT_FONT_12,
	TXT_BUTTON_LEFT,
	TXT_BUTTON_RIGHT,
	TXT_BUTTON_UP,
	TXT_BUTTON_DOWN,
	TXT_BUTTON_SELECT,
	TXT_BUTTON_START,
	TXT_BUTTON_A,
	TXT_BUTTON_B,
	TXT_FLOPPY,
	TXT_TAGS
};

typedef struct {
	uint8_t enabled;
	uint8_t bck;
	uint8_t bck_color;
	uint8_t font;
	uint8_t factor;
	int start_x;
	int start_y;
	int x;
	int y;
	int alpha_start_fade;
	int alpha[3];
	time_t start;
	char text[TXT_MAX_MSG];
	int length;
	int length_pixels;
	int index;
} _txt_element;
struct _txt_info {
	uint8_t index;
	uint8_t count;
	_txt_element buffers[TXT_MAX_LINES];
	_txt_element *lines[2][TXT_MAX_LINES];
} txt_info;
struct _txt_single {
	uint8_t count;
	_txt_element *lines[TXT_MAX_LINES];
} txt_single;
struct _txt_tas {
	_txt_element counter_frames;
	_txt_element controllers[4];
} txt_tas;
struct _txt_fds {
	_txt_element floppy;
} txt_fds;
static char txt_tags[][10] = {
	"[normal]",
	"[red]",
	"[yellow]",
	"[green]",
	"[cyan]",
	"[brown]",
	"[blue]",
	"[black]",
	"[font8]",
	"[font12]",
	"[left]",
	"[right]",
	"[up]",
	"[down]",
	"[select]",
	"[start]",
	"[a]",
	"[b]",
	"[floppy]"
};
static uint32_t txt_table[TXT_BLACK + 1];

static void INLINE rendering(_txt_element *txt, SDL_Surface *pScreen);

void textInit(void) {
	uint8_t i;

	txt_info.index = 0;

	memset(txt_info.buffers, 0, sizeof(_txt_element) * TXT_MAX_LINES);

	for (i = 0; i < TXT_MAX_LINES; i++) {
		txt_info.lines[txt_info.index][i] = &txt_info.buffers[i];
		txt_single.lines[i] = NULL;
	}

	{
		_txt_element *ele;
		uint8_t i;

		ele = &txt_tas.counter_frames;
		ele->bck = TRUE;
		ele->bck_color = TXT_BLACK;
		ele->font = FONT_8X10;
		ele->factor = 1;
		ele->alpha[0] = 255;
		ele->alpha[1] = 170;
		ele->alpha[2] = 70;
		ele->start_x = TXTLEFT;
		ele->start_y = TXTUP;
		ele->x = 0;
		ele->y = 1 * font_size[ele->font][1];

		for (i = 0; i < 4; i++) {
			ele = &txt_tas.controllers[i];
			ele->bck = TRUE;
			ele->bck_color = TXT_BLUE;
			ele->font = FONT_8X10;
			ele->factor = 1;
			ele->alpha[0] = 255;
			ele->alpha[1] = 170;
			ele->alpha[2] = 150;
			ele->length_pixels = 8 * font_size[ele->font][0];
			ele->start_x = TXTLEFT;
			ele->start_y = TXTUP;
			ele->x = (i * (8 + 1)) * font_size[ele->font][0];
			ele->y = 0;
		}
	}
}
void textReset(void *surface) {
	SDL_Surface *srf = surface;

	txt_table[TXT_NORMAL] = SDL_MapRGBA(srf->format, 0xFF, 0xFF, 0xFF, 0);
	txt_table[TXT_RED]    = SDL_MapRGBA(srf->format, 0xFF, 0x4C, 0x3E, 0);
	txt_table[TXT_YELLOW] = SDL_MapRGBA(srf->format, 0xFF, 0xFF, 0   , 0);
	txt_table[TXT_GREEN]  = SDL_MapRGBA(srf->format, 0   , 0xFF, 0   , 0);
	txt_table[TXT_CYAN]   = SDL_MapRGBA(srf->format, 0   , 0xFF, 0xFF, 0);
	txt_table[TXT_BROWN]  = SDL_MapRGBA(srf->format, 0xEB, 0x89, 0x31, 0);
	txt_table[TXT_BLUE]   = SDL_MapRGBA(srf->format, 0x2D, 0x8D, 0xBD, 0);
	txt_table[TXT_BLACK]  = SDL_MapRGBA(srf->format, 0   , 0   , 0   , 0);
}
void textAddLine(int type, int factor, int font, int alpha, int start_x, int start_y, int x, int y,
        const char *fmt, ...) {
	uint8_t i, new = !txt_info.index;
	_txt_element *ele = NULL;
	va_list ap;

	if (type == TXTINFO) {
		for (i = 0; i < TXT_MAX_LINES; i++) {
			if (i == (TXT_MAX_LINES - 1)) {
				txt_info.lines[new][0] = txt_info.lines[txt_info.index][i];
			} else {
				txt_info.lines[new][i + 1] = txt_info.lines[txt_info.index][i];
			}
		}

		txt_info.index = new;

		ele = txt_info.lines[txt_info.index][0];
		ele->enabled = TRUE;
		ele->bck = FALSE;
		ele->font = font;
		ele->factor = factor;
		if (!txt_info.lines[txt_info.index][1]->start) {
			ele->start = time(NULL);
		} else {
			ele->start = txt_info.lines[txt_info.index][1]->start + 1;
		}
		ele->alpha_start_fade = alpha;
		ele->alpha[0] = alpha;
		ele->alpha[1] = 170;
		ele->alpha[2] = 40;
	}

	if (type == TXTSINGLE) {
		for (i = 0; i < TXT_MAX_LINES; i++) {
			if (txt_single.lines[i] == NULL) {
				txt_single.lines[i] = malloc(sizeof(_txt_element));
				txt_single.count++;

				ele = txt_single.lines[i];

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
		ele->length_pixels += ele->factor * font_size[ch_font][0];
		ele->length++;
	}
}
void textRendering(BYTE render, void *surface) {
	SDL_Surface *srf = surface;

	#define FADE_SPEED 4

	{
		int pos_x = 8, pos_y = srf->h - 8;
		uint8_t i;

		for (i = 0; i < TXT_MAX_LINES; i++) {
			_txt_element *ele = txt_info.lines[txt_info.index][i];

			if (ele->enabled == TRUE) {
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
				if ((gfx.scale != X1) && render){
					rendering(ele, srf);
				}
			}
		}
	}

	if (txt_single.count) {
		uint8_t i;

		for (i = 0; i < TXT_MAX_LINES; i++) {
			_txt_element *ele = txt_single.lines[i];

			if (ele != NULL) {
				if (ele->enabled == TRUE) {
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
					if ((gfx.scale != X1) && render){
						rendering(ele, srf);
					}
				} else {
					free(txt_single.lines[i]);
					txt_single.lines[i] = NULL;
					txt_single.count--;
				}
			}
		}
	}

	if (tas.type) {
		_txt_element *ele = &txt_tas.counter_frames;

		/* counter frames */
		{
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

			ele->length_pixels = length * font_size[ele->font][0];
			if (render){
				rendering(ele, srf);
			}
		}

		{
			ele = &txt_tas.controllers[0];

			ele->text[0] = 0;
			port_control(port1, UP, "[up]");
			port_control(port1, DOWN, "[down]");
			port_control(port1, LEFT, "[left]");
			port_control(port1, RIGHT, "[right]");
			port_control(port1, SELECT, "[select]");
			port_control(port1, START, "[start]");
			port_control(port1, BUT_A, "[a]");
			port_control(port1, BUT_B, "[b]");

			if (render){
				rendering(ele, srf);
			}
		}
	}

	if (fds.info.enabled) {
		_txt_element *ele = &txt_fds.floppy;

		if ((fds.info.last_operation | fds.drive.disk_ejected)) {
			ele->enabled = TRUE;
			ele->bck = FALSE;
			ele->font = FONT_12X10;
			ele->factor = 1;
			ele->start_x = TXTRIGHT;
			ele->start_y = TXTDOWN;
			ele->x = 0;
			ele->y = 0;
			ele->alpha[0] = 180;
			ele->alpha[2] = 0;
			ele->length_pixels = font_size[ele->font][0];

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

			if (render){
				rendering(ele, srf);
			}
		}
	}
}
void textQuit(void) {
	if (txt_single.count) {
		uint8_t i;

		for (i = 0; i < TXT_MAX_LINES; i++) {
			_txt_element *ele = txt_single.lines[i];

			if (ele != NULL) {
				free(txt_single.lines[i]);
				txt_single.lines[i] = NULL;
				txt_single.count--;
			}
		}
	}
	return;
}

static void INLINE rendering(_txt_element *txt, SDL_Surface *surface) {
	int i = 0, font_x = 0, font_y = 0, ch_font = txt->font;
	uint32_t color[3], max_pixels = (surface->w - 16), pixels = 0;
	SDL_Surface *tmp = NULL;
	SDL_Rect surface_rect, tmp_rect;

	surface_rect.x = txt->x;
	surface_rect.y = txt->y;
	surface_rect.w = 0;
	surface_rect.h = 0;

	if (txt->start_x >= TXTCENTER) {
		if (txt->start_x == TXTCENTER) {
			surface_rect.x = ((surface->w - txt->length_pixels) >> 1) + txt->x;
		} else if (txt->start_x == TXTLEFT) {
			surface_rect.x = 8 + txt->x;
		} else if (txt->start_x == TXTRIGHT) {
			surface_rect.x = ((surface->w - 8) - txt->length_pixels) + txt->x;
		}
		if (surface_rect.x < 0) {
			surface_rect.x = 0;
		}
	}

	if (txt->start_y >= TXTCENTER) {
		if (txt->start_y == TXTCENTER) {
			surface_rect.y = ((surface->h - (txt->factor * font_size[ch_font][1])) >> 1) + txt->y;
		} else if (txt->start_y == TXTUP) {
			surface_rect.y = 8 + txt->y;
		} else if (txt->start_y == TXTDOWN) {
			surface_rect.y = ((surface->h - 8) - font_size[ch_font][1]) + txt->y;
		}
		if (surface_rect.y < 0) {
			surface_rect.y = 0;
		}
	}

	tmp_rect.x = 0;
	tmp_rect.y = 0;

	create_tmp_surface();

	color[0] = (txt->alpha[0] << 24) | txt_table[TXT_NORMAL];
	color[1] = (txt->alpha[1] << 24) | txt_table[TXT_BLACK];
	if (!txt->bck_color) {
		color[2] = (txt->alpha[2] << 24) | txt_table[TXT_BLACK];
	} else {
		color[2] = (txt->alpha[2] << 24) | txt_table[txt->bck_color];
	}

	txt->index = 0;

	for (pixels = 0; pixels < max_pixels;) {
		char ch = ' ';

		if (i < strlen(txt->text)) {
			ch = txt->text[i];
		} else {
			break;
		}

		if (ch == '[') {
			int tag, found = FALSE;

			for (tag = 0; tag < LENGTH(txt_tags); tag++) {
				int len = strlen(txt_tags[tag]);

				if (strncmp(txt->text + i, txt_tags[tag], len) == 0) {
					if (tag <= TXT_BLACK) {
						color[0] = (txt->alpha[0] << 24) | txt_table[tag];
						i += len;
						found = TRUE;
					} else if (tag <= TXT_FONT_12) {
						ch_font = tag - TXT_FONT_8;
						create_tmp_surface();
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

		if ((pixels += tmp_rect.w) >= max_pixels) {
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
			SDL_Rect rect;

			rect.w = txt->factor;
			rect.h = txt->factor;

			for (y = 0; y < font_size[ch_font][1]; y++) {
				char *list;

				if (ch_font == 0) {
					list = font_8x10[font_y] + font_x;
				} else {
					list = font_12x10[font_y] + font_x;
				}

				rect.y = y * txt->factor;

				for (x = 0; x < font_size[ch_font][0]; x++) {
					rect.x = x * txt->factor;
					if (list[x] == '@') {
						SDL_FillRect(tmp, &rect, color[0]);
					} else if (list[x] == '+') {
						SDL_FillRect(tmp, &rect, color[1]);
					} else if (list[x] == '.') {
						SDL_FillRect(tmp, &rect, color[2]);
					} else {
						if (!txt->bck) {
							SDL_FillRect(tmp, &rect, 0);
						} else {
							SDL_FillRect(tmp, &rect, color[2]);
						}
					}
				}
				font_y++;
			}
			SDL_BlitSurface(tmp, &tmp_rect, surface, &surface_rect);
		}

		surface_rect.x += tmp_rect.w;

		txt->index++;
		i++;
	}
	SDL_FreeSurface(tmp);
}
