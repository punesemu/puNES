/*
 * no_effect.c
 *
 *  Created on: 09/mag/2012
 *      Author: fhorse
 */

#include "no_effect.h"

void opengl_init_no_effect(void) {
	return;
}
void opengl_set_no_effect(SDL_Surface *src) {
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glViewport(0, 0, (GLint) src->w, (GLint) src->h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, src->w, src->h, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_POLYGON_SMOOTH);
	glDisable(GL_STENCIL_TEST);

	glEnable(GL_DITHER);

	glDepthMask(GL_FALSE);

	glDisable(GL_TEXTURE_2D);
}
void opengl_unset_no_effect(void) {
	return;
}
void opengl_draw_scene_no_effect(SDL_Surface *surface) {
	/* ripulisco la scena opengl */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	opengl_update_texture(surface, FALSE);

	opengl_enable_texture();

	/* disegno la texture */
	glBegin(GL_QUADS);
		/* Bottom Left Of The Texture */
		glTexCoord2f(opengl.texcoords.l, opengl.texcoords.b);
		glVertex2f(opengl.quadcoords.l, opengl.quadcoords.b);
		/* Bottom Right Of The Texture */
		glTexCoord2f(opengl.texcoords.r, opengl.texcoords.b);
		glVertex2f(opengl.quadcoords.r, opengl.quadcoords.b);
		/* Top Right Of The Texture */
		glTexCoord2f(opengl.texcoords.r, opengl.texcoords.t);
		glVertex2f(opengl.quadcoords.r, opengl.quadcoords.t);
		/* Top Left Of The Texture */
		glTexCoord2f(opengl.texcoords.l, opengl.texcoords.t);
		glVertex2f(opengl.quadcoords.l, opengl.quadcoords.t);
	glEnd();

	if (opengl.glsl.shader_used) {
		glUseProgram(0);
	}

	glDisable(GL_TEXTURE_2D);
}
