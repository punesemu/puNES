/*
 * cube3d.c
 *
 *  Created on: 09/mag/2012
 *      Author: fhorse
 */

#include "cube3d.h"

GLfloat xVertex, yVertex, zVertex;
GLfloat distance;
GLfloat matrixDistance[60] = {
	-2.000f, -2.020f, -2.040f, -2.060f, -2.080f,
	-2.100f, -2.120f, -2.140f, -2.160f, -2.180f,
	-2.200f, -2.220f, -2.240f, -2.260f, -2.280f,
	-2.300f, -2.320f, -2.340f, -2.360f, -2.380f,
	-2.400f, -2.420f, -2.440f, -2.460f, -2.480f,
	-2.500f, -2.520f, -2.540f, -2.560f, -2.580f,
	-2.600f, -2.620f, -2.640f, -2.660f, -2.680f,
	-2.700f, -2.720f, -2.740f, -2.760f, -2.780f,
	-2.800f, -2.820f, -2.840f, -2.860f, -2.880f,
	-2.890f, -2.900f, -2.910f, -2.920f, -2.930f,
	-2.940f, -2.950f, -2.965f, -2.970f, -2.975f,
	-2.980f, -2.985f, -2.990f, -2.995f, -3.000f
};
_shader color;

void opengl_init_cube3d(void) {
	memset (&color, 0, sizeof(_shader));
}
void opengl_set_cube3d(SDL_Surface *src) {
	xVertex = 1.0f - ((1.0f / (src->w / 2)) * opengl.x_texture1);
	yVertex = 1.0f - ((1.0f / (src->h / 2)) * opengl.y_texture1);
	zVertex = xVertex;

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glViewport(0, 0, (GLint) src->w, (GLint) src->h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glFrustum(-1, 1, -1, 1, 1.0f + (1.0f - zVertex), 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glDisable(GL_TEXTURE_2D);

	if (opengl.glsl.shader_used) {
		color.id = SHADER_COLOR;
		glsl_shaders_init(&color);
	}
}
void opengl_unset_cube3d(void) {
	glsl_delete_shaders(&color);
}
void opengl_draw_scene_cube3d(SDL_Surface *surface) {
	/* ripulisco la scena opengl */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	opengl_update_texture(surface, TRUE);

	/* resetto la matrice corrente (modelview) */
	glLoadIdentity();

	if (opengl.factor_distance < 60) {
		distance = matrixDistance[opengl.factor_distance];
		opengl.factor_distance++;
	}
	glTranslatef(0.0f, 0.0f, distance);

	if (opengl.factor_distance > 30) {
		glRotatef(opengl.x_rotate, 1.0f, 0.0f, 0.0f);
		glRotatef(opengl.y_rotate, 0.0f, 1.0f, 0.0f);
	}

	opengl_enable_texture();

	/* cubo esterno */
	glBegin(GL_QUADS);
		/* avanti */
		glColor3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.0f, opengl.texture.y);
		glVertex3f(-xVertex, -yVertex, +zVertex);
		glTexCoord2f(opengl.texture.x, opengl.texture.y);
		glVertex3f(+xVertex, -yVertex, +zVertex);
		glTexCoord2f(opengl.texture.x, 0.0f);
		glVertex3f(+xVertex, +yVertex, +zVertex);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-xVertex, +yVertex, +zVertex);
		/* dietro */
		glColor3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, opengl.texture.y);
		glVertex3f(+xVertex, -yVertex, -zVertex);
		glTexCoord2f(opengl.texture.x, opengl.texture.y);
		glVertex3f(-xVertex, -yVertex, -zVertex);
		glTexCoord2f(opengl.texture.x, 0.0f);
		glVertex3f(-xVertex, +yVertex, -zVertex);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(+xVertex, +yVertex, -zVertex);
		/* sopra */
		/*
		glTexCoord2f(0.0f, opengl.texture.y);
		glVertex3f(-xVertex, +yVertex, +zVertex);
		glTexCoord2f(opengl.texture.x, opengl.texture.y);
		glVertex3f(+xVertex, +yVertex, +zVertex);
		glTexCoord2f(opengl.texture.x, 0.0f);
		glVertex3f(+xVertex, +yVertex, -zVertex);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-xVertex, +yVertex, -zVertex);
		*/
		/* sotto */
		/*
		glTexCoord2f(0.0f, opengl.texture.y);
		glVertex3f(+xVertex, -yVertex, +zVertex);
		glTexCoord2f(opengl.texture.x, opengl.texture.y);
		glVertex3f(-xVertex, -yVertex, +zVertex);
		glTexCoord2f(opengl.texture.x, 0.0f);
		glVertex3f(-xVertex, -yVertex, -zVertex);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(+xVertex, -yVertex, -zVertex);
		*/
		/* destra */
		glColor3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, opengl.texture.y);
		glVertex3f(+xVertex, -yVertex, +zVertex);
		glTexCoord2f(opengl.texture.x, opengl.texture.y);
		glVertex3f(+xVertex, -yVertex, -zVertex);
		glTexCoord2f(opengl.texture.x, 0.0f);
		glVertex3f(+xVertex, +yVertex, -zVertex);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(+xVertex, +yVertex, +zVertex);
		/* sinistra */
		glColor3f(0.0f, 0.0f, 1.0f);
		glTexCoord2f(0.0f, opengl.texture.y);
		glVertex3f(-xVertex, -yVertex, -zVertex);
		glTexCoord2f(opengl.texture.x, opengl.texture.y);
		glVertex3f(-xVertex, -yVertex, +zVertex);
		glTexCoord2f(opengl.texture.x, 0.0f);
		glVertex3f(-xVertex, +yVertex, +zVertex);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-xVertex, +yVertex, -zVertex);
	glEnd();

	if (opengl.glsl.shader_used) {
		glUseProgram(color.prg);
	}

	glDisable(GL_TEXTURE_2D);

	/* cubo interno */
	glBegin(GL_QUADS);
		/* avanti */
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(+xVertex, -yVertex, +zVertex);
		glVertex3f(-xVertex, -yVertex, +zVertex);
		glVertex3f(-xVertex, +yVertex, +zVertex);
		glVertex3f(+xVertex, +yVertex, +zVertex);
		/* dietro */
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(-xVertex, -yVertex, -zVertex);
		glVertex3f(+xVertex, -yVertex, -zVertex);
		glVertex3f(+xVertex, +yVertex, -zVertex);
		glVertex3f(-xVertex, +yVertex, -zVertex);
		/* destra */
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(+xVertex, -yVertex, -zVertex);
		glVertex3f(+xVertex, -yVertex, +zVertex);
		glVertex3f(+xVertex, +yVertex, +zVertex);
		glVertex3f(+xVertex, +yVertex, -zVertex);
		/* sinistra */
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(-xVertex, -yVertex, +zVertex);
		glVertex3f(-xVertex, -yVertex, -zVertex);
		glVertex3f(-xVertex, +yVertex, -zVertex);
		glVertex3f(-xVertex, +yVertex, +zVertex);
	glEnd();

	if (opengl.glsl.shader_used) {
		glUseProgram(0);
	}
}
