/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
 *  for some codes :
 *  Copyright (C) 2010-2015 The RetroArch team
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
 *
 */

#include "common.h"
#include "opengl.h"
#include "ppu.h"
#include "conf.h"
#include "qt.h"
#include "cgp.h"

#define MAT_ELEM_4X4(mat, r, c) ((mat).data[4 * (c) + (r)])
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define BUFFER_VB_OFFSET(a, i) ((char *)&a + (i))

enum _opengl_texture_format {
	TI_INTFRM = GL_RGBA8,
	TI_FRM = GL_BGRA,
	TI_TYPE = GL_UNSIGNED_BYTE,
	TI_F_INTFRM = GL_RGBA32F,
	TI_F_TYPE = GL_FLOAT,
	TI_S_INTFRM = GL_SRGB8_ALPHA8,
	TI_S_TYPE = GL_UNSIGNED_BYTE
};

static BYTE opengl_glew_init(void);
static BYTE opengl_texture_create(_texture *texture, GLuint index, GLuint clean);
static void opengl_texture_simple_create(_texture_simple *texture, GLuint w, GLuint h, BYTE text);
static BYTE opengl_texture_lut_create(_lut *lut, GLuint index);
static void opengl_shader_delete(_shader *shd);
#if !defined (RELEASE)
static void opengl_shader_print_log(GLuint obj, BYTE ret);
#endif
static char *opengl_shader_file2string(const GLchar *path);
static void opengl_shader_uni_texture_clear(_shader_uniforms_tex *sut);
static void opengl_shader_uni_texture(_shader_uniforms_tex *uni, GLint prg, GLchar *fmt, ...);
static GLint opengl_shader_get_uni(GLuint prog, const char *param);
static GLint opengl_shader_get_atr(GLuint prog, const char *param);
static void opengl_vertex_buffer_set(_vertex_buffer *vb, _texture_rect *rect);
static const GLint opengl_integer_get(const GLenum penum);
static void opengl_matrix_4x4_identity(_math_matrix_4x4 *mat);
static void opengl_matrix_4x4_ortho(_math_matrix_4x4 *mat, GLfloat left, GLfloat right,
		GLfloat bottom, GLfloat top, GLfloat znear, GLfloat zfar);
INLINE void opengl_shader_filter(uint8_t linear, uint8_t mipmap, uint8_t interpolation,
		GLuint *mag, GLuint *min);
INLINE static void opengl_shader_params_text_set(_shader *shd);

// glsl
static BYTE opengl_shader_glsl_init(GLuint pass, _shader *shd, GLchar *code, const GLchar *path);
INLINE static void opengl_shader_glsl_params_set(const _shader *shd, GLuint fcountmod,
		GLuint fcount);
INLINE static void opengl_shader_glsl_disable_attrib(void);
// cg
#if defined (WITH_OPENGL_CG)
#if defined (DEBUG)
static void opengl_shader_cg_error_handler(CGcontext ctx, CGerror error, void *data);
#endif
static BYTE opengl_shader_cg_init(GLuint pass, _shader *shd, GLchar *code, const GLchar *path);
static void opengl_shader_cg_clstate_ctrl(CGparameter *dst, CGparameter *param,
        const char *semantic);
static void opengl_shader_cg_param2f_ctrl(CGparameter *dst, CGparameter *param,
        const char *semantic);
static void opengl_shader_cg_uni_texture_clear(_shader_uniforms_tex_cg *sut);
static void opengl_shader_cg_uni_texture(_shader_uniforms_tex_cg *sut, _shader_prg_cg *prg,
		char *fmt, ...);
INLINE static void opengl_shader_cg_params_set(const _texture *texture, GLuint fcountmod,
		GLuint fcount);
INLINE static void opengl_shader_cg_disable_stpm(void);
#endif

