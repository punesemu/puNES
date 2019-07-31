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

#ifndef SHADERS_H_
#define SHADERS_H_

#include "common.h"

enum meta_shader_type { MS_MEM, MS_CGP, MS_GLSLP };
enum max_pass { MAX_PASS = 24, MAX_PREV = 7, MAX_PARAM = 128 };
enum texture_wrap_type {
	TEXTURE_WRAP_BORDER,
	TEXTURE_WRAP_EDGE,
	TEXTURE_WRAP_REPEAT,
	TEXTURE_WRAP_MIRRORED_REPEAT
};
enum texture_filter_type {
	TEXTURE_LINEAR_DISAB,
	TEXTURE_LINEAR_ENAB,
	TEXTURE_LINEAR_DEFAULT,
};
enum shader_scale_type {
	SHADER_SCALE_INPUT,
	SHADER_SCALE_ABSOLUTE,
	SHADER_SCALE_VIEWPORT,
	SHADER_SCALE_DEFAULT
};
enum shader_errors {
	EXIT_ERROR_SHADER = EXIT_ERROR + 1
};

typedef struct _xy_uint {
	unsigned int x, y;
} _xy_uint;
typedef struct _xy_float {
	float x, y;
} _xy_float;
typedef struct _wh_uint {
	unsigned int w, h;
} _wh_uint;
typedef struct _shader_scale {
	_xy_uint type;
	_xy_float scale;
	_xy_uint abs;
} _shader_scale;
typedef struct _shader_pass {
	uint8_t type;

	char *code;
	uTCHAR path[LENGTH_FILE_NAME_LONG];
	char alias[64];

	uint8_t mipmap_input;
	uint8_t linear;
	uint8_t fbo_flt;
	uint8_t fbo_srgb;
	uint8_t wrap;
	int frame_count_mod;

	_shader_scale sc;
} _shader_pass;
typedef struct _lut_pass {
	char name[64];
	uTCHAR path[LENGTH_FILE_NAME_LONG];

	uint8_t mipmap;
	uint8_t linear;
	uint8_t wrap;
} _lut_pass;
typedef struct _param_shd {
	char name[64];
	char desc[64];

	float value; //current
	float initial;
	float min;
	float max;
	float step;
} _param_shd;
typedef struct _shader_effect {
	uint8_t type;

	uint8_t pass;
	uint8_t last_pass;
	uint8_t running_pass;
	_shader_pass sp[MAX_PASS + 1];

	uint8_t luts;
	_lut_pass lp[MAX_PASS];

	uint8_t params;
	_param_shd param[MAX_PARAM];

	int8_t feedback_pass;
} _shader_effect;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC _shader_effect shader_effect;

EXTERNC BYTE shaders_set(int shader);
EXTERNC void shader_se_set_default(_shader_effect *se);
EXTERNC char *shader_code_blend(void);

#undef EXTERNC

#endif /* SHADERS_H_ */
