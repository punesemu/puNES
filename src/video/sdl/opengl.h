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

#ifndef OPENGL_H_
#define OPENGL_H_

#include "glew/glew.h"
#include <SDL.h>
#if defined (WITH_OPENGL_CG)
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#endif
#include "shaders.h"
#include "common.h"
#include "gfx.h"

#if defined (WITH_OPENGL_CG)
typedef struct _shader_prg_cg {
	CGprogram v, f;
} _shader_prg_cg;
typedef struct _shader_uniforms_prog_cg {
	CGparameter video_size;
	CGparameter output_size;
	CGparameter texture_size;

	CGparameter frame_count;
	CGparameter frame_direction;

	CGparameter lut[MAX_PASS];

	CGparameter param[MAX_PARAM];
} _shader_uniforms_prog_cg;
typedef struct _shader_uniforms_tex_cg {
	struct _vsut {
		CGparameter video_size;
		CGparameter texture_size;
		CGparameter tex_coord;
	} v;
	struct _fsut {
		CGparameter texture;
		CGparameter video_size;
		CGparameter texture_size;
	} f;
}  _shader_uniforms_tex_cg;
typedef struct _shader_uniforms_cg {
	CGparameter mvp;

	CGparameter tex;
	CGparameter lut_tex;
	CGparameter color;
	CGparameter vertex;

	_shader_uniforms_prog_cg v;
	_shader_uniforms_prog_cg f;

	_shader_uniforms_tex_cg orig;
	_shader_uniforms_tex_cg passprev[MAX_PASS];
	_shader_uniforms_tex_cg prev[MAX_PREV];
	_shader_uniforms_tex_cg feedback;
} _shader_uniforms_cg;
#endif

typedef struct _math_matrix_4x4 {
	float data[16];
} _math_matrix_4x4;
typedef struct _vertex_buffer {
	// vertexes
	GLfloat x, y;
	// white_color
	GLfloat c0, c1, c2, c3;
	// tex coords
	GLfloat s0, t0;
	// lut tex coords
	GLfloat luttx[2];
	// orig tex coords
	GLfloat origtx[2];
	// feedback tex coords
	GLfloat feedtx[2];
	// passprev tex coords
	GLfloat pptx[MAX_PASS * 2];
} _vertex_buffer;
typedef struct _lut {
	GLuint id;
	int w, h;
	const unsigned char *bits;
	const char *name;
} _lut;
typedef struct _shader_uniforms_tex {
	int texture;
	int input_size;
	int texture_size;
	int tex_coord;
}  _shader_uniforms_tex;
typedef struct _shader_uniforms {
	int mvp;
	int tex_coord;
	int vertex_coord;
	int COLOR;
	int color;
	int lut_tex_coord;

	int input_size;
	int output_size;
	int texture_size;

	int frame_count;
	unsigned frame_count_mod;
	int frame_direction;

	int lut[MAX_PASS];

	int param[MAX_PARAM];

	_shader_uniforms_tex orig;
	_shader_uniforms_tex passprev[MAX_PASS];
	_shader_uniforms_tex prev[MAX_PREV];
	_shader_uniforms_tex feedback;
}  _shader_uniforms;
typedef struct _shader_info {
	GLfloat input_size[2];
	GLfloat output_size[2];
	GLfloat texture_size[2];
} _shader_info;
typedef struct _shader {
	GLuint type;
	GLuint vbo;

	struct _glslp {
		GLuint prg;
		_shader_uniforms uni;
	} glslp;

#if defined (WITH_OPENGL_CG)
	struct _cgp {
		_shader_prg_cg prg;
		_shader_uniforms_cg uni;
	} cgp;
#endif

	_vertex_buffer vb[4];
	_shader_info info;
} _shader;
typedef struct _texture_rect {
	GLint w;
	GLint h;
	_wh_uint base;
} _texture_rect;
typedef struct _texture {
	GLuint id;
	GLuint fbo;
	_texture_rect rect;
	_viewport vp;
	_shader shader;
} _texture;
typedef struct _texture_simple {
	GLuint id;
	_texture_rect rect;
	_shader shader;
} _texture_simple;
typedef struct _opengl {
	BYTE supported;
	GLfloat scale;

	_math_matrix_4x4 mvp;

	struct _attribsarray {
		GLuint count;
		GLuint attrib[MAX_PASS + MAX_PREV + 1 + 1 + 4 + 4];
	} attribs;

	struct _opengl_supported_fbo {
		BYTE flt;
		BYTE srgb;
	} supported_fbo;

	struct _opengl_sdl {
		SDL_Surface *surface;
		Uint32 flags;
	} sdl;

	struct _opengl_screen {
		GLint in_use;
		GLuint index;
		_texture_simple tex[MAX_PREV + 1];
	} screen;

	struct _feedback {
		GLint in_use;
		_texture tex;
	} feedback;

	_texture_simple text;
	_texture texture[MAX_PASS + 1];
	_lut lut[MAX_PASS];

#if defined (WITH_OPENGL_CG)
	struct _cg {
		CGcontext ctx;

		struct _clientstate {
			GLuint count;
			CGparameter state[MAX_PASS + MAX_PREV + 1 + 1 + 4];
		} states;

		struct _textureparameter {
			GLuint count;
			CGparameter param[MAX_PASS + MAX_PREV + 4];
		} params;

		struct _profile_cg {
			CGprofile v;
			CGprofile f;
		} profile;
	} cg;
#endif
} _opengl;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC _opengl opengl;

EXTERNC void opengl_init(void);
EXTERNC BYTE opengl_context_create(SDL_Surface *src);
EXTERNC void opengl_context_delete(void);

EXTERNC void opengl_set_shader(GLuint shader);
EXTERNC void opengl_draw_scene(SDL_Surface *surface);
EXTERNC void opengl_text_clear(_txt_element *ele);
EXTERNC void opengl_text_blit(_txt_element *ele, _rect *rect);
EXTERNC int opengl_flip(SDL_Surface *surface);

EXTERNC void opengl_screenshot(void);

#undef EXTERNC

#endif /* OPENGL_H_ */
