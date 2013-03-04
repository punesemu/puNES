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
		glTexCoord2f(0.0f, 0.0f);
		glVertex2i(opengl.x_texture1, opengl.y_texture1);
		/* Bottom Right Of The Texture */
		glTexCoord2f(opengl.texture.x, 0.0f);
		glVertex2i(opengl.x_texture2, opengl.y_texture1);
		/* Top Right Of The Texture */
		glTexCoord2f(opengl.texture.x, opengl.texture.y);
		glVertex2i(opengl.x_texture2, opengl.y_texture2);
		/* Top Left Of The Texture */
		glTexCoord2f(0.0f, opengl.texture.y);
		glVertex2i(opengl.x_texture1, opengl.y_texture2);
	glEnd();

	if (opengl.glsl.shader_used) {
		glUseProgram(0);
	}

	glDisable(GL_TEXTURE_2D);
}
