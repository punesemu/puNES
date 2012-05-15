/*
 * no_effect.c
 *
 *  Created on: 09/mag/2012
 *      Author: fhorse
 */

#include "no_effect.h"

void opengl_set_no_effect(SDL_Surface *src) {
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glViewport(0, 0, (GLint) src->w, (GLint) src->h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, src->w, src->h, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glDisable(GL_TEXTURE_2D);
}
void opengl_draw_scene_no_effect(SDL_Surface *surface) {
	/* ripulisco la scena opengl */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	opengl_update_texture(surface);

	if (opengl.glsl) {
		glUseProgram(shader.program);
	}

	/* disegno la texture */
	glBegin(GL_QUADS);
		/* Bottom Left Of The Texture */
		glTexCoord2f(0.0f, 0.0f);
		glVertex2i(opengl.xTexture1, opengl.yTexture1);
		/* Bottom Right Of The Texture */
		glTexCoord2f(opengl.texture.x, 0.0f);
		glVertex2i(opengl.xTexture2, opengl.yTexture1);
		/* Top Right Of The Texture */
		glTexCoord2f(opengl.texture.x, opengl.texture.y);
		glVertex2i(opengl.xTexture2, opengl.yTexture2);
		/* Top Left Of The Texture */
		glTexCoord2f(0.0f, opengl.texture.y);
		glVertex2i(opengl.xTexture1, opengl.yTexture2);
	glEnd();

	if (opengl.glsl) {
		glUseProgram(0);
	}

	glDisable(GL_TEXTURE_2D);
}
