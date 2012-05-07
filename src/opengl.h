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

void sdlInitGL(void);
void sdlCreateSurfaceGL(SDL_Surface *src, WORD width, WORD height, BYTE flags);
int sdlFlipScreenGL(SDL_Surface *surface);
int sdlPowerOfTwoGL(int base);

struct _opengl {
	BYTE aspectRatio;
	BYTE rotation;
	BYTE glsl_enabled;
	SDL_Surface *surfaceGL;
	GLuint texture;
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
} opengl;

#endif /* OPENGL_H_ */
