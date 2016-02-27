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

#include "common.h"
#include "opengl.h"
#include "ppu.h"
#include "conf.h"

#include "qt.h"
#include "cgp.h"

#define FHOPENGLGEST

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

enum _texture_info {
	TI_INTFRM = GL_RGBA8,
	TI_FRM = GL_BGRA,
	TI_TYPE = GL_UNSIGNED_BYTE,
	TI_F_INTFRM = GL_RGBA32F,
	TI_F_TYPE = GL_FLOAT,
	TI_S_INTFRM = GL_SRGB8_ALPHA8,
	TI_S_TYPE = GL_UNSIGNED_BYTE
};

static BYTE glew_init(void);

static void shd_init(_shader *shd, const GLchar *code);
static void shd_delete(_shader *shd);
static void shd_print_log(GLuint obj, BYTE ret);
char *shd_file2string(const char *path);
static void shd_uni_texture(_shader_uniforms_tex *uni, GLint prg, GLchar *fmt, ...);
static GLint shd_get_uni(GLuint prog, const char *param);
static GLint shd_get_atr(GLuint prog, const char *param);
static void shd_set_vertex_buffer(_vertex_buffer *vb, _texture_rect *rect);
INLINE static void shd_set_params_text(_shader *shd);
INLINE static void shd_set_params(const _shader *shd, GLuint fcountmod, GLuint fcount);

static BYTE tex_create(_texture *texture, GLuint index, GLuint clean);
static void tex_create_simple(_texture_simple *texture, GLuint w, GLuint h, BYTE fixed);
static void tex_create_lut(_lut *lut, GLuint index);
static void tex_delete_screen(void);
static void tex_delete_text(void);

static GLint power_of_two(GLint base);
static const GLint get_integer(const GLenum penum);

