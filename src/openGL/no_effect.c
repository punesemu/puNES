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

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, shader.texture_text);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, surfaceSDL->w);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, opengl.surface_text->w,
			opengl.surface_text->h, opengl.texture.format,
	        opengl.texture.type, opengl.surface_text->pixels);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glBindTexture(GL_TEXTURE_2D, opengl.texture.data);

	opengl_update_texture(surface);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, shader.texture_text);

	glEnable(GL_TEXTURE_2D);

	glUseProgram(shader.program);

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

	glUseProgram(0);

	glDisable(GL_TEXTURE_2D);
}
