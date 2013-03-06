/*
 * text.h
 *
 *  Created on: 01/mar/2013
 *      Author: fhorse
 */

#ifndef TEXT_H_
#define TEXT_H_

#include "common.h"

#define text_add_line_info(factor, ...)\
	text_add_line(0, 0, 0, 0, 0, 0, 0, 0, __VA_ARGS__)
#define text_add_line_single(factor, font, alpha, start_x, start_y, x, y, ...)\
	text_add_line(0, 0, 0, 0, 0, 0, 0, 0, __VA_ARGS__)

void text_init(void);
void text_reset(void);
void text_add_line(int type, int factor, int font, int alpha, int start_x, int start_y, int x,
        int y, const char *fmt, ...);
void text_rendering(BYTE render);
void text_quit(void);

#endif /* TEXT_H_ */
