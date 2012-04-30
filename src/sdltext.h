/*
 * sdltext.h
 *
 *  Created on: 04/feb/2012
 *      Author: fhorse
 */

#ifndef SDLTEXT_H_
#define SDLTEXT_H_

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

#define textAddLineInfo(factor, ...)\
	textAddLine(TXTINFO, factor, FONT_12X10, 255, 0, 0, 0, 0, __VA_ARGS__)
#define textAddLineSingle(factor, font, alpha, start_x, start_y, x, y, ...)\
	textAddLine(TXTSINGLE, factor, font, alpha, start_x, start_y, x, y, __VA_ARGS__)

void textInit(void);
void textReset(void *surface);
void textAddLine(int type, int factor, int font, int alpha, int start_x, int start_y, int x, int y,
        const char *fmt, ...);
void textRendering(BYTE render, void *surface);
void textQuit(void);

#endif /* SDLTEXT_H_ */
