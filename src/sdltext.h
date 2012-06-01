/*
 * sdltext.h
 *
 *  Created on: 04/feb/2012
 *      Author: fhorse
 */

#ifndef SDLTEXT_H_
#define SDLTEXT_H_

#include <SDL.h>
#include "common.h"

enum txt_type {
	TXTINFO,
	TXTSINGLE
};
enum txt_position {
	TXTCENTER = 65000,
	TXTLEFT,
	TXTRIGHT,
	TXTUP,
	TXTDOWN
};
enum txt_fonts {
	FONT_8X10,
	FONT_12X10,
};

#define TXT_MAX_MSG 256
#define TXT_MAX_LINES 10

#define textAddLineInfo(factor, ...)\
	textAddLine(TXTINFO, factor, FONT_12X10, 255, 0, 0, 0, 0, __VA_ARGS__)
#define textAddLineSingle(factor, font, alpha, start_x, start_y, x, y, ...)\
	textAddLine(TXTSINGLE, factor, font, alpha, start_x, start_y, x, y, __VA_ARGS__)

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
	int w;
	int h;
	int alpha_start_fade;
	int alpha[3];
	time_t start;
	char text[TXT_MAX_MSG];
	int length;

	int index;

	SDL_Surface *surface;
} _txt_element;

struct _text {
	SDL_Surface *surface;
	BYTE on_screen;
	uint32_t w;
	uint32_t h;

	struct _txt_info {
		uint8_t index;
		uint8_t count;
		_txt_element buffers[TXT_MAX_LINES];
		_txt_element *lines[2][TXT_MAX_LINES];
	} info;
	struct _txt_single {
		uint8_t count;
		_txt_element *lines[TXT_MAX_LINES];
	} single;
	struct _txt_tas {
		_txt_element counter_frames;
		_txt_element controllers[4];
	} tas;
	struct _txt_fds {
		_txt_element floppy;
	} fds;
} text;

void textInit(void);
void textReset(void);
void textAddLine(int type, int factor, int font, int alpha, int start_x, int start_y, int x, int y,
        const char *fmt, ...);
void textRendering(BYTE render);
void textQuit(void);

void sdl_text_blit(_txt_element *ele, SDL_Rect *dst_rect);
void (*text_blit)(_txt_element *ele, SDL_Rect *dst_rect);

#endif /* SDLTEXT_H_ */
