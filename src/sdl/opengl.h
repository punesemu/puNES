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

struct _opengl {
	BYTE rotation;

	BYTE supported;
	BYTE glew;

	struct {
		BYTE compliant;
		BYTE enabled;
		BYTE shader_used;
	} glsl;

	GLint scale_force;
	GLfloat scale;
	GLfloat factor;
	BYTE interpolation;

	SDL_Surface *surface_gl;

	GLint w_texture;
	GLint h_texture;
	GLint x_texture1;
	GLint y_texture1;
	GLint x_texture2;
	GLint y_texture2;

	Uint32 flags;
	BYTE factor_distance;

	float x_rotate;
	float y_rotate;
	float x_diff;
	float y_diff;

	_texture texture;
} opengl;

void sdl_init_gl(void);
void sdl_quit_gl(void);
void sdl_create_surface_gl(SDL_Surface *src, WORD width, WORD height, BYTE flags);

void opengl_enable_texture(void);
void opengl_create_texture(_texture *texture, uint32_t width, uint32_t height,
        uint8_t interpolation, uint8_t pow);
void opengl_update_texture(SDL_Surface *surface, uint8_t generate_mipmap);

void opengl_text_clear(_txt_element *ele);
void opengl_text_blit(_txt_element *ele, SDL_Rect *dst_rect);

int opengl_flip(SDL_Surface *surface);
int opengl_power_of_two(int base);

void glew_init(void);

void glsl_shaders_init(_shader *shd);
void glsl_delete_shaders(_shader *shd);

/* funzioni virtuali */
void (*opengl_init_effect)(void);
void (*opengl_set_effect)(SDL_Surface *src);
void (*opengl_unset_effect)(void);
void (*opengl_draw_scene)(SDL_Surface *surface);

#endif /* OPENGL_H_ */
