/*
 * text.c
 *
 *  Created on: 02/mar/2013
 *      Author: fhorse
 */

#include "text.h"

enum txt_fade { FADE_SPEED = 4 };
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
static uint32_t txt_table[TXT_BLACK + 1];

void text_init(void) {
	uint8_t i;

	text_clear = sdl_text_clear;
	text_blit = sdl_text_blit;

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
void text_reset(void) {
	return;
}
void text_add_line(int type, int factor, int font, int alpha, int start_x, int start_y, int x,
        int y, const char *fmt, ...) {
	return;
}
void text_rendering(BYTE render) {
	return;
}
void text_quit(void) {
	return;
}