static const _vertex_buffer vb_upright[4] = {
  { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
  { 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
  { 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
  { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
};
static const _vertex_buffer vb_flipped[4] = {
  { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
  { 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
  { 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
  { 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
};
static const GLchar *uni_prefixes[] = { "", "ruby", };

void sdl_init_gl(void) {
	opengl.sdl.surface = NULL;
	opengl.sdl.flags = SDL_HWSURFACE | SDL_OPENGL;
	opengl.vp = NULL;

	memset(&opengl.alias_define, 0x00, sizeof(opengl.alias_define));
	memset(&opengl.screen, 0x00, sizeof(opengl.screen));
	memset(&opengl.feedback, 0x00, sizeof(opengl.feedback));
	memset(&opengl.text, 0x00, sizeof(_texture_simple));
	memset(&opengl.texture, 0x00, LENGTH(opengl.texture) * sizeof(_texture));
	memset(&opengl.lut, 0x00, LENGTH(opengl.lut) * sizeof(_lut));

	if (glew_init() == EXIT_ERROR) {
		opengl.supported = FALSE;
		return;
	}

	// Calculate projection
	matrix_4x4_ortho(&opengl.mvp, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
}
void sdl_quit_gl(void) {
	tex_delete_screen();
	tex_delete_text();
}
BYTE sdl_setup_renderingl_gl(SDL_Surface *src) {
	GLuint i, w, h;

	glGetError();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DITHER);
	glDisable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);

	tex_delete_screen();

	if ((cfg->filter == NO_FILTER) || (cfg->filter >= FLTSHDSTART)) {
		w = gfx.rows;
		h = gfx.lines;
	} else {
		w = gfx.w[CURRENT];
		h = gfx.h[CURRENT];
	}

	opengl.sdl.surface = gfx_create_RGB_surface(src, w, h);

	// screen
	tex_create_simple(&opengl.screen.tex[0], w, h, TRUE);

	// creo le restanti texture/fbo
	for (i = 0; i < shader_effect.pass; i++) {
		if (tex_create(&opengl.texture[i], i, FALSE) == EXIT_ERROR) {
			sdl_quit_gl();
			return (EXIT_ERROR);
		}
	}

	// non inizializzo le shader nel tex_create perche' opengl.alias_define
	// viene costruita proprio durante il tex_create e a me serve completa quando
	// inizializzo le shaders.
	for (i = 0; i < shader_effect.pass; i++) {
		shd_init(&opengl.texture[i].shader, shader_effect.sp[i].code);
	}

	// PREV (calcolo il numero di screen da utilizzare)
	// deve essere fatto dopo il shd_init().
	for (i = 0; i < shader_effect.pass; i++) {
		GLuint a;

		for (a = 0; a < LENGTH(opengl.texture[i].shader.uni.prev); a++) {
			if (opengl.texture[i].shader.uni.prev[a].texture >= 0) {
				if (opengl.screen.in_use < (a + 1)) {
					opengl.screen.in_use = (a + 1);
				}
			}
		}
	}

	opengl.screen.in_use++;

	// PREV
	for (i = 1; i < opengl.screen.in_use; i++) {
		tex_create_simple(&opengl.screen.tex[i], w, h, TRUE);
	}

	// il vieport generale e' quello dell'ultimo pass
	opengl.vp = &opengl.texture[shader_effect.last_pass].vp;

	// configuro l'aspect ratio del fullscreen
	if (cfg->fullscreen && !cfg->stretch) {
		GLfloat ratio_surface = ((GLfloat) gfx.rows / (GLfloat) gfx.lines) * gfx.pixel_aspect_ratio;
		GLfloat ratio_frame = (GLfloat) src->w / (GLfloat) src->h;
		GLfloat delta;

		if (ratio_frame > ratio_surface) {
			delta = (ratio_surface / ratio_frame - 1.0f) / 2.0f + 0.5f;
			opengl.vp->x = (int) roundf((GLfloat) src->w * (0.5f - delta));
			opengl.vp->w = (unsigned) roundf(2.0f * (GLfloat) src->w * delta);
		} else {
			delta = (ratio_frame / ratio_surface - 1.0f) / 2.0f + 0.5f;
			opengl.vp->y = (int) roundf((GLfloat) src->h * (0.5f - delta));
			opengl.vp->h = (unsigned) roundf(2.0f * (GLfloat) src->h * delta);
		}
	}

	// FEEDBACK
	if ((shader_effect.feedback_pass >= 0) && (shader_effect.feedback_pass < shader_effect.pass)) {
		opengl.feedback.in_use = TRUE;

		if (tex_create(&opengl.feedback.tex, shader_effect.feedback_pass, TRUE) == EXIT_ERROR) {
			sdl_quit_gl();
			return (EXIT_ERROR);
		}
	}

	// testo
	{
		_shader *sh = &opengl.text.shader;

		tex_delete_text();

		tex_create_simple(&opengl.text, opengl.vp->w, opengl.vp->h, FALSE);

		glGenBuffers(1, &sh->vbo);
		memcpy(sh->vb, vb_flipped, sizeof(vb_flipped));

		glBindBuffer(GL_ARRAY_BUFFER, sh->vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(sh->vb), sh->vb, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		shd_init(sh, shader_code_blend());
	}

	for (i = 0; i < shader_effect.luts; i++) {
		tex_create_lut(&opengl.lut[i], i);
	}

	// setto tutto quello che mi serve per il rendering
	for (i = 0; i < shader_effect.pass; i++) {
		_texture *tex = &opengl.texture[i];
		_shader *shd = &tex->shader;
		_texture_rect *prev = NULL;
		GLuint a, b;

		if (i == 0) {
			prev = &opengl.screen.tex[0].rect;
		} else {
			prev = &opengl.texture[i - 1].rect;
		}

		shd->info.input_size[0] = (GLfloat) prev->base.w;
		shd->info.input_size[1] = (GLfloat) prev->base.h;
		shd->info.texture_size[0] = (GLfloat) prev->w,
		shd->info.texture_size[1] = (GLfloat) prev->h;
		shd->info.output_size[0] = (GLfloat) tex->vp.w;
		shd->info.output_size[1] = (GLfloat) tex->vp.h;

		shd_set_vertex_buffer(&shd->vb[0], prev);

		for (a = 0; a < LENGTH(shd->vb); a++) {
			// ORIG e PREV texture coord
			shd->vb[a].origtx[0] = opengl.screen.tex[0].shader.vb[a].s0;
			shd->vb[a].origtx[1] = opengl.screen.tex[0].shader.vb[a].t0;

			// FEEDBACK
			if (opengl.feedback.in_use) {
				shd->vb[a].feedtx[0] = opengl.texture[shader_effect.feedback_pass].shader.vb[a].s0;
				shd->vb[a].feedtx[1] = opengl.texture[shader_effect.feedback_pass].shader.vb[a].t0;
			}

			// LUT texture coord
			shd->vb[a].luttx[0] = vb_upright[a].s0;
			shd->vb[a].luttx[1] = vb_upright[a].t0;
		}

		// PASSPREV texture coord
		for (a = 0; a < i; a++) {
			for (b = 0; b < LENGTH(shd->vb); b++) {
				shd->vb[b].pptx[(a * 2) + 0] = opengl.texture[a + 1].shader.vb[b].s0;
				shd->vb[b].pptx[(a * 2) + 1] = opengl.texture[a + 1].shader.vb[b].t0;
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, shd->vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(shd->vb), shd->vb, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glFinish();

 	return (EXIT_OK);
}

void opengl_draw_scene(SDL_Surface *surface) {
	GLuint i;

	// screen
	glBindTexture(GL_TEXTURE_2D, opengl.screen.tex[opengl.screen.index].id);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->w);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, surface->w, surface->h, TI_FRM, TI_TYPE,
			surface->pixels);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	if (opengl.supported_fbo.srgb) {
		glEnable(GL_FRAMEBUFFER_SRGB);
	}

	// fbo e pass
	for (i = 0; i < shader_effect.pass; i++) {
		const _texture *t = &opengl.texture[i];
		const _shader_pass *sp = &shader_effect.sp[i];
		GLuint id, fbo = t->fbo;

		shader_effect.running_pass = i;

		if (i == shader_effect.last_pass) {
			fbo = 0;
			if (opengl.supported_fbo.srgb) {
				glDisable(GL_FRAMEBUFFER_SRGB);
			}
		}

		if (i == 0) {
			id = opengl.screen.tex[opengl.screen.index].id;
		} else {
			id = opengl.texture[i - 1].id;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(t->vp.x, t->vp.y, t->vp.w, t->vp.h);
		glUseProgram(t->shader.prg);
		shd_set_params(&t->shader, sp->frame_count_mod, ppu.frames);
		glBindTexture(GL_TEXTURE_2D, id);
		if (sp->mipmap_input) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	opengl.screen.index = ((opengl.screen.index + 1) % opengl.screen.in_use);

	if (opengl.feedback.in_use) {
		GLuint fbo = opengl.feedback.tex.fbo;
		GLuint tex = opengl.feedback.tex.id;

		opengl.feedback.tex.fbo = opengl.texture[shader_effect.feedback_pass].fbo;
		opengl.feedback.tex.id = opengl.texture[shader_effect.feedback_pass].id;
		opengl.texture[shader_effect.feedback_pass].fbo = fbo;
		opengl.texture[shader_effect.feedback_pass].id = tex;
	}

	// testo
	if (!cfg->txt_on_screen || !text.on_screen) {
		return;
	}

	glViewport(0, 0, opengl.text.rect.w, opengl.text.rect.h);
	glUseProgram(opengl.text.shader.prg);
	glBindTexture(GL_TEXTURE_2D, opengl.text.id);
	shd_set_params_text(&opengl.text.shader);
	glEnable(GL_BLEND);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisable(GL_BLEND);
}
void opengl_text_clear(_txt_element *ele) {
	if (!ele->blank) {
		return;
	}

	glBindTexture(GL_TEXTURE_2D, opengl.text.id);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, ele->w);
	glTexSubImage2D(GL_TEXTURE_2D, 0, ele->x, ele->y, ele->w, ele->h,
			TI_FRM, TI_TYPE, ele->blank->pixels);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}
void opengl_text_blit(_txt_element *ele, _rect *rect) {
	if (!cfg->txt_on_screen) {
		return;
	}

	glBindTexture(GL_TEXTURE_2D, opengl.text.id);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, rect->w);
	glTexSubImage2D(GL_TEXTURE_2D, 0, rect->x, rect->y, rect->w, rect->h,
			TI_FRM, TI_TYPE, ele->surface->pixels);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}
int opengl_flip(SDL_Surface *surface) {
	SDL_GL_SwapBuffers();

	return (0);
}

static BYTE glew_init(void) {
	if (opengl.supported){
		GLenum err;

		glewExperimental = GL_TRUE;

		if ((err = glewInit()) != GLEW_OK) {
			fprintf(stderr, "INFO: %s\n", glewGetErrorString(err));
		} else {
			fprintf(stderr, "INFO: GPU %s (%s, %s)\n", glGetString(GL_RENDERER),
					glGetString(GL_VENDOR),
					glGetString(GL_VERSION));
			fprintf(stderr, "INFO: GL Version %d.%d %s\n",
					get_integer(GL_MAJOR_VERSION), get_integer(GL_MINOR_VERSION),
					get_integer(GL_CONTEXT_CORE_PROFILE_BIT) ? "Core" : "Compatibility");

			if (!GLEW_VERSION_3_0) {
				fprintf(stderr, "INFO: OpenGL 3.0 not supported. Disabled.\n");
				return (EXIT_ERROR);
			}

			if (!GLEW_ARB_framebuffer_object) {
				return (EXIT_ERROR);
			}

			if (!(glGenFramebuffers && glBindFramebuffer && glFramebufferTexture2D &&
					glCheckFramebufferStatus && glDeleteFramebuffers)) {
				return (EXIT_ERROR);
			}

			opengl.supported_fbo.flt = GLEW_ARB_texture_float;
			opengl.supported_fbo.srgb = (GLEW_EXT_texture_sRGB && GLEW_ARB_framebuffer_sRGB);

			return (EXIT_OK);
		}
	}

	return (EXIT_ERROR);
}

static void shd_init(_shader *shd, const GLchar *code) {
	const GLchar *src[3];
	GLuint i, vrt, frg;

	if (code == NULL) {
		return;
	}

	// program
	shd->prg = glCreateProgram();

	src[1] = opengl.alias_define;
	src[2] = code;

	// vertex
	src[0] = "#define VERTEX\n#define PARAMETER_UNIFORM\n";
	vrt = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vrt, 3, src, NULL);
	glCompileShader(vrt);
#if !defined (RELEASE)
	shd_print_log(vrt, FALSE);
#endif
	glAttachShader(shd->prg, vrt);
	glDeleteShader(vrt);

	// fragment
	src[0] = "#define FRAGMENT\n#define PARAMETER_UNIFORM\n";
	frg = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frg, 3, src, NULL);
	glCompileShader(frg);
#if !defined (RELEASE)
	shd_print_log(frg, FALSE);
#endif
	glAttachShader(shd->prg, frg);
	glDeleteShader(frg);

	glLinkProgram(shd->prg);
#if !defined (RELEASE)
	shd_print_log(shd->prg, TRUE);
#endif

	glUseProgram(shd->prg);

	glUniform1i(shd_get_uni(shd->prg, "Texture"), 0);

	shd->uni.mvp = shd_get_uni(shd->prg, "MVPMatrix");
	shd->uni.tex_coord = shd_get_atr(shd->prg, "TexCoord");
	shd->uni.vertex_coord = shd_get_atr(shd->prg, "VertexCoord");
	{
		// alcuni driver fanno distinzione tra COLOR e Color
		// se lascio solo Color con questi driver l'applicazione crasha
		// mentre su altri driver se uso solo COLOR la shader non funziona
		// quindi utilizzo entrambi.
		shd->uni.COLOR = shd_get_atr(shd->prg, "COLOR");
		shd->uni.color = shd_get_atr(shd->prg, "Color");
	}

	shd->uni.input_size = shd_get_uni(shd->prg, "InputSize");
	shd->uni.output_size = shd_get_uni(shd->prg, "OutputSize");
	shd->uni.texture_size = shd_get_uni(shd->prg, "TextureSize");

	shd->uni.frame_count = shd_get_uni(shd->prg, "FrameCount");
	shd->uni.frame_direction = shd_get_uni(shd->prg, "FrameDirection");

	for (i = 0; i < shader_effect.params; i++) {
		shd->uni.param[i] = shd_get_uni(shd->prg, shader_effect.param[i].name);
	}

	shd_uni_texture(&shd->uni.orig, shd->prg, "Orig");
	shd_uni_texture(&shd->uni.feedback, shd->prg, "Feedback");

	for (i = 0; i < shader_effect.pass; i++) {
		shd_uni_texture(&shd->uni.passprev[i], shd->prg, "Pass%u", i + 1);
		shd_uni_texture(&shd->uni.passprev[i], shd->prg, "PassPrev%u", i + 1);

		if (shader_effect.sp[i].alias[0]) {
			shd_uni_texture(&shd->uni.passprev[i], shd->prg, shader_effect.sp[i].alias);
		}
	}

	shd_uni_texture(&shd->uni.prev[0], shd->prg, "Prev");

	for (i = 1; i < LENGTH(shd->uni.prev); i++) {
		shd_uni_texture(&shd->uni.prev[i], shd->prg, "Prev%u", i);
	}

	for (i = 0; i < shader_effect.luts; i++) {
		shd->uni.lut[i] = shd_get_uni(shd->prg, shader_effect.lp[i].name);
	}

	shd->uni.lut_tex_coord = shd_get_atr(shd->prg, "LUTTexCoord");

	glUseProgram(0);
}
static void shd_delete(_shader *shd) {
	// program
	if (shd->prg) {
		glDeleteProgram(shd->prg);
		shd->prg = 0;
	}
}
static void shd_print_log(GLuint obj, BYTE ret) {
	GLint info_log_length = 0, max_length = 0;

	if (glIsShader(obj)) {
		glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &max_length);
	} else {
		glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &max_length);
	}

	if (max_length == 0) {
		return;
	}

	{
		char info_log[max_length];

		if (glIsShader(obj)) {
			glGetShaderInfoLog(obj, max_length, &info_log_length, info_log);
		} else {
			glGetProgramInfoLog(obj, max_length, &info_log_length, info_log);
		}

		info_log[info_log_length] = 0;

		if (info_log_length > 0) {
			printf("INFO: %s", info_log);
			if (ret == TRUE) {
				printf("\n");
			}
		}
	}
}
char *shd_file2string(const char *path) {
	FILE *fd;
	long len, r;
	char *str;

	if (!(fd = fopen(path, "r"))) {
		fprintf(stderr, "Can't open file '%s' for reading\n", path);
		return NULL;
	}

	fseek(fd, 0, SEEK_END);
	len = ftell(fd);

	printf("File '%s' is %ld long\n", path, len);

	fseek(fd, 0, SEEK_SET);

	if (!(str = (char *) malloc(len * sizeof(char)))) {
		fprintf(stderr, "Can't malloc space for '%s'\n", path);
		return NULL;
	}

	r = fread(str, sizeof(char), len, fd);

	str[r - 1] = '\0'; /* Shader sources have to term with null */

	fclose(fd);

	return str;
}
static void shd_uni_texture(_shader_uniforms_tex *sut, GLint prg, GLchar *fmt, ...) {
	char type[50], buff[50];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(type, sizeof(type), fmt, ap);
	va_end(ap);

	snprintf(buff, sizeof(buff), "%s%s", type, "Texture");
	sut->texture = shd_get_uni(prg, buff);
	snprintf(buff, sizeof(buff), "%s%s", type, "TextureSize");
	sut->texture_size = shd_get_uni(prg, buff);
	snprintf(buff, sizeof(buff), "%s%s", type, "InputSize");
	sut->input_size = shd_get_uni(prg, buff);
	snprintf(buff, sizeof(buff), "%s%s", type, "TexCoord");
	sut->tex_coord = shd_get_atr(prg, buff);
}
static GLint shd_get_uni(GLuint prog, const char *param) {
	GLuint i;
	GLint loc;
	char buff[50];

	for (i = 0; i < LENGTH(uni_prefixes); i++) {
		snprintf(buff, sizeof(buff), "%s%s", uni_prefixes[i], param);
		loc = glGetUniformLocation(prog, buff);
		if (loc >= 0) {
			return (loc);
		}
	}

	return (-1);
}
static GLint shd_get_atr(GLuint prog, const char *param) {
	GLuint i;
	GLint loc;
	char buff[50];

	for (i = 0; i < LENGTH(uni_prefixes); i++) {
		snprintf(buff, sizeof(buff), "%s%s", uni_prefixes[i], param);
		loc = glGetAttribLocation(prog, buff);
		if (loc >= 0) {
			return (loc);
		}
	}

	return (-1);
}
static void shd_set_vertex_buffer(_vertex_buffer *vb, _texture_rect *rect) {
	GLfloat x = (GLfloat) rect->base.w / (GLfloat) rect->w;
	GLfloat y = (GLfloat) rect->base.h / (GLfloat) rect->h;

	vb[1].s0 = x; vb[2].t0 = y;
	vb[3].s0 = x; vb[3].t0 = y;

}
INLINE static void shd_set_params_text(_shader *shd) {
	GLuint buffer_index = 0;

	if (shd->uni.mvp >= 0) {
		glUniformMatrix4fv(shd->uni.mvp, 1, GL_FALSE, opengl.mvp.data);
	}

	glBindBuffer(GL_ARRAY_BUFFER, shd->vbo);

	if (shd->uni.tex_coord >= 0) {
		glEnableVertexAttribArray(shd->uni.tex_coord);
		glVertexAttribPointer(shd->uni.tex_coord, 2, GL_FLOAT, GL_FALSE, sizeof(_vertex_buffer),
				BUFFER_OFFSET(buffer_index));
	}
	buffer_index += 2;

	if (shd->uni.vertex_coord >= 0) {
		glEnableVertexAttribArray(shd->uni.vertex_coord);
		glVertexAttribPointer(shd->uni.vertex_coord, 2, GL_FLOAT, GL_FALSE, sizeof(_vertex_buffer),
		        BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));

	}
	buffer_index += 2;

	{
		if (shd->uni.COLOR >= 0) {
			glEnableVertexAttribArray(shd->uni.COLOR);
			glVertexAttribPointer(shd->uni.COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(_vertex_buffer),
					BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
		}
		if (shd->uni.color >= 0) {
			glEnableVertexAttribArray(shd->uni.color);
			glVertexAttribPointer(shd->uni.color, 4, GL_FLOAT, GL_FALSE, sizeof(_vertex_buffer),
					BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
		}
	}
	buffer_index += 4;

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
INLINE static void shd_set_params(const _shader *shd, GLuint fcountmod, GLuint fcount) {
	GLuint i, buffer_index = 0, texture_index = 1;

	if (shd->uni.mvp >= 0) {
		glUniformMatrix4fv(shd->uni.mvp, 1, GL_FALSE, opengl.mvp.data);
	}
	if (shd->uni.input_size >= 0) {
		glUniform2fv(shd->uni.input_size, 1, shd->info.input_size);
	}
	if (shd->uni.output_size >= 0) {
		glUniform2fv(shd->uni.output_size, 1, shd->info.output_size);
	}
	if (shd->uni.texture_size >= 0) {
		glUniform2fv(shd->uni.texture_size, 1, shd->info.texture_size);
	}
	if (shd->uni.frame_count >= 0) {
		if (fcountmod) {
			fcount %= fcountmod;
		}
		glUniform1i(shd->uni.frame_count, fcount);
	}
	if (shd->uni.frame_direction >= 0) {
		//glUniform1i(shd->uni.frame_direction, state_manager_frame_is_reversed() ? -1 : 1);
		glUniform1i(shd->uni.frame_direction, 1);
	}

	// lut
	for (i = 0; i < shader_effect.luts; i++) {
		if (shd->uni.lut[i] >= 0) {
			glActiveTexture(GL_TEXTURE0 + texture_index);
			glBindTexture(GL_TEXTURE_2D, opengl.lut[i].id);
			glUniform1i(shd->uni.lut[i], texture_index);
			texture_index++;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, shd->vbo);

	if (shd->uni.tex_coord >= 0) {
		glEnableVertexAttribArray(shd->uni.tex_coord);
		glVertexAttribPointer(shd->uni.tex_coord, 2, GL_FLOAT, GL_FALSE,
				sizeof(_vertex_buffer), BUFFER_OFFSET(buffer_index));
	}
	buffer_index += 2;

	if (shd->uni.vertex_coord >= 0) {
		glEnableVertexAttribArray(shd->uni.vertex_coord);
		glVertexAttribPointer(shd->uni.vertex_coord, 2, GL_FLOAT, GL_FALSE,
				sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
	}
	buffer_index += 2;

	{
		if (shd->uni.COLOR >= 0) {
			glEnableVertexAttribArray(shd->uni.COLOR);
			glVertexAttribPointer(shd->uni.COLOR, 4, GL_FLOAT, GL_FALSE,
					sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
		}
		if (shd->uni.color >= 0) {
			glEnableVertexAttribArray(shd->uni.color);
			glVertexAttribPointer(shd->uni.color, 4, GL_FLOAT, GL_FALSE,
					sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
		}
	}
	buffer_index += 4;

	// ORIG
	if (shd->uni.orig.texture >= 0) {
		glActiveTexture(GL_TEXTURE0 + texture_index);
		glBindTexture(GL_TEXTURE_2D, opengl.screen.tex[opengl.screen.index].id);
		glUniform1i(shd->uni.orig.texture, texture_index);
		texture_index++;
	}
	if (shd->uni.orig.input_size >= 0) {
		glUniform2fv(shd->uni.orig.input_size, 1,
				opengl.screen.tex[opengl.screen.index].shader.info.input_size);
	}
	if (shd->uni.orig.texture_size >= 0) {
		glUniform2fv(shd->uni.orig.texture_size, 1,
				opengl.screen.tex[opengl.screen.index].shader.info.texture_size);
	}
	if (shd->uni.orig.tex_coord >= 0) {
		glEnableVertexAttribArray(shd->uni.orig.tex_coord);
		glVertexAttribPointer(shd->uni.orig.tex_coord, 2, GL_FLOAT, GL_FALSE,
				sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
	}
	// PREV (uso le stesse tex_coord di ORIG)
	{
		GLint circle_index = opengl.screen.index - 1;

		for (i = 0; i < (opengl.screen.in_use - 1); i++) {
			if (circle_index < 0) {
				circle_index = opengl.screen.in_use - 1;
			}

			if (shd->uni.prev[i].texture >= 0) {
				glActiveTexture(GL_TEXTURE0 + texture_index);
				glBindTexture(GL_TEXTURE_2D, opengl.screen.tex[circle_index].id);
				glUniform1i(shd->uni.prev[i].texture, texture_index);
				texture_index++;
			}

			if (shd->uni.prev[i].tex_coord >= 0) {
				glEnableVertexAttribArray(shd->uni.prev[i].tex_coord);
				glVertexAttribPointer(shd->uni.prev[i].tex_coord, 2, GL_FLOAT, GL_FALSE,
						sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
			}

			circle_index--;
		}
	}
	buffer_index += 2;

	// FEEDBACK
	if (opengl.feedback.in_use) {
		if (shd->uni.feedback.texture >= 0) {
			glActiveTexture(GL_TEXTURE0 + texture_index);
			glBindTexture(GL_TEXTURE_2D, opengl.feedback.tex.id);
			glUniform1i(shd->uni.feedback.texture, texture_index);
			texture_index++;
		}
		if (shd->uni.feedback.input_size >= 0) {
			glUniform2fv(shd->uni.feedback.input_size, 1,
					opengl.texture[shader_effect.feedback_pass].shader.info.input_size);
		}
		if (shd->uni.feedback.texture_size >= 0) {
			glUniform2fv(shd->uni.feedback.texture_size, 1,
					opengl.texture[shader_effect.feedback_pass].shader.info.texture_size);
		}
		if (shd->uni.feedback.tex_coord >= 0) {
			glEnableVertexAttribArray(shd->uni.feedback.tex_coord);
			glVertexAttribPointer(shd->uni.feedback.tex_coord, 2, GL_FLOAT, GL_FALSE,
					sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
		}
	}
	buffer_index += 2;

	// PASSPREV
	for (i = 0; i < shader_effect.running_pass; i++) {
		GLuint index = (shader_effect.running_pass - 1) - i;

		if (shd->uni.passprev[i].texture >= 0) {
			glActiveTexture(GL_TEXTURE0 + texture_index);
			glBindTexture(GL_TEXTURE_2D, opengl.texture[index].id);
			glUniform1i(shd->uni.passprev[i].texture, texture_index);
			texture_index++;
		}
		if (shd->uni.passprev[i].input_size >= 0) {
			glUniform2fv(shd->uni.passprev[i].input_size, 1,
					opengl.texture[i].shader.info.input_size);
		}
		if (shd->uni.passprev[i].texture_size >= 0) {
			glUniform2fv(shd->uni.passprev[i].texture_size, 1,
					opengl.texture[i].shader.info.texture_size);
		}
		if (shd->uni.passprev[i].tex_coord >= 0) {
			glEnableVertexAttribArray(shd->uni.passprev[i].tex_coord);
			glVertexAttribPointer(shd->uni.passprev[i].tex_coord, 2, GL_FLOAT, GL_FALSE,
					sizeof(_vertex_buffer),
					BUFFER_OFFSET(sizeof(GLfloat) * (buffer_index + (index * 2))));
		}
	}
	buffer_index += (MAX_PASS * 2);

	// LUT
	if (shd->uni.lut_tex_coord >= 0) {
		glEnableVertexAttribArray(shd->uni.lut_tex_coord);
		glVertexAttribPointer(shd->uni.lut_tex_coord, 2, GL_FLOAT, GL_FALSE,
				sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
	}
	buffer_index += 2;

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glActiveTexture(GL_TEXTURE0);

	// params
	for (i = 0; i < shader_effect.params; i++) {
		if (shd->uni.param[i] >= 0) {
			glUniform1fv(shd->uni.param[i], 1, &shader_effect.param[i].value);
		}
	}
}

static BYTE tex_create(_texture *texture, GLuint index, GLuint clean) {
	_shader_pass *sp = &shader_effect.sp[index];
	_shader_scale *sc = &sp->sc;
	const _shader_pass *next = &shader_effect.sp[index + 1];
	const _vertex_buffer *vb = vb_upright;
	const _texture_rect *prev;
	const GLuint flt = (opengl.interpolation || opengl.PSS) ? GL_LINEAR : GL_NEAREST;
	const GLuint flt_mip =(opengl.interpolation || opengl.PSS) ?
			GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;
	_texture_rect *rect = &texture->rect;
	_texture_viewport *vp = &texture->vp;
	GLuint min_flt, mag_flt, wrap;

	if (index == 0) {
		prev = &opengl.screen.tex[0].rect;
	} else {
		prev = &opengl.texture[index - 1].rect;
	}

	if (index == shader_effect.last_pass) {
		vb = vb_flipped;
		sc->scale.x = 1.0f;
		sc->scale.y = 1.0f;
		sc->type.x = SHADER_SCALE_VIEWPORT;
		sc->type.y = SHADER_SCALE_VIEWPORT;
	}

#if defined FHOPENGLGEST
	switch (sc->type.x) {
		case SHADER_SCALE_DEFAULT:
		case SHADER_SCALE_INPUT:
			rect->base.w = (GLfloat) prev->base.w * sc->scale.x;
			rect->max.w = (GLfloat) prev->max.w * sc->scale.x;
			break;
		case SHADER_SCALE_ABSOLUTE:
			rect->base.w = rect->max.w = sc->abs.x;
			break;
		case SHADER_SCALE_VIEWPORT:
			rect->base.w = rect->max.w = (GLfloat) surface_sdl->w * sc->scale.x;
			break;
	}
	switch (sc->type.y) {
		case SHADER_SCALE_DEFAULT:
		case SHADER_SCALE_INPUT:
			rect->base.h = (GLfloat) prev->base.h * sc->scale.y;
			rect->max.h = (GLfloat) prev->max.h * sc->scale.y;
			break;
		case SHADER_SCALE_ABSOLUTE:
			rect->base.h = rect->max.h = sc->abs.y;
			break;
		case SHADER_SCALE_VIEWPORT:
			rect->base.h = rect->max.h = (GLfloat) surface_sdl->h * sc->scale.y;
			break;
	}

	rect->w = power_of_two(rect->base.w);
	rect->h = power_of_two(rect->base.h);
#else
	switch (sc->type.x) {
		case SHADER_SCALE_DEFAULT:
		case SHADER_SCALE_INPUT:
			rect->w = (GLfloat) prev->w * sc->scale.x;
			rect->base.w = (GLfloat) prev->base.w * sc->scale.x;
			rect->max.w = (GLfloat) prev->max.w * sc->scale.x;
			break;
		case SHADER_SCALE_ABSOLUTE:
			rect->w = rect->base.w = rect->max.w = sc->abs.x;
			break;
		case SHADER_SCALE_VIEWPORT:
			rect->w = rect->base.w = rect->max.w = (GLfloat) surface_sdl->w * sc->scale.x;
			break;
	}
	switch (sc->type.y) {
		case SHADER_SCALE_DEFAULT:
		case SHADER_SCALE_INPUT:
			rect->h = (GLfloat) prev->h * sc->scale.y;
			rect->base.h = (GLfloat) prev->base.h * sc->scale.y;
			rect->max.h = (GLfloat) prev->max.h * sc->scale.y;
			break;
		case SHADER_SCALE_ABSOLUTE:
			rect->h = rect->base.h = rect->max.h = sc->abs.y;
			break;
		case SHADER_SCALE_VIEWPORT:
			rect->h = rect->base.h = rect->max.h = (GLfloat) surface_sdl->h * sc->scale.y;
			break;
	}

	rect->w = power_of_two(rect->w);
	rect->h = power_of_two(rect->h);
#endif

	vp->x = 0;
	vp->y = 0;
	vp->w = rect->base.w;
	vp->h = rect->base.h;

	glGenTextures(1, &texture->id);
	glBindTexture(GL_TEXTURE_2D, texture->id);

	min_flt = next->mipmap_input ? flt_mip : flt;

	if (next->linear) {
		min_flt = next->mipmap_input ?
				(next->linear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST) :
				(next->linear ? GL_LINEAR : GL_NEAREST);
	}

	switch (min_flt) {
		case GL_LINEAR_MIPMAP_LINEAR:
			mag_flt = GL_LINEAR;
			break;
		case GL_NEAREST_MIPMAP_NEAREST:
			mag_flt = GL_NEAREST;
			break;
		default:
			mag_flt = min_flt;
			break;
	}

	switch (next->wrap) {
		case TEXTURE_WRAP_BORDER:
		default:
			wrap = GL_CLAMP_TO_BORDER;
			break;
		case TEXTURE_WRAP_EDGE:
			wrap = GL_CLAMP_TO_EDGE;
			break;
		case TEXTURE_WRAP_REPEAT:
			wrap = GL_REPEAT;
			break;
		case TEXTURE_WRAP_MIRRORED_REPEAT:
			wrap = GL_MIRRORED_REPEAT;
			break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_flt);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_flt);

	// creo la texture nella GPU
	if (sp->fbo_flt && opengl.supported_fbo.flt) {
		glTexImage2D(GL_TEXTURE_2D, 0, TI_F_INTFRM, rect->w, rect->h, 0, TI_FRM, TI_F_TYPE, NULL);
	} else if (sp->fbo_srgb && opengl.supported_fbo.srgb) {
		glTexImage2D(GL_TEXTURE_2D, 0, TI_S_INTFRM, rect->w, rect->h, 0, TI_FRM, TI_S_TYPE, NULL);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, TI_INTFRM, rect->w, rect->h, 0, TI_FRM, TI_TYPE, NULL);
	}

	// gestione fbo
	glGenFramebuffers(1, &texture->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, texture->fbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->id, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "INFO: Error on create FBO.\n");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		return (EXIT_ERROR);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenBuffers(1, &texture->shader.vbo);
	memcpy(texture->shader.vb, vb, sizeof(vb_flipped));

	if (sp->alias[0]) {
		char define[128];

		snprintf(define, sizeof(define), "#define %s_ALIAS\n", sp->alias);
		strncpy(opengl.alias_define, define, sizeof(opengl.alias_define));
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	return (EXIT_OK);
}
static void tex_create_simple(_texture_simple *texture, GLuint w, GLuint h, BYTE fixed) {
	_texture_rect *rect = &texture->rect;
	_shader *shd = &texture->shader;

	glGenTextures(1, &texture->id);
	glBindTexture(GL_TEXTURE_2D, texture->id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	rect->base.w = w;
	rect->base.h = h;

	if (fixed) {
#if defined FHOPENGLGEST
		rect->w = power_of_two(rect->base.w);
		rect->h = power_of_two(rect->base.h);
#else
		// rect->w = 1024 e rect->h = 1024 sono
		// le dimensioni che imposta retroarch
		// ma su alcune shader l'effetto e' piu'
		// sgranato ("mudlord/emboss.h" e
		// "antialiasing/fx-aa.h" sono un esempio)
		rect->w = 1024;
		rect->h = 1024;
#endif
	} else {
		rect->w = rect->base.w;
		rect->h = rect->base.h;
	}

	shd->info.input_size[0] = (GLfloat) rect->base.w;
	shd->info.input_size[1] = (GLfloat) rect->base.h;
	shd->info.texture_size[0] = (GLfloat) rect->w;
	shd->info.texture_size[1] = (GLfloat) rect->h;

	shd_set_vertex_buffer(&shd->vb[0], rect);

	// pulisco la texture
	{
		GLuint size = rect->w * rect->h * 4;
		GLubyte *empty = malloc(size);

		memset(empty, 0x00, size);
		glTexImage2D(GL_TEXTURE_2D, 0, TI_INTFRM, rect->w, rect->h, 0, TI_FRM, TI_TYPE, empty);
		free(empty);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}
static void tex_create_lut(_lut *lut, GLuint index) {
	_lut_pass *lp = &shader_effect.lp[index];
	GLuint min_flt, mag_flt, wrap;

	glGenTextures(1, &lut->id);
	glBindTexture(GL_TEXTURE_2D, lut->id);

	min_flt = lp->mipmap ?
			(lp->linear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST) :
			(lp->linear ? GL_LINEAR : GL_NEAREST);

	switch (min_flt) {
		case GL_LINEAR_MIPMAP_LINEAR:
			mag_flt = GL_LINEAR;
			break;
		case GL_NEAREST_MIPMAP_NEAREST:
			mag_flt = GL_NEAREST;
			break;
		default:
			mag_flt = min_flt;
			break;
	}

	switch (lp->wrap) {
		case TEXTURE_WRAP_BORDER:
		default:
			wrap = GL_CLAMP_TO_BORDER;
			break;
		case TEXTURE_WRAP_EDGE:
			wrap = GL_CLAMP_TO_EDGE;
			break;
		case TEXTURE_WRAP_REPEAT:
			wrap = GL_REPEAT;
			break;
		case TEXTURE_WRAP_MIRRORED_REPEAT:
			wrap = GL_MIRRORED_REPEAT;
			break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_flt);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_flt);

	gui_load_lut(lut, lp->path);

	glTexImage2D(GL_TEXTURE_2D, 0, TI_INTFRM, lut->w, lut->h, 0, TI_FRM, TI_TYPE, lut->bits);

	if (lp->mipmap) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}
static void tex_delete_screen(void) {
	GLint i;

	if (opengl.sdl.surface) {
		SDL_FreeSurface(opengl.sdl.surface);
		opengl.sdl.surface = NULL;
	}

	memset(&opengl.alias_define, 0x00, sizeof(opengl.alias_define));

	opengl.screen.in_use = 0;
	opengl.screen.index = 0;

	for (i = 0; i < LENGTH(opengl.screen.tex); i++) {
		if (opengl.screen.tex[i].id) {
			glDeleteTextures(1, &opengl.screen.tex[i].id);
			opengl.screen.tex[i].id = 0;
		}
	}

	{
		opengl.feedback.in_use = FALSE;

		if (opengl.feedback.tex.id) {
			glDeleteTextures(1, &opengl.feedback.tex.id);
			opengl.feedback.tex.id = 0;
		}
		if (opengl.feedback.tex.fbo) {
			glDeleteFramebuffers(1, &opengl.feedback.tex.fbo);
			opengl.feedback.tex.fbo = 0;
		}
		if (opengl.feedback.tex.shader.vbo) {
			glDeleteBuffers(1, &opengl.feedback.tex.shader.vbo);
			opengl.feedback.tex.shader.vbo = 0;
		}
	}

	for (i = 0; i < LENGTH(opengl.texture); i++) {
		if (opengl.texture[i].id) {
			glDeleteTextures(1, &opengl.texture[i].id);
			opengl.texture[i].id = 0;
		}
		if (opengl.texture[i].fbo) {
			glDeleteFramebuffers(1, &opengl.texture[i].fbo);
			opengl.texture[i].fbo = 0;
		}
		if (opengl.texture[i].shader.vbo) {
			glDeleteBuffers(1, &opengl.texture[i].shader.vbo);
			opengl.texture[i].shader.vbo = 0;
		}
		shd_delete(&opengl.texture[i].shader);
	}

	for (i = 0; i < LENGTH(opengl.lut); i++) {
		if (opengl.lut[i].id) {
			glDeleteTextures(1, &opengl.lut[i].id);
			opengl.lut[i].id = 0;
		}
	}
}
static void tex_delete_text(void) {
	if (opengl.text.id) {
		glDeleteTextures(1, &opengl.text.id);
		opengl.text.id = 0;
	}

	if (opengl.text.shader.vbo) {
		glDeleteBuffers(1, &opengl.text.shader.vbo);
		opengl.text.shader.vbo = 0;
	}

	shd_delete(&opengl.text.shader);
}

static GLint power_of_two(GLint base) {
	GLint pot = 1;

	while (pot < base) {
		pot <<= 1;
	}
	return (pot);
}
const GLint get_integer(const GLenum penum) {
	GLint result;

	glGetIntegerv(penum, &result);

	return (result);
}
