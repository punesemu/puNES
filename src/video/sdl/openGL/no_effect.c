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

#include "no_effect.h"

INLINE void draw_primitive_no_effect(void);

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

	glEnable(GL_TEXTURE_2D);

	opengl_update_scr_texture(surface, FALSE);

	/* disegno la texture dello screen */
	draw_primitive_no_effect();

	if (opengl.glsl.shader_used) {
		glUseProgram(0);
	}

	if (opengl_update_txt_texture(FALSE) == EXIT_OK) {
		/* disegno la texture del testo */
		draw_primitive_no_effect();

		glDisable(GL_BLEND);
	}

	glDisable(GL_TEXTURE_2D);
}

INLINE void draw_primitive_no_effect(void) {
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
}
