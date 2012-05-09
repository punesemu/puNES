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

#define slowFactor	3

typedef struct {
	GLuint data;
	GLenum format;
	GLenum type;
	GLint format_internal;

	GLfloat x;
	GLfloat y;
} _texture;
typedef struct {

} _vertex;
struct _opengl {
	BYTE aspectRatio;
	BYTE rotation;
	BYTE glsl_enabled;
	SDL_Surface *surfaceGL;

	GLint wTexture;
	GLint hTexture;
	GLint xTexture1;
	GLint yTexture1;
	GLint xTexture2;
	GLint yTexture2;
	Uint32 flagsOpengl;
	BYTE factorDistance;
	BYTE glew;
	float xRotate;
	float yRotate;
	float xDiff;
	float yDiff;

	_texture texture;

} opengl;

void sdlInitGL(void);
void sdlCreateSurfaceGL(SDL_Surface *src, WORD width, WORD height, BYTE flags);
int opengl_flip(SDL_Surface *surface);

void opengl_create_texture(GLuint *texture, GLint texture_real_width, GLint texture_real_height);
void opengl_update_texture(SDL_Surface *surface);
int opengl_power_of_two(int base);

/* funzioni virtuali */
void (*opengl_set)(SDL_Surface *src);
void (*opengl_draw_scene)(SDL_Surface *surface);

#endif /* OPENGL_H_ */