static const GLchar *uni_prefixes[] = { "", "ruby", };
static const _vertex_buffer vb_upright[4] = {
	{ 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
	{ 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
	{ 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
	{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
};
static const _vertex_buffer vb_flipped[4] = {
	{ 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
	{ 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
};

void opengl_init(void) {
	memset(&gfx.vp, 0x00, sizeof(gfx.vp));

	opengl.sdl.surface = NULL;
	opengl.sdl.flags = SDL_HWSURFACE | SDL_OPENGL;

	memset(&opengl.attribs, 0x00, sizeof(opengl.attribs));
	memset(&opengl.screen, 0x00, sizeof(opengl.screen));
	memset(&opengl.feedback, 0x00, sizeof(opengl.feedback));
	memset(&opengl.text, 0x00, sizeof(_texture_simple));
	memset(&opengl.texture, 0x00, LENGTH(opengl.texture) * sizeof(_texture));
	memset(&opengl.lut, 0x00, LENGTH(opengl.lut) * sizeof(_lut));

#if defined (WITH_OPENGL_CG)
	memset(&opengl.cg, 0x00, sizeof(opengl.cg));
#endif

	if (opengl_glew_init() == EXIT_ERROR) {
		opengl.supported = FALSE;
		return;
	}

	// Calculate projection
	opengl_matrix_4x4_ortho(&opengl.mvp, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
}
BYTE opengl_context_create(SDL_Surface *src) {
	GLuint i, w, h;

	glGetError();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DITHER);
	glDisable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);

	opengl_context_delete();

#if defined (WITH_OPENGL_CG)
	if (shader_effect.type == MS_CGP) {
		if ((opengl.cg.ctx = cgCreateContext()) == NULL) {
			return (EXIT_ERROR);
		}

#if defined (DEBUG)
		cgGLSetDebugMode(CG_TRUE);
		cgSetErrorHandler(opengl_shader_cg_error_handler, NULL);
#endif

		opengl.cg.profile.v = cgGLGetLatestProfile(CG_GL_VERTEX);
		opengl.cg.profile.f = cgGLGetLatestProfile(CG_GL_FRAGMENT);

		if ((opengl.cg.profile.v == CG_PROFILE_UNKNOWN)
				|| (opengl.cg.profile.f == CG_PROFILE_UNKNOWN)) {
			opengl_context_delete();
			return (EXIT_ERROR);
		}

		fprintf(stderr, "OPENGLCG: vertex profile %s\n", cgGetProfileString(opengl.cg.profile.v));
		cgGLSetOptimalOptions(opengl.cg.profile.v);
		cgGLEnableProfile(opengl.cg.profile.v);

		fprintf(stderr, "OPENGLCG: fragment profile %s\n", cgGetProfileString(opengl.cg.profile.f));
		cgGLSetOptimalOptions(opengl.cg.profile.f);
		cgGLEnableProfile(opengl.cg.profile.f);
	}
#endif

	if ((cfg->filter == NO_FILTER) || (cfg->filter >= FLTSHDSTART)) {
		w = gfx.rows;
		h = gfx.lines;
	} else {
		w = gfx.w[CURRENT];
		h = gfx.h[CURRENT];
	}

	opengl.sdl.surface = gfx_create_RGB_surface(src, w, h);

	// devo precalcolarmi il viewport finale
	{
		_viewport *vp = &gfx.vp;

		vp->x = 0;
		vp->y = 0;
		vp->w = src->w;
		vp->h = src->h;

		// configuro l'aspect ratio del fullscreen
		if (cfg->fullscreen && !cfg->stretch) {
			GLfloat ratio_surface = (((GLfloat) gfx.rows * gfx.pixel_aspect_ratio)
					/ (GLfloat) gfx.lines);
			GLfloat ratio_frame = (GLfloat) src->w / (GLfloat) src->h;

			if (ratio_frame > ratio_surface) {
				vp->w = (int) ((GLfloat) src->h * ratio_surface);
				vp->x = (int) (((GLfloat) src->w - (GLfloat) vp->w) * 0.5f);
			} else {
				vp->h = (int) ((GLfloat) src->w * ratio_surface);
				vp->y = (int) (((GLfloat) src->h - (GLfloat) vp->h) * 0.5f);
			}
		}
	}

	// screen
	opengl_texture_simple_create(&opengl.screen.tex[0], w, h, FALSE);

	// creo le restanti texture/fbo
	for (i = 0; i < shader_effect.pass; i++) {
		int rc;

		fprintf(stderr, "OPENGL: Setting pass %d\n", i);

		if (opengl_texture_create(&opengl.texture[i], i, FALSE) == EXIT_ERROR) {
			opengl_context_delete();
			return (EXIT_ERROR);
		}

		opengl.texture[i].shader.type = shader_effect.sp[i].type;

		if (opengl.texture[i].shader.type == MS_CGP) {
#if defined (WITH_OPENGL_CG)
			rc = opengl_shader_cg_init(i, &opengl.texture[i].shader, shader_effect.sp[i].code,
					shader_effect.sp[i].path);
#else
			return (EXIT_ERROR_SHADER);
#endif
		} else {
			rc = opengl_shader_glsl_init(i, &opengl.texture[i].shader, shader_effect.sp[i].code,
					shader_effect.sp[i].path);
		}

		if (rc != EXIT_OK) {
			opengl_context_delete();
			return (rc);
		}
	}

	// PREV (calcolo il numero di screen da utilizzare)
	// deve essere fatto dopo il opengl_shader_xxx_init().
	for (i = 0; i < shader_effect.pass; i++) {
		GLuint a;

		for (a = 0; a < LENGTH(opengl.texture[i].shader.glslp.uni.prev); a++) {
			if (opengl.texture[i].shader.type == MS_CGP) {
#if defined (WITH_OPENGL_CG)
				if (opengl.texture[i].shader.cgp.uni.prev[a].f.texture) {
					if (opengl.screen.in_use < (a + 1)) {
						opengl.screen.in_use = (a + 1);
					}
				}
#endif
			} else {
				if (opengl.texture[i].shader.glslp.uni.prev[a].texture >= 0) {
					if (opengl.screen.in_use < (a + 1)) {
						opengl.screen.in_use = (a + 1);
					}
				}
			}
		}
	}

	opengl.screen.in_use++;

	// PREV
	for (i = 1; i < opengl.screen.in_use; i++) {
		opengl_texture_simple_create(&opengl.screen.tex[i], w, h, FALSE);
	}

	// FEEDBACK
	if ((shader_effect.feedback_pass >= 0) && (shader_effect.feedback_pass < shader_effect.pass)) {
		opengl.feedback.in_use = TRUE;

		if (opengl_texture_create(&opengl.feedback.tex, shader_effect.feedback_pass, TRUE)
				== EXIT_ERROR) {
			opengl_context_delete();
			return (EXIT_ERROR);
		}
	}

	// testo
	{
		_shader *shd = &opengl.text.shader;

		opengl_texture_simple_create(&opengl.text, src->w, src->h, TRUE);

		glGenBuffers(1, &shd->vbo);
		memcpy(shd->vb, vb_flipped, sizeof(vb_flipped));

		glBindBuffer(GL_ARRAY_BUFFER, shd->vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(shd->vb), shd->vb, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		opengl_shader_glsl_init(0, shd, shader_code_blend(), NULL);
	}

	// lut
	for (i = 0; i < shader_effect.luts; i++) {
		if (opengl_texture_lut_create(&opengl.lut[i], i) == EXIT_ERROR) {
			opengl_context_delete();
			return (EXIT_ERROR_SHADER);
		}
	}

	// setto tutto quello che mi serve per il rendering
	for (i = 0; i < shader_effect.pass; i++) {
		_texture *texture = &opengl.texture[i];
		_shader *shd = &texture->shader;
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
		shd->info.output_size[0] = (GLfloat) texture->vp.w;
		shd->info.output_size[1] = (GLfloat) texture->vp.h;

		opengl_vertex_buffer_set(&shd->vb[0], prev);

		for (a = 0; a < LENGTH(shd->vb); a++) {
			// LUT texture coord
			shd->vb[a].luttx[0] = vb_upright[a].s0;
			shd->vb[a].luttx[1] = vb_upright[a].t0;

			// ORIG e PREV texture coord
			shd->vb[a].origtx[0] = opengl.screen.tex[0].shader.vb[a].s0;
			shd->vb[a].origtx[1] = opengl.screen.tex[0].shader.vb[a].t0;

			// FEEDBACK
			if (opengl.feedback.in_use) {
				shd->vb[a].feedtx[0] = opengl.texture[shader_effect.feedback_pass].shader.vb[a].s0;
				shd->vb[a].feedtx[1] = opengl.texture[shader_effect.feedback_pass].shader.vb[a].t0;
			}
		}

		// PASSPREV texture coord
		for (a = 0; a < i; a++) {
			for (b = 0; b < LENGTH(shd->vb); b++) {
				shd->vb[b].pptx[(a * 2) + 0] = opengl.texture[a].shader.vb[b].s0;
				shd->vb[b].pptx[(a * 2) + 1] = opengl.texture[a].shader.vb[b].t0;
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, shd->vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(shd->vb), shd->vb, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glFinish();

	memcpy(gfx.last_shader_file, cfg->shader_file, sizeof(gfx.last_shader_file));

 	return (EXIT_OK);
}
void opengl_context_delete(void) {
	GLint i;

	opengl_shader_glsl_disable_attrib();

#if defined (WITH_OPENGL_CG)
	opengl_shader_cg_disable_stpm();
	if (opengl.cg.profile.f) {
		cgGLUnbindProgram(opengl.cg.profile.f);
	}
	if (opengl.cg.profile.v) {
		cgGLUnbindProgram(opengl.cg.profile.v);
	}
#endif

	if (opengl.sdl.surface) {
		SDL_FreeSurface(opengl.sdl.surface);
		opengl.sdl.surface = NULL;
	}

	opengl.screen.in_use = 0;
	opengl.screen.index = 0;

	for (i = 0; i < LENGTH(opengl.screen.tex); i++) {
		if (opengl.screen.tex[i].id) {
			glDeleteTextures(1, &opengl.screen.tex[i].id);
			opengl.screen.tex[i].id = 0;
		}
	}

	{
		if (opengl.text.id) {
			glDeleteTextures(1, &opengl.text.id);
			opengl.text.id = 0;
		}

		if (opengl.text.shader.vbo) {
			glDeleteBuffers(1, &opengl.text.shader.vbo);
			opengl.text.shader.vbo = 0;
		}

		opengl_shader_delete(&opengl.text.shader);
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

		opengl_shader_delete(&opengl.texture[i].shader);
	}

	for (i = 0; i < LENGTH(opengl.lut); i++) {
		if (opengl.lut[i].id) {
			glDeleteTextures(1, &opengl.lut[i].id);
			opengl.lut[i].id = 0;
		}
	}

#if defined (WITH_OPENGL_CG)
	if (opengl.cg.ctx) {
		cgDestroyContext(opengl.cg.ctx);
		opengl.cg.ctx = NULL;
	}
#endif
}
void opengl_draw_scene(SDL_Surface *surface) {
	static GLuint prev_type = MS_MEM;
	const _texture_simple *scrtex = &opengl.screen.tex[opengl.screen.index];
	GLuint i;

	// screen
	glBindTexture(GL_TEXTURE_2D, scrtex->id);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->w);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, surface->w, surface->h, TI_FRM, TI_TYPE,
			surface->pixels);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	if (opengl.supported_fbo.srgb) {
		glEnable(GL_FRAMEBUFFER_SRGB);
	}

	// fbo e pass
	for (i = 0; i < shader_effect.pass; i++) {
		const _texture *texture = &opengl.texture[i];
		const _shader_pass *sp = &shader_effect.sp[i];
		GLuint id, fbo = texture->fbo, mag, min;

		shader_effect.running_pass = i;

		if (i == shader_effect.last_pass) {
			fbo = 0;
			if (opengl.supported_fbo.srgb) {
				glDisable(GL_FRAMEBUFFER_SRGB);
			}
		}

		if (i == 0) {
			id = scrtex->id;
		} else {
			id = opengl.texture[i - 1].id;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(texture->vp.x, texture->vp.y, texture->vp.w, texture->vp.h);
		glBindTexture(GL_TEXTURE_2D, id);
		if (sp->mipmap_input) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		opengl_shader_filter(sp->linear, sp->mipmap_input, (cfg->interpolation || gfx.PSS), &mag,
		        &min);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
		if (texture->shader.type == MS_CGP) {
#if defined (WITH_OPENGL_CG)
			if (prev_type != MS_CGP) {
				glUseProgram(0);
			}

			if (texture->shader.cgp.prg.f && texture->shader.cgp.prg.v) {
				cgGLBindProgram(texture->shader.cgp.prg.f);
				cgGLBindProgram(texture->shader.cgp.prg.v);

				cgGLEnableProfile(opengl.cg.profile.f);
				cgGLEnableProfile(opengl.cg.profile.v);

				opengl_shader_cg_params_set(texture, sp->frame_count_mod, ppu.frames);
			}
#endif
		} else {
			glUseProgram(texture->shader.glslp.prg);
			opengl_shader_glsl_params_set(&texture->shader, sp->frame_count_mod, ppu.frames);
		}
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		if (texture->shader.type == MS_CGP) {
#if defined (WITH_OPENGL_CG)
			opengl_shader_cg_disable_stpm();

			cgGLDisableProfile(opengl.cg.profile.f);
			cgGLDisableProfile(opengl.cg.profile.v);

			cgGLUnbindProgram(opengl.cg.profile.f);
			cgGLUnbindProgram(opengl.cg.profile.v);
#endif
		} else {
			opengl_shader_glsl_disable_attrib();
		}
		prev_type = texture->shader.type;
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
	glBindTexture(GL_TEXTURE_2D, opengl.text.id);
	glUseProgram(opengl.text.shader.glslp.prg);
	opengl_shader_params_text_set(&opengl.text.shader);
	glEnable(GL_BLEND);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisable(GL_BLEND);
	opengl_shader_glsl_disable_attrib();
	prev_type = MS_MEM;
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

static BYTE opengl_glew_init(void) {
	if (opengl.supported){
		GLenum err;

		glewExperimental = GL_TRUE;

		if ((err = glewInit()) != GLEW_OK) {
			fprintf(stderr, "OPENGL: %s\n", glewGetErrorString(err));
		} else {
			fprintf(stderr, "OPENGL: GPU %s (%s, %s)\n", glGetString(GL_RENDERER),
					glGetString(GL_VENDOR),
					glGetString(GL_VERSION));
			fprintf(stderr, "OPENGL: GL Version %d.%d %s\n",
					opengl_integer_get(GL_MAJOR_VERSION), opengl_integer_get(GL_MINOR_VERSION),
					opengl_integer_get(GL_CONTEXT_CORE_PROFILE_BIT) ? "Core" : "Compatibility");

			if (!GLEW_VERSION_3_0) {
				fprintf(stderr, "OPENGL: OpenGL 3.0 not supported. Disabled.\n");
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
static BYTE opengl_texture_create(_texture *texture, GLuint index, GLuint clean) {
	_shader_pass *sp = &shader_effect.sp[index];
	_shader_scale *sc = &sp->sc;
	const _shader_pass *next = &shader_effect.sp[index + 1];
	const _vertex_buffer *vb = vb_upright;
	const _texture_rect *prev;
	_texture_rect *rect = &texture->rect;
	_viewport *vp = &texture->vp;
	GLuint wrap;

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

#if defined (FH_SHADERS_GEST)
	switch (sc->type.x) {
		case SHADER_SCALE_DEFAULT:
		case SHADER_SCALE_INPUT:
			rect->base.w = (GLfloat) prev->base.w * sc->scale.x;
			break;
		case SHADER_SCALE_ABSOLUTE:
			rect->base.w = sc->abs.x;
			break;
		case SHADER_SCALE_VIEWPORT:
			rect->base.w = (GLfloat) gfx.vp.w * sc->scale.x;
			break;
	}
	switch (sc->type.y) {
		case SHADER_SCALE_DEFAULT:
		case SHADER_SCALE_INPUT:
			rect->base.h = (GLfloat) prev->base.h * sc->scale.y;
			break;
		case SHADER_SCALE_ABSOLUTE:
			rect->base.h = sc->abs.y;
			break;
		case SHADER_SCALE_VIEWPORT:
			rect->base.h = (GLfloat) gfx.vp.h * sc->scale.y;
			break;
	}

	rect->w = emu_power_of_two(rect->base.w);
	rect->h = emu_power_of_two(rect->base.h);
#else
	switch (sc->type.x) {
		case SHADER_SCALE_DEFAULT:
		case SHADER_SCALE_INPUT:
			rect->w = (GLfloat) prev->w * sc->scale.x;
			rect->base.w = (GLfloat) prev->base.w * sc->scale.x;
			break;
		case SHADER_SCALE_ABSOLUTE:
			rect->w = rect->base.w = sc->abs.x;
			break;
		case SHADER_SCALE_VIEWPORT:
			rect->w = rect->base.w = (GLfloat) gfx.vp.w * sc->scale.x;
			break;
	}
	switch (sc->type.y) {
		case SHADER_SCALE_DEFAULT:
		case SHADER_SCALE_INPUT:
			rect->h = (GLfloat) prev->h * sc->scale.y;
			rect->base.h = (GLfloat) prev->base.h * sc->scale.y;
			break;
		case SHADER_SCALE_ABSOLUTE:
			rect->h = rect->base.h = sc->abs.y;
			break;
		case SHADER_SCALE_VIEWPORT:
			rect->h = rect->base.h = (GLfloat) gfx.vp.h * sc->scale.y;
			break;
	}

	rect->w = emu_power_of_two(rect->w);
	rect->h = emu_power_of_two(rect->h);
#endif

	if (index == shader_effect.last_pass) {
		vp->x = gfx.vp.x;
		vp->y = gfx.vp.y;
		vp->w = gfx.vp.w;
		vp->h = gfx.vp.h;
	} else {
		vp->x = 0;
		vp->y = 0;
		vp->w = rect->base.w;
		vp->h = rect->base.h;
	}

	glGenTextures(1, &texture->id);
	glBindTexture(GL_TEXTURE_2D, texture->id);

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

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "OPENGL: Error on create FBO.\n");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		return (EXIT_ERROR);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenBuffers(1, &texture->shader.vbo);
	memcpy(texture->shader.vb, vb, sizeof(vb_flipped));

	return (EXIT_OK);
}
static void opengl_texture_simple_create(_texture_simple *texture, GLuint w, GLuint h, BYTE text) {
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

	if (!text) {
#if defined (FH_SHADERS_GEST)
		rect->w = emu_power_of_two(rect->base.w);
		rect->h = emu_power_of_two(rect->base.h);
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

	memcpy(shd->vb, vb_flipped, sizeof(vb_flipped));

	opengl_vertex_buffer_set(&shd->vb[0], rect);

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
static BYTE opengl_texture_lut_create(_lut *lut, GLuint index) {
	_lut_pass *lp = &shader_effect.lp[index];
	GLuint mag, min, wrap;

	glGenTextures(1, &lut->id);
	glBindTexture(GL_TEXTURE_2D, lut->id);

	lut->name = lp->name;

	opengl_shader_filter(lp->linear, lp->mipmap, lp->linear, &mag, &min);

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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);

	if (gui_load_lut(lut, lp->path) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, TI_INTFRM, lut->w, lut->h, 0, TI_FRM, TI_TYPE, lut->bits);

	if (lp->mipmap) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	return (EXIT_OK);
}
static void opengl_shader_delete(_shader *shd) {
	if (shd->type == MS_CGP) {
#if defined (WITH_OPENGL_CG)
		if (shd->cgp.prg.f) {
			cgDestroyProgram(shd->cgp.prg.f);
			shd->cgp.prg.f = NULL;
		}
		if (shd->cgp.prg.v) {
			cgDestroyProgram(shd->cgp.prg.v);
			shd->cgp.prg.v = NULL;
		}
		memset(&shd->cgp.uni, 0x00, sizeof(_shader_uniforms_cg));
#endif
	} else {
		if (shd->glslp.prg) {
			glDeleteProgram(shd->glslp.prg);
			shd->glslp.prg = 0;
		}
	}
}
#if !defined (RELEASE)
static void opengl_shader_print_log(GLuint obj, BYTE ret) {
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
			printf("OPENGL: %s", info_log);
			if (ret == TRUE) {
				printf("\n");
			}
		}
	}
}
#endif
static char *opengl_shader_file2string(const GLchar *path) {
	FILE *fd;
	long len, r;
	char *str;

	if (!(fd = fopen(path, "r"))) {
		fprintf(stderr, "OPENGL: Can't open file '%s' for reading\n", path);
		return (NULL);
	}

	fseek(fd, 0, SEEK_END);
	len = ftell(fd);

	fseek(fd, 0, SEEK_SET);

	if (!(str = (char *) malloc(len * sizeof(char)))) {
		fprintf(stderr, "OPENGL: Can't malloc space for '%s'\n", path);
		return (NULL);
	}

	r = fread(str, sizeof(char), len, fd);

	str[r - 1] = '\0';

	fclose(fd);

	return (str);
}
static void opengl_shader_uni_texture_clear(_shader_uniforms_tex *sut) {
	sut->texture = -1;
	sut->texture_size = -1;
	sut->input_size = -1;
	sut->tex_coord = -1;
}
static void opengl_shader_uni_texture(_shader_uniforms_tex *sut, GLint prg, GLchar *fmt, ...) {
	char type[50], buff[50];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(type, sizeof(type), fmt, ap);
	va_end(ap);

	if (sut->texture == -1) {
		snprintf(buff, sizeof(buff), "%s%s", type, "Texture");
		sut->texture = opengl_shader_get_uni(prg, buff);
	}
	if (sut->texture_size == -1) {
		snprintf(buff, sizeof(buff), "%s%s", type, "TextureSize");
		sut->texture_size = opengl_shader_get_uni(prg, buff);
	}
	if (sut->input_size == -1) {
		snprintf(buff, sizeof(buff), "%s%s", type, "InputSize");
		sut->input_size = opengl_shader_get_uni(prg, buff);
	}
	if (sut->tex_coord == -1) {
		snprintf(buff, sizeof(buff), "%s%s", type, "TexCoord");
		sut->tex_coord = opengl_shader_get_atr(prg, buff);
	}
}
static GLint opengl_shader_get_uni(GLuint prog, const char *param) {
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
static GLint opengl_shader_get_atr(GLuint prog, const char *param) {
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
static void opengl_vertex_buffer_set(_vertex_buffer *vb, _texture_rect *rect) {
	GLfloat x = (GLfloat) rect->base.w / rect->w;
	GLfloat y = (GLfloat) rect->base.h / rect->h;

	vb[1].s0 = x; vb[2].t0 = y;
	vb[3].s0 = x; vb[3].t0 = y;
}
static const GLint opengl_integer_get(const GLenum penum) {
	GLint result;

	glGetIntegerv(penum, &result);

	return (result);
}
static void opengl_matrix_4x4_identity(_math_matrix_4x4 *mat) {
	int i;

	memset(mat, 0, sizeof(*mat));

	for (i = 0; i < 4; i++) {
		MAT_ELEM_4X4(*mat, i, i) = 1.0f;
	}
}
static void opengl_matrix_4x4_ortho(_math_matrix_4x4 *mat, GLfloat left, GLfloat right,
		GLfloat bottom, GLfloat top, GLfloat znear, GLfloat zfar) {
	float tx, ty, tz;

	opengl_matrix_4x4_identity(mat);

	tx = -(right + left) / (right - left);
	ty = -(top + bottom) / (top - bottom);
	tz = -(zfar + znear) / (zfar - znear);

	MAT_ELEM_4X4(*mat, 0, 0) = 2.0f / (right - left);
	MAT_ELEM_4X4(*mat, 1, 1) = 2.0f / (top - bottom);
	MAT_ELEM_4X4(*mat, 2, 2) = -2.0f / (zfar - znear);
	MAT_ELEM_4X4(*mat, 0, 3) = tx;
	MAT_ELEM_4X4(*mat, 1, 3) = ty;
	MAT_ELEM_4X4(*mat, 2, 3) = tz;
}
INLINE void opengl_shader_filter(uint8_t linear, uint8_t mipmap, uint8_t interpolation,
		GLuint *mag, GLuint *min) {
	switch (linear) {
		case TEXTURE_LINEAR_DISAB:
			(*min) = mipmap ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
			break;
		case TEXTURE_LINEAR_ENAB:
			(*min) = mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
			break;
		default:
			(*min) = mipmap ?
					(interpolation ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST) :
					(interpolation ? GL_LINEAR : GL_NEAREST);
			break;
	}

	switch ((*min)) {
		case GL_LINEAR_MIPMAP_LINEAR:
			(*mag) = GL_LINEAR;
			break;
		case GL_NEAREST_MIPMAP_NEAREST:
			(*mag) = GL_NEAREST;
			break;
		default:
			(*mag) = (*min);
			break;
	}
}
INLINE static void opengl_shader_params_text_set(_shader *shd) {
	GLuint buffer_index = 0;

	if (shd->glslp.uni.mvp >= 0) {
		glUniformMatrix4fv(shd->glslp.uni.mvp, 1, GL_FALSE, opengl.mvp.data);
	}

	glBindBuffer(GL_ARRAY_BUFFER, shd->vbo);

	if (shd->glslp.uni.vertex_coord >= 0) {
		glEnableVertexAttribArray(shd->glslp.uni.vertex_coord);
		glVertexAttribPointer(shd->glslp.uni.vertex_coord, 2, GL_FLOAT, GL_FALSE,
				sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
		opengl.attribs.attrib[opengl.attribs.count++] = shd->glslp.uni.vertex_coord;
	}
	buffer_index += 2;

	{
		if (shd->glslp.uni.COLOR >= 0) {
			glEnableVertexAttribArray(shd->glslp.uni.COLOR);
			glVertexAttribPointer(shd->glslp.uni.COLOR, 4, GL_FLOAT, GL_FALSE,
					sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
			opengl.attribs.attrib[opengl.attribs.count++] = shd->glslp.uni.COLOR;
		}
		if (shd->glslp.uni.color >= 0) {
			glEnableVertexAttribArray(shd->glslp.uni.color);
			glVertexAttribPointer(shd->glslp.uni.color, 4, GL_FLOAT, GL_FALSE,
					sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
			opengl.attribs.attrib[opengl.attribs.count++] = shd->glslp.uni.color;
		}
	}
	buffer_index += 4;

	if (shd->glslp.uni.tex_coord >= 0) {
		glEnableVertexAttribArray(shd->glslp.uni.tex_coord);
		glVertexAttribPointer(shd->glslp.uni.tex_coord, 2, GL_FLOAT, GL_FALSE,
				sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
		opengl.attribs.attrib[opengl.attribs.count++] = shd->glslp.uni.tex_coord;
	}
	buffer_index += 2;

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// glsl
static BYTE opengl_shader_glsl_init(GLuint pass, _shader *shd, GLchar *code, const GLchar *path) {
	const GLchar *src[3];
	char alias_define[MAX_PASS * 128];
	GLuint i, vrt, frg;
	GLint success = 0;

	if ((code == NULL) && ((path == NULL) || !path[0])) {
		return (EXIT_ERROR_SHADER);
	}

	if (path && path[0]) {
		code = opengl_shader_file2string(path);
	}

	// program
	shd->glslp.prg = glCreateProgram();

	if (!shd->glslp.prg) {
		if (path && path[0] && code) {
			free(code);
			code = NULL;
		}
		return (EXIT_ERROR);
	}

	memset (alias_define, 0x00, sizeof(alias_define));

	for (i = 0; i < pass; i++) {
		_shader_pass *sp = &shader_effect.sp[i];

		if (sp->alias[0]) {
			char define[128];

			snprintf(define, sizeof(define), "#define %s_ALIAS\n", sp->alias);
			strncat(alias_define, define, sizeof(define));
		}
	}

	src[1] = alias_define;
	src[2] = code;

	// vertex
	src[0] = "#define VERTEX\n#define PARAMETER_UNIFORM\n";
	vrt = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vrt, 3, src, NULL);
	glCompileShader(vrt);
#if !defined (RELEASE)
	opengl_shader_print_log(vrt, FALSE);
#endif
	glGetShaderiv(vrt, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		if (path && path[0] && code) {
			free(code);
			code = NULL;
		}
		return (EXIT_ERROR_SHADER);
	}
	glAttachShader(shd->glslp.prg, vrt);
	glDeleteShader(vrt);

	// fragment
	src[0] = "#define FRAGMENT\n#define PARAMETER_UNIFORM\n";
	frg = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frg, 3, src, NULL);
	glCompileShader(frg);
#if !defined (RELEASE)
	opengl_shader_print_log(frg, FALSE);
#endif
	glGetShaderiv(vrt, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		if (path && path[0] && code) {
			free(code);
			code = NULL;
		}
		return (EXIT_ERROR_SHADER);
	}
	glAttachShader(shd->glslp.prg, frg);
	glDeleteShader(frg);

	glLinkProgram(shd->glslp.prg);
#if !defined (RELEASE)
	opengl_shader_print_log(shd->glslp.prg, TRUE);
#endif
	glGetProgramiv(shd->glslp.prg, GL_LINK_STATUS, &success);
	if (success == GL_FALSE) {
		if (path && path[0] && code) {
			free(code);
			code = NULL;
		}
		return (EXIT_ERROR_SHADER);
	}

	if (path && path[0] && code) {
		free(code);
		code = NULL;
	}

	glUseProgram(shd->glslp.prg);

	glUniform1i(opengl_shader_get_uni(shd->glslp.prg, "Texture"), 0);

	shd->glslp.uni.mvp = opengl_shader_get_uni(shd->glslp.prg, "MVPMatrix");
	shd->glslp.uni.vertex_coord = opengl_shader_get_atr(shd->glslp.prg, "VertexCoord");
	{
		// alcuni driver fanno distinzione tra COLOR e Color
		// se lascio solo Color con questi driver l'applicazione crasha
		// mentre su altri driver se uso solo COLOR la shader non funziona
		// quindi utilizzo entrambi.
		shd->glslp.uni.COLOR = opengl_shader_get_atr(shd->glslp.prg, "COLOR");
		shd->glslp.uni.color = opengl_shader_get_atr(shd->glslp.prg, "Color");
	}
	shd->glslp.uni.tex_coord = opengl_shader_get_atr(shd->glslp.prg, "TexCoord");
	shd->glslp.uni.lut_tex_coord = opengl_shader_get_atr(shd->glslp.prg, "LUTTexCoord");

	shd->glslp.uni.input_size = opengl_shader_get_uni(shd->glslp.prg, "InputSize");
	shd->glslp.uni.output_size = opengl_shader_get_uni(shd->glslp.prg, "OutputSize");
	shd->glslp.uni.texture_size = opengl_shader_get_uni(shd->glslp.prg, "TextureSize");

	shd->glslp.uni.frame_count = opengl_shader_get_uni(shd->glslp.prg, "FrameCount");
	shd->glslp.uni.frame_direction = opengl_shader_get_uni(shd->glslp.prg, "FrameDirection");

	for (i = 0; i < shader_effect.params; i++) {
		shd->glslp.uni.param[i] = opengl_shader_get_uni(shd->glslp.prg, shader_effect.param[i].name);
	}

	for (i = 0; i < shader_effect.luts; i++) {
		shd->glslp.uni.lut[i] = opengl_shader_get_uni(shd->glslp.prg, shader_effect.lp[i].name);
	}

	opengl_shader_uni_texture_clear(&shd->glslp.uni.orig);
	opengl_shader_uni_texture(&shd->glslp.uni.orig, shd->glslp.prg, "Orig");
	if (pass > 1) {
		opengl_shader_uni_texture(&shd->glslp.uni.orig, shd->glslp.prg, "PassPrev%u", pass);
	}

	opengl_shader_uni_texture_clear(&shd->glslp.uni.feedback);
	opengl_shader_uni_texture(&shd->glslp.uni.feedback, shd->glslp.prg, "Feedback");

	for (i = 0; i < pass; i++) {
		opengl_shader_uni_texture_clear(&shd->glslp.uni.passprev[i]);

		opengl_shader_uni_texture(&shd->glslp.uni.passprev[i], shd->glslp.prg, "Pass%u", i);
		opengl_shader_uni_texture(&shd->glslp.uni.passprev[i], shd->glslp.prg, "PassPrev%u",
		        pass - i);

		if (shader_effect.sp[i].alias[0]) {
			opengl_shader_uni_texture(&shd->glslp.uni.passprev[i], shd->glslp.prg,
			        shader_effect.sp[i].alias);
		}
	}

	opengl_shader_uni_texture_clear(&shd->glslp.uni.prev[0]);
	opengl_shader_uni_texture(&shd->glslp.uni.prev[0], shd->glslp.prg, "Prev");

	for (i = 1; i < LENGTH(shd->glslp.uni.prev); i++) {
		opengl_shader_uni_texture_clear(&shd->glslp.uni.prev[i]);
		opengl_shader_uni_texture(&shd->glslp.uni.prev[i], shd->glslp.prg, "Prev%u", i);
	}

	glUseProgram(0);

	return (EXIT_OK);
}
INLINE static void opengl_shader_glsl_params_set(const _shader *shd, GLuint fcountmod,
		GLuint fcount) {
	GLuint i, buffer_index = 0, texture_index = 1;

	if (shd->glslp.uni.mvp >= 0) {
		glUniformMatrix4fv(shd->glslp.uni.mvp, 1, GL_FALSE, opengl.mvp.data);
	}
	if (shd->glslp.uni.input_size >= 0) {
		glUniform2fv(shd->glslp.uni.input_size, 1, shd->info.input_size);
	}
	if (shd->glslp.uni.output_size >= 0) {
		glUniform2fv(shd->glslp.uni.output_size, 1, shd->info.output_size);
	}
	if (shd->glslp.uni.texture_size >= 0) {
		glUniform2fv(shd->glslp.uni.texture_size, 1, shd->info.texture_size);
	}
	if (shd->glslp.uni.frame_count >= 0) {
		if (fcountmod) {
			fcount %= fcountmod;
		}
		glUniform1i(shd->glslp.uni.frame_count, fcount);
	}
	if (shd->glslp.uni.frame_direction >= 0) {
		//glUniform1i(shd->glslp.uni.frame_direction, state_manager_frame_is_reversed() ? -1 : 1);
		glUniform1i(shd->glslp.uni.frame_direction, 1);
	}

	glBindBuffer(GL_ARRAY_BUFFER, shd->vbo);

	if (shd->glslp.uni.vertex_coord >= 0) {
		glEnableVertexAttribArray(shd->glslp.uni.vertex_coord);
		glVertexAttribPointer(shd->glslp.uni.vertex_coord, 2, GL_FLOAT, GL_FALSE,
				sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
		opengl.attribs.attrib[opengl.attribs.count++] = shd->glslp.uni.vertex_coord;
	}
	buffer_index += 2;

	{
		if (shd->glslp.uni.COLOR >= 0) {
			glEnableVertexAttribArray(shd->glslp.uni.COLOR);
			glVertexAttribPointer(shd->glslp.uni.COLOR, 4, GL_FLOAT, GL_FALSE,
					sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
			opengl.attribs.attrib[opengl.attribs.count++] = shd->glslp.uni.COLOR;
		}
		if (shd->glslp.uni.color >= 0) {
			glEnableVertexAttribArray(shd->glslp.uni.color);
			glVertexAttribPointer(shd->glslp.uni.color, 4, GL_FLOAT, GL_FALSE,
					sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
			opengl.attribs.attrib[opengl.attribs.count++] = shd->glslp.uni.color;
		}
	}
	buffer_index += 4;

	if (shd->glslp.uni.tex_coord >= 0) {
		glEnableVertexAttribArray(shd->glslp.uni.tex_coord);
		glVertexAttribPointer(shd->glslp.uni.tex_coord, 2, GL_FLOAT, GL_FALSE,
				sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
		opengl.attribs.attrib[opengl.attribs.count++] = shd->glslp.uni.tex_coord;
	}
	buffer_index += 2;

	if (shd->glslp.uni.lut_tex_coord >= 0) {
		glEnableVertexAttribArray(shd->glslp.uni.lut_tex_coord);
		glVertexAttribPointer(shd->glslp.uni.lut_tex_coord, 2, GL_FLOAT, GL_FALSE,
				sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
		opengl.attribs.attrib[opengl.attribs.count++] = shd->glslp.uni.lut_tex_coord;
	}
	buffer_index += 2;

	// lut
	for (i = 0; i < shader_effect.luts; i++) {
		if (shd->glslp.uni.lut[i] >= 0) {
			glActiveTexture(GL_TEXTURE0 + texture_index);
			glBindTexture(GL_TEXTURE_2D, opengl.lut[i].id);
			glUniform1i(shd->glslp.uni.lut[i], texture_index);
			texture_index++;
		}
	}

	// ORIG
	if (shd->glslp.uni.orig.texture >= 0) {
		glActiveTexture(GL_TEXTURE0 + texture_index);
		glBindTexture(GL_TEXTURE_2D, opengl.screen.tex[opengl.screen.index].id);
		glUniform1i(shd->glslp.uni.orig.texture, texture_index);
		texture_index++;
	}
	if (shd->glslp.uni.orig.input_size >= 0) {
		glUniform2fv(shd->glslp.uni.orig.input_size, 1,
				opengl.screen.tex[opengl.screen.index].shader.info.input_size);
	}
	if (shd->glslp.uni.orig.texture_size >= 0) {
		glUniform2fv(shd->glslp.uni.orig.texture_size, 1,
				opengl.screen.tex[opengl.screen.index].shader.info.texture_size);
	}
	if (shd->glslp.uni.orig.tex_coord >= 0) {
		glEnableVertexAttribArray(shd->glslp.uni.orig.tex_coord);
		glVertexAttribPointer(shd->glslp.uni.orig.tex_coord, 2, GL_FLOAT, GL_FALSE,
				sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
		opengl.attribs.attrib[opengl.attribs.count++] = shd->glslp.uni.orig.tex_coord;
	}
	// PREV (uso le stesse tex_coord di ORIG)
	{
		GLint circle_index = opengl.screen.index - 1;

		for (i = 0; i < (opengl.screen.in_use - 1); i++) {
			if (circle_index < 0) {
				circle_index = opengl.screen.in_use - 1;
			}
			if (shd->glslp.uni.prev[i].texture >= 0) {
				glActiveTexture(GL_TEXTURE0 + texture_index);
				glBindTexture(GL_TEXTURE_2D, opengl.screen.tex[circle_index].id);
				glUniform1i(shd->glslp.uni.prev[i].texture, texture_index);
				texture_index++;
			}
			if (shd->glslp.uni.prev[i].tex_coord >= 0) {
				glEnableVertexAttribArray(shd->glslp.uni.prev[i].tex_coord);
				glVertexAttribPointer(shd->glslp.uni.prev[i].tex_coord, 2, GL_FLOAT, GL_FALSE,
						sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
				opengl.attribs.attrib[opengl.attribs.count++] = shd->glslp.uni.prev[i].tex_coord;
			}
			circle_index--;
		}
	}
	buffer_index += 2;

	// FEEDBACK
	if (opengl.feedback.in_use) {
		if (shd->glslp.uni.feedback.texture >= 0) {
			glActiveTexture(GL_TEXTURE0 + texture_index);
			glBindTexture(GL_TEXTURE_2D, opengl.feedback.tex.id);
			glUniform1i(shd->glslp.uni.feedback.texture, texture_index);
			texture_index++;
		}
		if (shd->glslp.uni.feedback.input_size >= 0) {
			glUniform2fv(shd->glslp.uni.feedback.input_size, 1,
					opengl.texture[shader_effect.feedback_pass].shader.info.input_size);
		}
		if (shd->glslp.uni.feedback.texture_size >= 0) {
			glUniform2fv(shd->glslp.uni.feedback.texture_size, 1,
					opengl.texture[shader_effect.feedback_pass].shader.info.texture_size);
		}
		if (shd->glslp.uni.feedback.tex_coord >= 0) {
			glEnableVertexAttribArray(shd->glslp.uni.feedback.tex_coord);
			glVertexAttribPointer(shd->glslp.uni.feedback.tex_coord, 2, GL_FLOAT, GL_FALSE,
					sizeof(_vertex_buffer), BUFFER_OFFSET(sizeof(GLfloat) * buffer_index));
			opengl.attribs.attrib[opengl.attribs.count++] = shd->glslp.uni.feedback.tex_coord;
		}
	}
	buffer_index += 2;

	// PASSPREV
	for (i = 0; i < shader_effect.running_pass; i++) {
		GLuint next = i + 1;

		if (shd->glslp.uni.passprev[i].texture >= 0) {
			glActiveTexture(GL_TEXTURE0 + texture_index);
			glBindTexture(GL_TEXTURE_2D, opengl.texture[i].id);
			glUniform1i(shd->glslp.uni.passprev[i].texture, texture_index);
			texture_index++;
		}
		if (shd->glslp.uni.passprev[i].input_size >= 0) {
			glUniform2fv(shd->glslp.uni.passprev[i].input_size, 1,
					opengl.texture[next].shader.info.input_size);
		}
		if (shd->glslp.uni.passprev[i].texture_size >= 0) {
			glUniform2fv(shd->glslp.uni.passprev[i].texture_size, 1,
					opengl.texture[next].shader.info.texture_size);
		}
		if (shd->glslp.uni.passprev[i].tex_coord >= 0) {
			glEnableVertexAttribArray(shd->glslp.uni.passprev[i].tex_coord);
			glVertexAttribPointer(shd->glslp.uni.passprev[i].tex_coord, 2, GL_FLOAT, GL_FALSE,
					sizeof(_vertex_buffer),
					BUFFER_OFFSET(sizeof(GLfloat) * (buffer_index + (next * 2))));
			opengl.attribs.attrib[opengl.attribs.count++] = shd->glslp.uni.passprev[i].tex_coord;
		}
	}
	buffer_index += (MAX_PASS * 2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glActiveTexture(GL_TEXTURE0);

	// params
	for (i = 0; i < shader_effect.params; i++) {
		if (shd->glslp.uni.param[i] >= 0) {
			glUniform1fv(shd->glslp.uni.param[i], 1, &shader_effect.param[i].value);
		}
	}
}
INLINE static void opengl_shader_glsl_disable_attrib(void) {
	GLuint i;

	for (i = 0; i < opengl.attribs.count; i++) {
		glDisableVertexAttribArray(opengl.attribs.attrib[i]);
	}
	opengl.attribs.count = 0;
}
// cg
#if defined (WITH_OPENGL_CG)
#if defined (DEBUG)
static void opengl_shader_cg_error_handler(CGcontext ctx, CGerror error, void *data) {
	switch (error) {
		case CG_INVALID_PARAM_HANDLE_ERROR:
			fprintf(stderr, "OPENGLCG: Invalid param handle.\n");
			break;
		case CG_INVALID_PARAMETER_ERROR:
			fprintf(stderr, "OPENGLCG: Invalid parameter.\n");
			break;
		default:
			break;
	}

	fprintf(stderr, "OPENGLCG: \"%s\"\n", cgGetErrorString(error));
}
#endif
static BYTE opengl_shader_cg_init(GLuint pass, _shader *shd, GLchar *code, const GLchar *path) {
	const char *list;
	const char *argv[64];
	char alias[MAX_PASS][128];
	GLuint i, argc;

	memset(alias, 0x00, sizeof(alias));

	argc = 0;
	argv[argc++] = "-DPARAMETER_UNIFORM";
	for (i = 0; i < pass; i++) {
		_shader_pass *sp = &shader_effect.sp[i];

		if (sp->alias[0]) {
			snprintf(alias[i], 128, "-D%s_ALIAS", sp->alias);
			argv[argc++] = alias[i];
		}
	}
	argv[argc] = NULL;

	// fragment
	{
		if ((path == NULL) || !path[0]) {
			shd->cgp.prg.f = cgCreateProgram(opengl.cg.ctx, CG_SOURCE, code, opengl.cg.profile.f,
					"main_fragment", argv);
		} else {
			shd->cgp.prg.f = cgCreateProgramFromFile(opengl.cg.ctx, CG_SOURCE, path,
					opengl.cg.profile.f, "main_fragment", argv);
		}
		if (!shd->cgp.prg.f && (list = cgGetLastListing(opengl.cg.ctx))) {
			printf("OPENGLCG: fragment program errors :\n%s\n", list);
		}
	}

	// vertex
	{
		if ((path == NULL) || !path[0]) {
			shd->cgp.prg.v = cgCreateProgram(opengl.cg.ctx, CG_SOURCE, code, opengl.cg.profile.v,
					"main_vertex", argv);
		} else {
			shd->cgp.prg.v = cgCreateProgramFromFile(opengl.cg.ctx, CG_SOURCE, path,
					opengl.cg.profile.v, "main_vertex", argv);
		}
		if (!shd->cgp.prg.v && (list = cgGetLastListing(opengl.cg.ctx))) {
			printf("OPENGLCG: vertex program errors :\n%s\n", list);
		}
	}

	if (!shd->cgp.prg.f || !shd->cgp.prg.v) {
		fprintf(stderr, "OPENGLCG: %s\n", cgGetErrorString(cgGetError()));
		return (EXIT_ERROR_SHADER);
	}

	cgGLLoadProgram(shd->cgp.prg.f);
	cgGLLoadProgram(shd->cgp.prg.v);

	cgGLBindProgram(shd->cgp.prg.f);
	cgGLBindProgram(shd->cgp.prg.v);

	shd->cgp.uni.vertex = NULL;
	shd->cgp.uni.color = NULL;
	shd->cgp.uni.tex = NULL;
	shd->cgp.uni.lut_tex = NULL;

	shd->cgp.uni.mvp = cgGetNamedParameter(shd->cgp.prg.v, "modelViewProj");
	if (!shd->cgp.uni.mvp) {
		shd->cgp.uni.mvp = cgGetNamedParameter(shd->cgp.prg.v, "IN.mvp_matrix");
	}

	shd->cgp.uni.v.video_size = cgGetNamedParameter(shd->cgp.prg.v, "IN.video_size");
	shd->cgp.uni.f.video_size = cgGetNamedParameter(shd->cgp.prg.f, "IN.video_size");
	shd->cgp.uni.v.texture_size = cgGetNamedParameter(shd->cgp.prg.v, "IN.texture_size");
	shd->cgp.uni.f.texture_size = cgGetNamedParameter(shd->cgp.prg.f, "IN.texture_size");
	shd->cgp.uni.v.output_size = cgGetNamedParameter(shd->cgp.prg.v, "IN.output_size");
	shd->cgp.uni.f.output_size = cgGetNamedParameter(shd->cgp.prg.f, "IN.output_size");

	shd->cgp.uni.v.frame_count = cgGetNamedParameter(shd->cgp.prg.v, "IN.frame_count");
	shd->cgp.uni.f.frame_count = cgGetNamedParameter(shd->cgp.prg.f, "IN.frame_count");

	shd->cgp.uni.v.frame_direction = cgGetNamedParameter(shd->cgp.prg.v, "IN.frame_direction");
	shd->cgp.uni.f.frame_direction = cgGetNamedParameter(shd->cgp.prg.f, "IN.frame_direction");

	{
		CGparameter param = cgGetFirstParameter(shd->cgp.prg.v, CG_PROGRAM);

		for (; param; param = cgGetNextParameter(param)) {
			const char *semantic = NULL;

			if (cgGetParameterDirection(param) != CG_IN
					|| cgGetParameterVariability(param) != CG_VARYING) {
				continue;
			}

			if (!(semantic = cgGetParameterSemantic(param))) {
				continue;
			}

			fprintf(stderr, "OPENGLCG: Found semantic \"%s\" in prog.\n", semantic);

			if (strcmp(semantic, "POSITION") == 0) {
				opengl_shader_cg_clstate_ctrl(&shd->cgp.uni.vertex, &param, semantic);
				continue;
			}
			if (strcmp(semantic, "COLOR") == 0) {
				opengl_shader_cg_clstate_ctrl(&shd->cgp.uni.color, &param, semantic);
				continue;
			}
			if (strcmp(semantic, "COLOR0") == 0) {
				opengl_shader_cg_clstate_ctrl(&shd->cgp.uni.color, &param, semantic);
				continue;
			}
			if (strcmp(semantic, "TEXCOORD") == 0) {
				opengl_shader_cg_clstate_ctrl(&shd->cgp.uni.tex, &param, semantic);
				continue;
			}
			if (strcmp(semantic, "TEXCOORD0") == 0) {
				opengl_shader_cg_clstate_ctrl(&shd->cgp.uni.tex, &param, semantic);
				continue;
			}
			if (strcmp(semantic, "TEXCOORD1") == 0) {
				opengl_shader_cg_clstate_ctrl(&shd->cgp.uni.lut_tex, &param, semantic);
				continue;
			}
		}

		if (!shd->cgp.uni.vertex) {
			shd->cgp.uni.vertex = cgGetNamedParameter(shd->cgp.prg.v, "IN.vertex_coord");
		}
		if (!shd->cgp.uni.color) {
			shd->cgp.uni.color = cgGetNamedParameter(shd->cgp.prg.v, "IN.color");
		}
		if (!shd->cgp.uni.tex) {
			shd->cgp.uni.tex = cgGetNamedParameter(shd->cgp.prg.v, "IN.tex_coord");
		}
		if (!shd->cgp.uni.lut_tex) {
			shd->cgp.uni.lut_tex = cgGetNamedParameter(shd->cgp.prg.v, "IN.lut_tex_coord");
		}
	}

	for (i = 0; i < shader_effect.luts; i++) {
		shd->cgp.uni.v.lut[i] = cgGetNamedParameter(shd->cgp.prg.v, shader_effect.lp[i].name);
		shd->cgp.uni.f.lut[i] = cgGetNamedParameter(shd->cgp.prg.f, shader_effect.lp[i].name);
	}

	for (i = 0; i < shader_effect.params; i++) {
		shd->cgp.uni.v.param[i] = cgGetNamedParameter(shd->cgp.prg.v, shader_effect.param[i].name);
		shd->cgp.uni.f.param[i] = cgGetNamedParameter(shd->cgp.prg.f, shader_effect.param[i].name);
	}

	opengl_shader_cg_uni_texture_clear(&shd->cgp.uni.orig);
	opengl_shader_cg_uni_texture(&shd->cgp.uni.orig, &shd->cgp.prg, "ORIG");
	if (pass > 1) {
		opengl_shader_cg_uni_texture(&shd->cgp.uni.orig, &shd->cgp.prg, "PASSPREV%u", pass);
	}

	opengl_shader_cg_uni_texture_clear(&shd->cgp.uni.feedback);
	opengl_shader_cg_uni_texture(&shd->cgp.uni.feedback, &shd->cgp.prg, "FEEDBACK");

	for (i = 0; i < pass; i++) {
		opengl_shader_cg_uni_texture_clear(&shd->cgp.uni.passprev[i]);

		opengl_shader_cg_uni_texture(&shd->cgp.uni.passprev[i], &shd->cgp.prg, "PASS%u", i);
		opengl_shader_cg_uni_texture(&shd->cgp.uni.passprev[i], &shd->cgp.prg, "PASSPREV%u",
				pass - i);

		if (shader_effect.sp[i].alias[0]) {
			opengl_shader_cg_uni_texture(&shd->cgp.uni.passprev[i], &shd->cgp.prg,
					shader_effect.sp[i].alias);
		}
	}

	opengl_shader_cg_uni_texture_clear(&shd->cgp.uni.prev[0]);
	opengl_shader_cg_uni_texture(&shd->cgp.uni.prev[0], &shd->cgp.prg, "PREV");

	for (i = 1; i < LENGTH(shd->cgp.uni.prev); i++) {
		opengl_shader_cg_uni_texture_clear(&shd->cgp.uni.prev[i]);
		opengl_shader_cg_uni_texture(&shd->cgp.uni.prev[i], &shd->cgp.prg, "PREV%u", i);
	}

	return (EXIT_OK);
}
static void opengl_shader_cg_clstate_ctrl(CGparameter *dst, CGparameter *param,
        const char *semantic) {
	if (!(*param)) {
		return;
	}

	cgGLEnableClientState((*param));

	switch(cgGetError()) {
		case CG_NO_ERROR:
			(*dst) = (*param);
			cgGLDisableClientState((*param));
			break;
		default:
			(*dst) = NULL;
			fprintf(stderr, "OPENGLCG: Parameter \"%s\" disabled.\n", semantic);
			break;
	}
}
static void opengl_shader_cg_param2f_ctrl(CGparameter *dst, CGparameter *param,
        const char *semantic) {
	if (!(*param)) {
		return;
	}

	cgGLSetParameter2f((*param), 1.0f, 1.0f);

	switch(cgGetError()) {
		case CG_NO_ERROR:
			(*dst) = (*param);
			break;
		default:
			(*dst) = NULL;
			fprintf(stderr, "OPENGLCG: Parameter \"%s\" disabled.\n", semantic);
			break;
	}
}
static void opengl_shader_cg_uni_texture_clear(_shader_uniforms_tex_cg *sut) {
	sut->f.texture = NULL;
	sut->v.video_size = NULL;
	sut->f.video_size = NULL;
	sut->v.texture_size = NULL;
	sut->f.texture_size = NULL;
	sut->v.tex_coord = NULL;
}
static void opengl_shader_cg_uni_texture(_shader_uniforms_tex_cg *sut, _shader_prg_cg *prg,
		char *fmt, ...) {
	CGparameter param;
	char type[50], buff[50];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(type, sizeof(type), fmt, ap);
	va_end(ap);

	snprintf(buff, sizeof(buff), "%s%s", type, ".texture");
	if (!sut->f.texture) {
		sut->f.texture = cgGetNamedParameter(prg->f, buff);
	}
	snprintf(buff, sizeof(buff), "%s%s", type, ".video_size");
	if (!sut->v.video_size) {
		param = cgGetNamedParameter(prg->v, buff);
		opengl_shader_cg_param2f_ctrl(&sut->v.video_size, &param, buff);
	}
	if (!sut->f.video_size) {
		param = cgGetNamedParameter(prg->f, buff);
		opengl_shader_cg_param2f_ctrl(&sut->f.video_size, &param, buff);
	}
	snprintf(buff, sizeof(buff), "%s%s", type, ".texture_size");
	if (!sut->v.texture_size) {
		param = cgGetNamedParameter(prg->v, buff);
		opengl_shader_cg_param2f_ctrl(&sut->v.texture_size, &param, buff);
	}
	if (!sut->f.texture_size) {
		param = cgGetNamedParameter(prg->f, buff);
		opengl_shader_cg_param2f_ctrl(&sut->f.texture_size, &param, buff);
	}
	snprintf(buff, sizeof(buff), "%s%s", type, ".tex_coord");
	if (!sut->v.tex_coord) {
		param = cgGetNamedParameter(prg->v, buff);
		opengl_shader_cg_clstate_ctrl(&sut->v.tex_coord, &param, buff);
	}
}
INLINE static void opengl_shader_cg_params_set(const _texture *texture, GLuint fcountmod,
		GLuint fcount) {
	GLuint i, buffer_index = 0;
	const _shader *shd = &texture->shader;

	if (shd->cgp.uni.mvp) {
		cgGLSetMatrixParameterfc(shd->cgp.uni.mvp, (const float *) &opengl.mvp.data);
	}

	// IN.vertex_coord
	if (shd->cgp.uni.vertex) {
		cgGLSetParameterPointer(shd->cgp.uni.vertex, 2, GL_FLOAT, sizeof(_vertex_buffer),
				BUFFER_VB_OFFSET(shd->vb, sizeof(GLfloat) * buffer_index));
		cgGLEnableClientState(shd->cgp.uni.vertex);
		opengl.cg.states.state[opengl.cg.states.count++] = shd->cgp.uni.vertex;
	}
	buffer_index += 2;

	// IN.color
	if (shd->cgp.uni.color) {
		cgGLSetParameterPointer(shd->cgp.uni.color, 4, GL_FLOAT, sizeof(_vertex_buffer),
				BUFFER_VB_OFFSET(shd->vb, sizeof(GLfloat) * buffer_index));
		cgGLEnableClientState(shd->cgp.uni.color);
		opengl.cg.states.state[opengl.cg.states.count++] = shd->cgp.uni.color;
	}
	buffer_index += 4;

	// IN.tex_coord
	if (shd->cgp.uni.tex) {
		cgGLSetParameterPointer(shd->cgp.uni.tex, 2, GL_FLOAT, sizeof(_vertex_buffer),
				BUFFER_VB_OFFSET(shd->vb, sizeof(GLfloat) * buffer_index));
		cgGLEnableClientState(shd->cgp.uni.tex);
		opengl.cg.states.state[opengl.cg.states.count++] = shd->cgp.uni.tex;
	}
	buffer_index += 2;

	// IN.lut_tex_coord
	if (shd->cgp.uni.lut_tex) {
		cgGLSetParameterPointer(shd->cgp.uni.lut_tex, 2, GL_FLOAT, sizeof(_vertex_buffer),
				BUFFER_VB_OFFSET(shd->vb, sizeof(GLfloat) * buffer_index));
		cgGLEnableClientState(shd->cgp.uni.lut_tex);
		opengl.cg.states.state[opengl.cg.states.count++] = shd->cgp.uni.lut_tex;
	}
	buffer_index += 2;

	// IN
	{
		// IN.video_size
		if (shd->cgp.uni.v.video_size) {
			cgGLSetParameter2f(shd->cgp.uni.v.video_size, shd->info.input_size[0],
					shd->info.input_size[1]);
		}
		if (shd->cgp.uni.f.video_size) {
			cgGLSetParameter2f(shd->cgp.uni.f.video_size, shd->info.input_size[0],
					shd->info.input_size[1]);
		}
		// IN.texture_size
		if (shd->cgp.uni.v.texture_size) {
			cgGLSetParameter2f(shd->cgp.uni.v.texture_size, shd->info.texture_size[0],
					shd->info.texture_size[1]);
		}
		if (shd->cgp.uni.f.texture_size) {
			cgGLSetParameter2f(shd->cgp.uni.f.texture_size, shd->info.texture_size[0],
					shd->info.texture_size[1]);
		}
		// IN.output_size
		if (shd->cgp.uni.v.output_size) {
			cgGLSetParameter2f(shd->cgp.uni.v.output_size, shd->info.output_size[0],
					shd->info.output_size[1]);
		}
		if (shd->cgp.uni.f.output_size) {
			cgGLSetParameter2f(shd->cgp.uni.f.output_size, shd->info.output_size[0],
					shd->info.output_size[1]);
		}
		// IN.frame_count
		{
			GLfloat fc = (GLfloat) fcount;

			if (fcountmod) {
				fc = (GLfloat) (fcount % fcountmod);
			}

			if (shd->cgp.uni.v.frame_count) {
				cgGLSetParameter1f(shd->cgp.uni.v.frame_count, fc);
			}
			if (shd->cgp.uni.f.frame_count) {
				cgGLSetParameter1f(shd->cgp.uni.f.frame_count, fc);
			}
		}
		// IN.frame_direction
		{
			GLfloat frame_direction = -1;

			if (shd->cgp.uni.v.frame_direction) {
				cgGLSetParameter1f(shd->cgp.uni.v.frame_direction, frame_direction);
			}
			if (shd->cgp.uni.f.frame_direction) {
				cgGLSetParameter1f(shd->cgp.uni.f.frame_direction, frame_direction);
			}
		}
	}

	// params
	for (i = 0; i < shader_effect.params; i++) {
		if (shd->cgp.uni.f.param[i]) {
			cgGLSetParameter1f(shd->cgp.uni.f.param[i], shader_effect.param[i].value);
		}
		if (shd->cgp.uni.v.param[i]) {
			cgGLSetParameter1f(shd->cgp.uni.v.param[i], shader_effect.param[i].value);
		}
	}

	// lut
	for (i = 0; i < shader_effect.luts; i++) {
		if (shd->cgp.uni.f.lut[i]) {
			cgGLSetTextureParameter(shd->cgp.uni.f.lut[i], opengl.lut[i].id);
			cgGLEnableTextureParameter(shd->cgp.uni.f.lut[i]);
			opengl.cg.params.param[opengl.cg.params.count++] = shd->cgp.uni.f.lut[i];
		}
		if (shd->cgp.uni.v.lut[i]) {
			cgGLSetTextureParameter(shd->cgp.uni.v.lut[i], opengl.lut[i].id);
			cgGLEnableTextureParameter(shd->cgp.uni.v.lut[i]);
			opengl.cg.params.param[opengl.cg.params.count++] = shd->cgp.uni.v.lut[i];
		}
	}

	// ORIG
	{
		// ORIG.texture
		if (shd->cgp.uni.orig.f.texture) {
			cgGLSetTextureParameter(shd->cgp.uni.orig.f.texture,
					opengl.screen.tex[opengl.screen.index].id);
			cgGLEnableTextureParameter(shd->cgp.uni.orig.f.texture);
			opengl.cg.params.param[opengl.cg.params.count++] = shd->cgp.uni.orig.f.texture;
		}
		// ORIG.video_size
		if (shd->cgp.uni.orig.v.video_size) {
			cgGLSetParameter2f(shd->cgp.uni.orig.v.video_size,
					opengl.screen.tex[opengl.screen.index].shader.info.input_size[0],
					opengl.screen.tex[opengl.screen.index].shader.info.input_size[1]);
		}
		if (shd->cgp.uni.orig.f.video_size) {
			cgGLSetParameter2f(shd->cgp.uni.orig.f.video_size,
					opengl.screen.tex[opengl.screen.index].shader.info.input_size[0],
					opengl.screen.tex[opengl.screen.index].shader.info.input_size[1]);
		}
		// ORIG.texture_size
		if (shd->cgp.uni.orig.v.texture_size) {
			cgGLSetParameter2f(shd->cgp.uni.orig.v.texture_size,
					opengl.screen.tex[opengl.screen.index].shader.info.texture_size[0],
					opengl.screen.tex[opengl.screen.index].shader.info.texture_size[1]);
		}
		/**/
		if (shd->cgp.uni.orig.f.texture_size) {
			cgGLSetParameter2f(shd->cgp.uni.orig.f.texture_size,
					opengl.screen.tex[opengl.screen.index].shader.info.texture_size[0],
					opengl.screen.tex[opengl.screen.index].shader.info.texture_size[1]);
		}
		/**/
		// ORIG.tex_coord
		if (shd->cgp.uni.orig.v.tex_coord) {
			cgGLSetParameterPointer(shd->cgp.uni.orig.v.tex_coord, 2, GL_FLOAT,
					sizeof(_vertex_buffer),
					BUFFER_VB_OFFSET(shd->vb, sizeof(GLfloat) * buffer_index));
			cgGLEnableClientState(shd->cgp.uni.orig.v.tex_coord);
			opengl.cg.states.state[opengl.cg.states.count++] = shd->cgp.uni.orig.v.tex_coord;
		}
	}
	// PREV (uso le stesse tex_coord di ORIG)
	{
		GLint circle_index = opengl.screen.index - 1;

		for (i = 0; i < (opengl.screen.in_use - 1); i++) {
			if (circle_index < 0) {
				circle_index = opengl.screen.in_use - 1;
			}

			// PREV.texture
			if (shd->cgp.uni.prev[i].f.texture) {
				cgGLSetTextureParameter(shd->cgp.uni.prev[i].f.texture,
						opengl.screen.tex[circle_index].id);
				cgGLEnableTextureParameter(shd->cgp.uni.prev[i].f.texture);
				opengl.cg.params.param[opengl.cg.params.count++] = shd->cgp.uni.prev[i].f.texture;
			}
			// PREV.video_size
			if (shd->cgp.uni.prev[i].v.video_size) {
				cgGLSetParameter2f(shd->cgp.uni.prev[i].v.video_size,
						opengl.screen.tex[circle_index].shader.info.input_size[0],
						opengl.screen.tex[circle_index].shader.info.input_size[1]);
			}
			if (shd->cgp.uni.prev[i].f.video_size) {
				cgGLSetParameter2f(shd->cgp.uni.prev[i].f.video_size,
						opengl.screen.tex[circle_index].shader.info.input_size[0],
						opengl.screen.tex[circle_index].shader.info.input_size[1]);
			}
			// PREV.texture_size
			if (shd->cgp.uni.prev[i].v.texture_size) {
				cgGLSetParameter2f(shd->cgp.uni.prev[i].v.texture_size,
						opengl.screen.tex[circle_index].shader.info.texture_size[0],
						opengl.screen.tex[circle_index].shader.info.texture_size[1]);
			}
			if (shd->cgp.uni.prev[i].f.texture_size) {
				cgGLSetParameter2f(shd->cgp.uni.prev[i].f.texture_size,
						opengl.screen.tex[circle_index].shader.info.texture_size[0],
						opengl.screen.tex[circle_index].shader.info.texture_size[1]);
			}
			// PREV.tex_coord
			if (shd->cgp.uni.prev[i].v.tex_coord) {
				cgGLSetParameterPointer(shd->cgp.uni.prev[i].v.tex_coord, 2, GL_FLOAT,
						sizeof(_vertex_buffer),
						BUFFER_VB_OFFSET(shd->vb, sizeof(GLfloat) * buffer_index));
				cgGLEnableClientState(shd->cgp.uni.prev[i].v.tex_coord);
				opengl.cg.states.state[opengl.cg.states.count++] = shd->cgp.uni.prev[i].v.tex_coord;
			}
			circle_index--;
		}
	}
	buffer_index += 2;

	// FEEDBACK
	{
		// FEEDBACK.texture
		if (shd->cgp.uni.feedback.f.texture) {
			cgGLSetTextureParameter(shd->cgp.uni.feedback.f.texture, opengl.feedback.tex.id);
			cgGLEnableTextureParameter(shd->cgp.uni.feedback.f.texture);
			opengl.cg.params.param[opengl.cg.params.count++] = shd->cgp.uni.feedback.f.texture;
		}
		// FEEDBACK.video_size
		if (shd->cgp.uni.feedback.v.video_size) {
			cgGLSetParameter2f(shd->cgp.uni.feedback.v.video_size,
					opengl.texture[shader_effect.feedback_pass].shader.info.input_size[0],
					opengl.texture[shader_effect.feedback_pass].shader.info.input_size[1]);
		}
		if (shd->cgp.uni.feedback.f.video_size) {
			cgGLSetParameter2f(shd->cgp.uni.feedback.f.video_size,
					opengl.texture[shader_effect.feedback_pass].shader.info.input_size[0],
					opengl.texture[shader_effect.feedback_pass].shader.info.input_size[1]);
		}
		// FEEDBACK.texture_size
		if (shd->cgp.uni.feedback.v.texture_size) {
			cgGLSetParameter2f(shd->cgp.uni.feedback.v.texture_size,
					opengl.texture[shader_effect.feedback_pass].shader.info.texture_size[0],
					opengl.texture[shader_effect.feedback_pass].shader.info.texture_size[1]);
		}
		if (shd->cgp.uni.feedback.f.texture_size) {
			cgGLSetParameter2f(shd->cgp.uni.feedback.f.texture_size,
					opengl.texture[shader_effect.feedback_pass].shader.info.texture_size[0],
					opengl.texture[shader_effect.feedback_pass].shader.info.texture_size[1]);
		}
		// FEEDBACK.tex_coord
		if (shd->cgp.uni.feedback.v.tex_coord) {
			cgGLSetParameterPointer(shd->cgp.uni.feedback.v.tex_coord, 2, GL_FLOAT,
					sizeof(_vertex_buffer),
					BUFFER_VB_OFFSET(shd->vb, sizeof(GLfloat) * buffer_index));
			cgGLEnableClientState(shd->cgp.uni.feedback.v.tex_coord);
			opengl.cg.states.state[opengl.cg.states.count++] = shd->cgp.uni.feedback.v.tex_coord;
		}
	}
	buffer_index += 2;

	// PASSPREV
	for (i = 0; i < shader_effect.running_pass; i++) {
		GLuint next = i + 1;

		// PASSPREV[x].texture
		if (shd->cgp.uni.passprev[i].f.texture) {
			cgGLSetTextureParameter(shd->cgp.uni.passprev[i].f.texture, opengl.texture[i].id);
			cgGLEnableTextureParameter(shd->cgp.uni.passprev[i].f.texture);
			opengl.cg.params.param[opengl.cg.params.count++] = shd->cgp.uni.passprev[i].f.texture;
		}
		// PASSPREV[x].video_size
		if (shd->cgp.uni.passprev[i].v.video_size) {
			cgGLSetParameter2f(shd->cgp.uni.passprev[i].v.video_size,
					opengl.texture[next].shader.info.input_size[0],
					opengl.texture[next].shader.info.input_size[1]);
		}
		if (shd->cgp.uni.passprev[i].f.video_size) {
			cgGLSetParameter2f(shd->cgp.uni.passprev[i].f.video_size,
					opengl.texture[next].shader.info.input_size[0],
					opengl.texture[next].shader.info.input_size[1]);
		}
		// PASSPREV[x].texture_size
		if (shd->cgp.uni.passprev[i].v.texture_size) {
			cgGLSetParameter2f(shd->cgp.uni.passprev[i].v.texture_size,
					opengl.texture[next].shader.info.texture_size[0],
					opengl.texture[next].shader.info.texture_size[1]);
		}
		if (shd->cgp.uni.passprev[i].f.texture_size) {
			cgGLSetParameter2f(shd->cgp.uni.passprev[i].f.texture_size,
					opengl.texture[next].shader.info.texture_size[0],
					opengl.texture[next].shader.info.texture_size[1]);
		}
		// PASSPREV[x].tex_coord
		if (shd->cgp.uni.passprev[i].v.tex_coord) {
			cgGLSetParameterPointer(shd->cgp.uni.passprev[i].v.tex_coord, 2, GL_FLOAT,
					sizeof(_vertex_buffer),
					BUFFER_VB_OFFSET(shd->vb, sizeof(GLfloat) * (buffer_index + (next * 2))));
			cgGLEnableClientState(shd->cgp.uni.passprev[i].v.tex_coord);
			opengl.cg.states.state[opengl.cg.states.count++] = shd->cgp.uni.passprev[i].v.tex_coord;
		}
	}
	buffer_index += (MAX_PASS * 2);
}
INLINE static void opengl_shader_cg_disable_stpm(void) {
	GLuint i;

	for (i = 0; i < opengl.cg.states.count; i++) {
		cgGLDisableClientState(opengl.cg.states.state[i]);
	}
	opengl.cg.states.count = 0;

	for (i = 0; i < opengl.cg.params.count; i++) {
		cgGLDisableTextureParameter(opengl.cg.params.param[i]);
	}
	opengl.cg.params.count = 0;
}
#endif
