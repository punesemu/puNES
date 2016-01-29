/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

#ifndef SHADERS_H_
#define SHADERS_H_

enum shader_type {
	SHADER_COLOR,
	SHADER_NO_FILTER,
	SHADER_PHOSPHOR,
	SHADER_SCANLINE,
	SHADER_CRT,
	SHADER_DONTBLOOM,
	SHADER_TOTAL,
	SHADER_NONE = 255
};

typedef struct {
	GLuint id;
	GLenum format;
	GLenum type;
	GLint format_internal;

	GLfloat w;
	GLfloat h;
} _texture;
typedef struct {
	const GLchar *vertex;
	const GLchar *fragment;
} _shader_code;
typedef struct {
	GLuint prg;
	GLuint vrt;
	GLuint frg;

	GLuint id;
	_shader_code *code;

	struct {
		struct {
			GLint screen_emu;
			GLint video_mode;
			GLint texture;
		} size;
		struct {
			GLint scr;
		} texture;
		GLint frame_counter;
		GLint pixel_aspect_ratio;
		GLint full_interpolation;
		GLint param;
	} loc;
} _shader;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC _shader shader;

#undef EXTERNC

#endif /* SHADERS_H_ */

#if defined (_SHADERS_CODE_)
static _shader_code shader_code[SHADER_TOTAL] = {
#include "shaders/color.h"
#include "shaders/no_filter.h"
#include "shaders/phosphor.h"
#include "shaders/scanline.h"
#include "shaders/crt.h"
#include "shaders/dbl.h"
};
#undef _SHADERS_CODE_
#endif
