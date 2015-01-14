/*
 * opengl.h
 *
 *  Created on: 23/lug/2011
 *      Author: fhorse
 */

#ifndef OPENGL_H_
#define OPENGL_H_

#include "glew/glew.h"
#include <SDL.h>
#include "common.h"
#include "gfx.h"
#include "shaders.h"

#define slow_factor	3

enum power_of_two_switch { NO_POWER_OF_TWO, POWER_OF_TWO };

typedef struct {
	GLfloat l, r;
	GLfloat t, b;
} _texcoords;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC struct _opengl {
	BYTE rotation;

	BYTE supported;
	BYTE glew;

	struct {
		BYTE compliant;
		BYTE enabled;
		BYTE shader_used;
		BYTE param;
	} glsl;

	GLint scale_force;
	GLfloat scale;
	GLfloat factor;
	BYTE interpolation;
	BYTE PSS;

	SDL_Surface *surface_gl;

	_texture screen;
	_texture text;

	_texcoords texcoords;
	_texcoords quadcoords;

	Uint32 flags;
	BYTE factor_distance;

	float x_rotate;
	float y_rotate;
	float x_diff;
	float y_diff;
} opengl;

EXTERNC void sdl_init_gl(void);
EXTERNC void sdl_quit_gl(void);
EXTERNC void sdl_create_surface_gl(SDL_Surface *src, WORD width, WORD height, BYTE flags);

EXTERNC void opengl_create_texture(_texture *texture, uint32_t width, uint32_t height, uint8_t pow);
EXTERNC void opengl_update_scr_texture(SDL_Surface *surface, uint8_t generate_mipmap);
EXTERNC BYTE opengl_update_txt_texture(uint8_t generate_mipmap);

EXTERNC void opengl_effect_change(BYTE mode);

EXTERNC void opengl_text_clear(_txt_element *ele);
EXTERNC void opengl_text_blit(_txt_element *ele, _rect *rect);

EXTERNC int opengl_flip(SDL_Surface *surface);
EXTERNC int opengl_power_of_two(int base);

EXTERNC void glew_init(void);

EXTERNC void glsl_shaders_init(_shader *shd);
EXTERNC void glsl_delete_shaders(_shader *shd);

/* funzioni virtuali */
EXTERNC void (*opengl_init_effect)(void);
EXTERNC void (*opengl_set_effect)(SDL_Surface *src);
EXTERNC void (*opengl_unset_effect)(void);
EXTERNC void (*opengl_draw_scene)(SDL_Surface *surface);

#undef EXTERNC

#endif /* OPENGL_H_ */
