/*
 * shaders.h
 *
 *  Created on: 09/mag/2012
 *      Author: fhorse
 */

#ifndef SHADERS_H_
#define SHADERS_H_

enum shader_type {
	SHADER_COLOR,
	SHADER_NO_FILTER,
	SHADER_SCALE2X,
	SHADER_SCALE3X,
	SHADER_SCALE4X,
	SHADER_HQ2X,
	SHADER_HQ4X,
	SHADER_4xBR,
	SHADER_PIXELLATE,
	SHADER_PHOSPHOR,
	SHADER_SCANLINE,
	SHADER_QUILAZ,
	SHADER_WATERPAINT,
	SHADER_CRT,
	SHADER_CRT2,
	SHADER_CRT3,
	SHADER_CRT4,
	SHADER_BLOOM,
	SHADER_DONTBLOOM,
	SHADER_NTSC,
	SHADER_NTSC2,
	SHADER_NTSC3,
	SHADER_TOON,
	SHADER_DARKSCREEN,
	SHADER_TOTAL,
	SHADER_NONE = 255
};

typedef struct {
	GLuint data;
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
		GLint param;
		GLint pixel_aspect_ratio;
	} loc;
} _shader;

_shader shader;

#endif /* SHADERS_H_ */

#if defined (_SHADERS_CODE_)
static _shader_code shader_code[SHADER_TOTAL] = {
#include "shaders/color.h"
#include "shaders/no_filter.h"
#include "shaders/scale2x.h"
#include "shaders/scale3x.h"
#include "shaders/scale4x.h"
#include "shaders/hq2x.h"
#include "shaders/hq4x.h"
#include "shaders/4xrb.h"
#include "shaders/pixellate.h"
#include "shaders/phosphor.h"
#include "shaders/scanline.h"
#include "shaders/quilaz.h"
#include "shaders/waterpaint.h"
#include "shaders/crt.h"
#include "shaders/crt2.h"
#include "shaders/crt3.h"
#include "shaders/crt4.h"
#include "shaders/bloom.h"
#include "shaders/dbl.h"
#include "shaders/ntsc.h"
#include "shaders/ntsc2.h"
#include "shaders/ntsc3.h"
#include "shaders/toon.h"
};
#undef _SHADERS_CODE_
#endif
