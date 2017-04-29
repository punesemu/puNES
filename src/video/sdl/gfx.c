/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

#include <unistd.h>
#include "info.h"
#include "emu.h"
#include "cpu.h"
#include "gfx.h"
#include "sdl_wid.h"
#include "overscan.h"
#include "clock.h"
#include "input.h"
#include "ppu.h"
#include "version.h"
#include "gui.h"
#include "text.h"
#include "palette.h"
#include "paldef.h"
#include "cgp.h"
#include "opengl.h"
#include "conf.h"
#include "vs_system.h"
#include "video/effects/pause.h"
#include "video/effects/tv_noise.h"
#if !defined (__WIN32__)
#include "gui/designer/pointers/target_32x32.xpm"
//#include "gui/designer/pointers/target_48x48.xpm"
#endif

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

static BYTE opengl_init(void);
static BYTE opengl_context_create(SDL_Surface *src);
static void opengl_context_delete(void);
static void opengl_draw_scene(SDL_Surface *surface);
static void opengl_screenshot(void);

static BYTE opengl_glew_init(void);
static BYTE opengl_texture_create(_texture *texture, GLuint index, GLuint clean);
static void opengl_texture_simple_create(_texture_simple *texture, GLuint w, GLuint h, BYTE text);
static BYTE opengl_texture_lut_create(_lut *lut, GLuint index);
static void opengl_shader_delete(_shader *shd);
#if !defined (RELEASE)
static void opengl_shader_print_log(GLuint obj, BYTE ret);
#endif
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
static BYTE opengl_shader_glsl_init(GLuint pass, _shader *shd, GLchar *code, const uTCHAR *path);
INLINE static void opengl_shader_glsl_params_set(const _shader *shd, GLuint fcountmod,
		GLuint fcount);
INLINE static void opengl_shader_glsl_disable_attrib(void);
// cg
#if defined (WITH_OPENGL_CG)
#if !defined (RELEASE)
static void opengl_shader_cg_error_handler(CGcontext ctx, CGerror error, void *data);
#endif
static BYTE opengl_shader_cg_init(GLuint pass, _shader *shd, GLchar *code, const uTCHAR *path);
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
#if !defined (__WIN32__)
static struct _cursor {
	SDL_Cursor *target;
	SDL_Cursor *org;
} cursor;
static SDL_Cursor *init_system_cursor(char *xpm[]);
#endif

BYTE gfx_init(void) {
	const SDL_VideoInfo *video_info;

	gfx.save_screenshot = FALSE;

	if (gui_create() == EXIT_ERROR) {
		fprintf(stderr, "gui initialization failed\n");
		return (EXIT_ERROR);
	}

#if defined (__WIN64__)
	sprintf(SDL_windowhack, "SDL_WINDOWID=%I64u", (uint64_t) gui_screen_id());
#else
	sprintf(SDL_windowhack, "SDL_WINDOWID=%i", (int) gui_screen_id());
#endif

	sdl_wid();

	// inizializzazione video SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
		return (EXIT_ERROR);
	}

	gui_after_set_video_mode();

	video_info = SDL_GetVideoInfo();

	// modalita' video con profondita' di colore
	// inferiori a 32 bits non sono supportate.
	if (video_info->vfmt->BitsPerPixel < 32) {
		fprintf(stderr, "Sorry but color depth less than 32 bits are not supported\n");
		return (EXIT_ERROR);
	}

	// per poter inizializzare il glew devo creare un contesto opengl prima
	if (!(surface_sdl = SDL_SetVideoMode(0, 0, 0, SDL_OPENGL))) {
		fprintf(stderr, "OpenGL not supported.\n");
		return (EXIT_ERROR);
	}

	// casi particolari provenienti dal settings_file_parse() e cmd_line_parse()
	if (cfg->fullscreen == FULLSCR) {
		gfx.scale_before_fscreen = cfg->scale;
	}

	if (opengl_init() == EXIT_ERROR) {
		fprintf(stderr, "OpenGL not supported.\n");
		return (EXIT_ERROR);
	}

	// inizializzo l'ntsc che utilizzero' non solo
	// come filtro ma anche nel gfx_set_screen() per
	// generare la paletta dei colori.
	if (ntsc_init(0, 0, 0, 0, 0) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	// mi alloco una zona di memoria dove conservare la
	// paletta nel formato di visualizzazione.
	if (!(gfx.palette = (uint32_t *) malloc(NUM_COLORS * sizeof(uint32_t)))) {
		fprintf(stderr, "Unable to allocate the palette\n");
		return (EXIT_ERROR);
	}

	if (pause_init() == EXIT_ERROR) {
		fprintf(stderr, "pause initialization failed\n");
		return (EXIT_ERROR);
	}

	if (tv_noise_init() == EXIT_ERROR) {
		fprintf(stderr, "tv_noise initialization failed\n");
		return (EXIT_ERROR);
	}

	if (cfg->fullscreen) {
		gfx_set_screen(cfg->scale, cfg->filter, cfg->shader, NO_FULLSCR, cfg->palette, FALSE, FALSE);
		cfg->fullscreen = NO_FULLSCR;
		cfg->scale = gfx.scale_before_fscreen;
		if (cfg->fullscreen_in_window) {
			gui_flush();
		}
		gui_fullscreen();
	} else {
		gfx_set_screen(cfg->scale, cfg->filter, cfg->shader, NO_FULLSCR, cfg->palette, FALSE, FALSE);
		// nella versione windows (non so in quella linux), sembra che
		// il VSync (con alcune schede video) non venga settato correttamente
		// al primo gfx_set_screen. E' necessario fare un gfx_reset_video
		// e poi nuovamente un gfx_set_screen. Nella versione linux il gui_reset_video()
		// non fa assolutamente nulla.
		gui_reset_video();
	}

	if (!surface_sdl) {
		fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
void gfx_set_screen(BYTE scale, DBWORD filter, DBWORD shader, BYTE fullscreen, BYTE palette,
		BYTE force_scale, BYTE force_palette) {
	BYTE set_mode;
	WORD width, height;
	DBWORD old_shader = cfg->shader;

	gfx_set_screen_start:
	set_mode = FALSE;
	width = 0, height = 0;

	// l'ordine dei vari controlli non deve essere cambiato:
	// 0) overscan
	// 1) filtro
	// 2) fullscreen
	// 3) fattore di scala
	// 4) tipo di paletta (IMPORTANTE: dopo il SDL_SetVideoMode)

	// overscan
	{
		overscan.enabled = cfg->oscan;

		gfx.rows = SCR_ROWS;
		gfx.lines = SCR_LINES;

		if (overscan.enabled == OSCAN_DEFAULT) {
			overscan.enabled = cfg->oscan_default;
		}
		if (overscan.enabled) {
			gfx.rows -= (overscan.borders->left + overscan.borders->right);
			gfx.lines -= (overscan.borders->up + overscan.borders->down);
		}
	}

	/* filtro */
	if (filter == NO_CHANGE) {
		filter = cfg->filter;
	}
	if ((filter != cfg->filter) || info.on_cfg || force_scale) {
		switch (filter) {
			default:
			case NO_FILTER:
				gfx.filter.func = scale_surface;
				gfx.filter.factor = X1;
				break;
			case SCALE2X:
			case SCALE3X:
			case SCALE4X:
				gfx.filter.func = scaleNx;
				gfx.filter.factor = filter + 1;
				break;
			case HQ2X:
			case HQ3X:
			case HQ4X:
				gfx.filter.func = hqNx;
				gfx.filter.factor = filter - 2;
				break;
			case XBRZ2X:
			case XBRZ3X:
			case XBRZ4X:
			case XBRZ5X:
			case XBRZ6X:
				gfx.filter.func = xBRZ;
				gfx.filter.factor = filter - 6;
				break;
			case XBRZ2XMT:
			case XBRZ3XMT:
			case XBRZ4XMT:
			case XBRZ5XMT:
			case XBRZ6XMT:
				gfx.filter.func = xBRZ_mt;
				gfx.filter.factor = filter - 11;
				break;
			case NTSC_FILTER:
				gfx.filter.func = ntsc_surface;
				gfx.filter.factor = X2;
				break;
		}
		// forzo il controllo del fattore di scale
		force_scale = TRUE;
		// indico che devo cambiare il video mode
		set_mode = TRUE;
	}

	/* shader */
	if (shader == NO_CHANGE) {
		shader = cfg->shader;
	}

	// fullscreen
	if (fullscreen == NO_CHANGE) {
		fullscreen = cfg->fullscreen;
	}
	if ((fullscreen != cfg->fullscreen) || info.on_cfg) {
		// forzo il controllo del fattore di scale
		force_scale = TRUE;
		// indico che devo cambiare il video mode
		set_mode = TRUE;
	}

	// fattore di scala
	if (scale == NO_CHANGE) {
		scale = cfg->scale;
	}
	if ((scale != cfg->scale) || info.on_cfg || force_scale) {
		if (filter == NTSC_FILTER) {
			width = gfx.w[PASS0] = gfx.w[NO_OVERSCAN] = NES_NTSC_OUT_WIDTH(SCR_ROWS);
			gfx.filter.width_pixel = (float) nes_ntsc_out_chunk / (float) nes_ntsc_in_chunk;
			if (overscan.enabled) {
				width -= ((float) (overscan.borders->left + overscan.borders->right) *
						gfx.filter.width_pixel);
			}
			switch (scale) {
				case X2:
					gfx.width_pixel = gfx.filter.width_pixel;
					break;
				default:
					width = ((float) width / 2.0f) * (float) scale;
					gfx.w[NO_OVERSCAN] = ((float) gfx.w[NO_OVERSCAN] / 2.0f) * (float) scale;
					gfx.width_pixel = (gfx.filter.width_pixel / 2.0f) * (float) scale;
					break;
			}
		} else {
			width = gfx.rows * scale;
			gfx.w[NO_OVERSCAN] = SCR_ROWS * scale;
			gfx.w[PASS0] = SCR_ROWS * gfx.filter.factor;
			gfx.filter.width_pixel = gfx.filter.factor;
			gfx.width_pixel = scale;
		}
		gfx.w[CURRENT] = width;

		height = gfx.lines * scale;
		gfx.h[CURRENT] = height;
		gfx.h[NO_OVERSCAN] = SCR_LINES * scale;
		gfx.h[PASS0] = SCR_LINES * gfx.filter.factor;

		set_mode = TRUE;
	}

	// cfg->scale e cfg->filter posso aggiornarli prima
	// del set_mode, mentre cfg->fullscreen e cfg->palette
	// devo farlo necessariamente dopo.
	// salvo il nuovo fattore di scala
	cfg->scale = scale;
	// salvo il nuovo filtro
	cfg->filter = filter;
	// salvo la nuova shader
	cfg->shader = shader;

	// devo eseguire un SDL_SetVideoMode?
	if (set_mode) {
		if (fullscreen) {
			gfx.w[VIDEO_MODE] = gfx.w[MONITOR];
			gfx.h[VIDEO_MODE] = gfx.h[MONITOR];
		} else if (cfg->oscan_black_borders) {
			gfx.w[VIDEO_MODE] = gfx.w[NO_OVERSCAN];
			gfx.h[VIDEO_MODE] = gfx.h[NO_OVERSCAN];
		} else {
			gfx.w[VIDEO_MODE] = width;
			gfx.h[VIDEO_MODE] = height;
		}

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

		//SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		//SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		//SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);

		// abilito il doublebuffering
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, TRUE);
		// abilito il vsync se e' necessario
		SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, cfg->vsync);

		// Pixel Aspect Ratio
		{
			gfx.pixel_aspect_ratio = 1.0f;

			if (cfg->filter == NTSC_FILTER) {
				gfx.pixel_aspect_ratio = 1.0f;
			} else {
				switch (cfg->pixel_aspect_ratio) {
					default:
					case PAR11:
						gfx.pixel_aspect_ratio = 1.0f;
						break;
					case PAR54:
						gfx.pixel_aspect_ratio = 5.0f / 4.0f;
						break;
					case PAR87:
						gfx.pixel_aspect_ratio = 8.0f / 7.0f;
						break;
					case PAR118:
						gfx.pixel_aspect_ratio = 2950000.0f / 2128137.0f;
						break;
				}
			}

			if ((gfx.pixel_aspect_ratio != 1.0f) && !fullscreen) {
				gfx.w[VIDEO_MODE] = (gfx.w[NO_OVERSCAN] * gfx.pixel_aspect_ratio);

				if (overscan.enabled && !cfg->oscan_black_borders) {
					float brd = 0;

					brd = (float) gfx.w[VIDEO_MODE] / (float) SCR_ROWS;
					brd *= (overscan.borders->right + overscan.borders->left);

					gfx.w[VIDEO_MODE] -= brd;
				}
			}
		}

		// faccio quello che serve prima del setvideo
		gui_set_video_mode();

		// inizializzo la superfice video
		surface_sdl = SDL_SetVideoMode(gfx.w[VIDEO_MODE], gfx.h[VIDEO_MODE], 0,
				SDL_HWSURFACE | SDL_OPENGL);

		gui_after_set_video_mode();

		// in caso di errore
		if (!surface_sdl) {
			fprintf(stderr, "SDL_SetVideoMode failed : %s\n", SDL_GetError());
			return;
		}

		gfx.bit_per_pixel = surface_sdl->format->BitsPerPixel;
	}

	// paletta
	if (palette == NO_CHANGE) {
		palette = cfg->palette;
	}
	if ((palette != cfg->palette) || info.on_cfg || force_palette) {
		if (palette == PALETTE_FILE) {
			if (ustrlen(cfg->palette_file) != 0) {
				if (palette_load_from_file(cfg->palette_file) == EXIT_ERROR) {
					umemset(cfg->palette_file, 0x00, usizeof(cfg->palette_file));
					text_add_line_info(1, "[red]error on palette file");
					if (cfg->palette != PALETTE_FILE) {
						palette = cfg->palette;
					} else if (machine.type == NTSC) {
						palette = PALETTE_NTSC;
					} else {
						palette = PALETTE_SONY;
					}
				} else {
					ntsc_set(NULL, cfg->ntsc_format, FALSE, (BYTE *) palette_base_file, 0,
							(BYTE *) palette_RGB);
				}
			}
		}

		switch (palette) {
			case PALETTE_PAL:
				ntsc_set(NULL, cfg->ntsc_format, FALSE, (BYTE *) palette_base_pal, 0,
						(BYTE *) palette_RGB);
				break;
			case PALETTE_NTSC:
				ntsc_set(NULL, cfg->ntsc_format, FALSE, 0, 0, (BYTE *) palette_RGB);
				break;
			case PALETTE_FRBX_NOSTALGIA:
				ntsc_set(NULL, cfg->ntsc_format, FALSE, (BYTE *) palette_firebrandx_nostalgia_FBX, 0,
						(BYTE *) palette_RGB);
				break;
			case PALETTE_FRBX_YUV:
				ntsc_set(NULL, cfg->ntsc_format, FALSE, (BYTE *) palette_firebrandx_YUV_v3, 0,
						(BYTE *) palette_RGB);
				break;
			case PALETTE_GREEN:
				rgb_modifier(NULL, palette_RGB, 0x00, -0x20, 0x20, -0x20);
				break;
			case PALETTE_FILE:
				break;
			default:
				ntsc_set(NULL, cfg->ntsc_format, palette, 0, 0, (BYTE *) palette_RGB);
				break;
		}

		if (vs_system.enabled) {
			switch (vs_system.ppu) {
				case RP2C03B:
				case RP2C03G:
					break;
				case RP2C04:
					ntsc_set(NULL, cfg->ntsc_format, FALSE, (BYTE *) palette_RP2C04_0001, 0,
					        (BYTE *) palette_RGB);
					break;
				case RP2C04_0002:
					ntsc_set(NULL, cfg->ntsc_format, FALSE, (BYTE *) palette_RP2C04_0002, 0,
					        (BYTE *) palette_RGB);
					break;
				case RP2C04_0003:
					ntsc_set(NULL, cfg->ntsc_format, FALSE, (BYTE *) palette_RP2C04_0003, 0,
					        (BYTE *) palette_RGB);
					break;
				case RP2C04_0004:
					ntsc_set(NULL, cfg->ntsc_format, FALSE, (BYTE *) palette_RP2C04_0004, 0,
					        (BYTE *) palette_RGB);
					break;
				case RC2C03B:
				case RC2C03C:
				case RC2C05_01:
				case RC2C05_02:
				case RC2C05_03:
				case RC2C05_04:
				case RC2C05_05:
				default:
					break;
			}
		}

		// inizializzo in ogni caso la tabella YUV dell'hqx
		hqx_init();

		//memorizzo i colori della paletta nel formato di visualizzazione
		{
			WORD i;

			for (i = 0; i < NUM_COLORS; i++) {
				gfx.palette[i] = gfx_color(255, palette_RGB[i].r, palette_RGB[i].g, palette_RGB[i].b);
			}
		}
	}

	// salvo il nuovo stato del fullscreen
	cfg->fullscreen = fullscreen;
	// salvo il nuovo tipo di paletta
	cfg->palette = palette;

	{
		gfx.PSS = ((cfg->pixel_aspect_ratio != PAR11) && cfg->PAR_soft_stretch) ? TRUE : FALSE;

		if (shaders_set(shader) == EXIT_ERROR) {
			umemcpy(cfg->shader_file, gfx.last_shader_file, usizeof(cfg->shader_file));
			if (old_shader == shader) {
				shader = NO_SHADER;
			} else {
				shader = old_shader;
			}
			goto gfx_set_screen_start;
		}

		// creo tutto il necessario per il rendering
		switch (opengl_context_create(surface_sdl)) {
			case EXIT_ERROR:
				fprintf(stderr, "OPENGL: Unable to initialize opengl context\n");
				break;
			case EXIT_ERROR_SHADER:
				text_add_line_info(1, "[red]errors[normal] on shader, use [green]'No shader'");
				fprintf(stderr, "OPENGL: Error on loading the shaders, switch to \"No shader\"\n");
				umemcpy(cfg->shader_file, gfx.last_shader_file, usizeof(cfg->shader_file));
				shader = NO_SHADER;
				goto gfx_set_screen_start;
		}
	}

	// gestione testo
	text.surface = surface_sdl;
	text.w = surface_sdl->w;
	text.h = surface_sdl->h;

	gfx_text_reset();

	// calcolo le proporzioni tra il disegnato a video (overscan e schermo
	// con le dimensioni per il filtro NTSC compresi) e quello che dovrebbe
	// essere (256 x 240). Mi serve per calcolarmi la posizione del puntatore
	// dello zapper.
	if (cfg->fullscreen) {
		gfx.w_pr = (float) gfx.vp.w / (float) SCR_ROWS;
		gfx.h_pr = (float) gfx.vp.h / (float) SCR_LINES;
	} else {
		gfx.w_pr = (float) (gfx.w[NO_OVERSCAN] * gfx.pixel_aspect_ratio) / (float) SCR_ROWS;
		gfx.h_pr = (float) gfx.h[NO_OVERSCAN] / (float) SCR_LINES;
	}

	// setto il titolo della finestra
	gui_update();

	if (info.on_cfg == TRUE) {
		info.on_cfg = FALSE;
	}
}
void gfx_draw_screen(BYTE forced) {
	void *palette = NULL;

	if (cfg->filter == NTSC_FILTER) {
		palette = NULL;
	} else {
		palette = (void *) gfx.palette;
	}

	if (!forced) {
		if (info.no_rom | info.turn_off) {
			if (cfg->filter == NTSC_FILTER) {
				palette = turn_off_effect.ntsc;
			} else {
				palette = (void *) turn_off_effect.palette;
			}

			if (++info.pause_frames_drawscreen == 2) {
				tv_noise_effect();
				info.pause_frames_drawscreen = 0;
				forced = TRUE;
			} else if (text.on_screen) {
				forced = TRUE;
			}
		} else if (info.pause) {
			if (!cfg->disable_sepia_color) {
				if (cfg->filter == NTSC_FILTER) {
					palette = pause_effect.ntsc;
				} else {
					palette = pause_effect.palette;
				}
			}

			if ((++info.pause_frames_drawscreen == 15) || text.on_screen) {
				info.pause_frames_drawscreen = 0;
				forced = TRUE;
			}
		}
	}

	// se il frameskip me lo permette (o se forzato), disegno lo screen
	if (forced || !ppu.skip_draw) {
		// applico l'effetto desiderato
		gfx.filter.func(screen.data,
				screen.line,
				palette,
				opengl.sdl.surface->pitch,
				opengl.sdl.surface->pixels,
				opengl.sdl.surface->w,
				opengl.sdl.surface->h);

		text_rendering(TRUE);

		opengl_draw_scene(opengl.sdl.surface);

		// disegno a video
		SDL_GL_SwapBuffers();

		// screenshot
		if (gfx.save_screenshot == TRUE) {
			opengl_screenshot();
			gfx.save_screenshot = FALSE;
		}
	}
}
void gfx_reset_video(void) {
	if (surface_sdl) {
		SDL_FreeSurface(surface_sdl);
		surface_sdl = NULL;
	}

	//sdl_wid();
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_InitSubSystem(SDL_INIT_VIDEO);

	gui_after_set_video_mode();
}
void gfx_quit(void) {
	if (gfx.palette) {
		free(gfx.palette);
		gfx.palette = NULL;
	}

	if (surface_sdl) {
		SDL_FreeSurface(surface_sdl);
	}

	pause_quit();
	tv_noise_quit();

	gfx_cursor_quit();

	opengl_context_delete();
	ntsc_quit();
	text_quit();
	SDL_Quit();
}

uint32_t gfx_color(BYTE a, BYTE r, BYTE g, BYTE b) {
	return (SDL_MapRGBA(surface_sdl->format, r, g, b, a));
}

void gfx_cursor_init(void) {
#if defined (__WIN32__)
	gui_cursor_init();
	gui_cursor_set();
#else
	memset(&cursor, 0x00, sizeof(cursor));

	cursor.org = SDL_GetCursor();

	if ((cursor.target = init_system_cursor(target_32x32_xpm)) == NULL) {
	//if ((cursor = init_system_cursor(target_48x48_xpm)) == NULL) {
		cursor.target = cursor.org;
		printf("SDL_Init failed: %s\n", SDL_GetError());
	}

	gfx_cursor_set();
#endif
}
void gfx_cursor_quit(void) {
#if defined (__WIN32__)
#else
	if (cursor.target) {
		SDL_FreeCursor(cursor.target);
	}
#endif
}
void gfx_cursor_set(void) {
#if defined (__WIN32__)
	gui_cursor_set();
#else
	if (input_zapper_is_connected() == TRUE) {
		SDL_SetCursor(cursor.target);
	} else {
		SDL_SetCursor(cursor.org);
	}
#endif
}
#if defined (__linux__)
void gfx_cursor_hide(BYTE hide) {
	if (hide == TRUE) {
		SDL_ShowCursor(SDL_DISABLE);
	} else {
		SDL_ShowCursor(SDL_ENABLE);
	}
}
#endif

void gfx_text_create_surface(_txt_element *ele) {
	ele->surface = gfx_create_RGB_surface(text.surface, ele->w, ele->h);
	ele->blank = gfx_create_RGB_surface(text.surface, ele->w, ele->h);
}
void gfx_text_release_surface(_txt_element *ele) {
	if (ele->surface) {
		SDL_FreeSurface(ele->surface);
		ele->surface = NULL;
	}
	if (ele->blank) {
		SDL_FreeSurface(ele->blank);
		ele->blank = NULL;
	}
}
void gfx_text_rect_fill(_txt_element *ele, _rect *rect, uint32_t color) {
	SDL_FillRect(ele->surface, rect, color);
}
void gfx_text_reset(void) {
	txt_table[TXT_NORMAL] = SDL_MapRGBA(text.surface->format, 0xFF, 0xFF, 0xFF, 0);
	txt_table[TXT_RED]    = SDL_MapRGBA(text.surface->format, 0xFF, 0x4C, 0x3E, 0);
	txt_table[TXT_YELLOW] = SDL_MapRGBA(text.surface->format, 0xFF, 0xFF, 0   , 0);
	txt_table[TXT_GREEN]  = SDL_MapRGBA(text.surface->format, 0   , 0xFF, 0   , 0);
	txt_table[TXT_CYAN]   = SDL_MapRGBA(text.surface->format, 0   , 0xFF, 0xFF, 0);
	txt_table[TXT_BROWN]  = SDL_MapRGBA(text.surface->format, 0xEB, 0x89, 0x31, 0);
	txt_table[TXT_BLUE]   = SDL_MapRGBA(text.surface->format, 0x2D, 0x8D, 0xBD, 0);
	txt_table[TXT_GRAY]   = SDL_MapRGBA(text.surface->format, 0xA0, 0xA0, 0xA0, 0);
	txt_table[TXT_BLACK]  = SDL_MapRGBA(text.surface->format, 0   , 0   , 0   , 0);
}
void gfx_text_clear(_txt_element *ele) {
	int x, y;

	if (!ele->blank) {
		return;
	}

	text_calculate_real_x_y(ele, &x, &y);

	glBindTexture(GL_TEXTURE_2D, opengl.text.id);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, ele->w);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, ele->w, ele->h, TI_FRM, TI_TYPE, ele->blank->pixels);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}
void gfx_text_blit(_txt_element *ele, _rect *rect) {
	if (!cfg->txt_on_screen) {
		return;
	}

	glBindTexture(GL_TEXTURE_2D, opengl.text.id);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, rect->w);
	glTexSubImage2D(GL_TEXTURE_2D, 0, rect->x, rect->y, rect->w, rect->h,
			TI_FRM, TI_TYPE, ele->surface->pixels);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

#if defined (__WIN32__)
#define __GFX_ALL_FUNC__
#include "gfx_functions_inline.h"
#undef __GFX_ALL_FUNC__

void gfx_sdlwe_set(int type, int arg) {
	sdlwe.event = type;
	sdlwe.arg = arg;
}
void gfx_sdlwe_tick(void) {
	if (sdlwe.event) {
		switch (sdlwe.event) {
			case SDLWIN_MAKE_RESET:
				gfx_MAKE_RESET(sdlwe.arg);
				break;
			case SDLWIN_CHANGE_ROM:
				gfx_CHANGE_ROM();
				break;
			case SDLWIN_SWITCH_MODE:
				gfx_SWITCH_MODE();
				break;
			case SDLWIN_FORCE_SCALE:
				gfx_FORCE_SCALE();
				break;
			case SDLWIN_SCALE:
				gfx_SCALE(sdlwe.arg);
				break;
			case SDLWIN_FILTER:
				gfx_FILTER(sdlwe.arg);
				break;
			case SDLWIN_VSYNC:
				gfx_VSYNC();
				break;
		}
		sdlwe.event = sdlwe.arg = SDLWIN_NONE;
	}
}
#endif

SDL_Surface *gfx_create_RGB_surface(SDL_Surface *src, uint32_t width, uint32_t height) {
	SDL_Surface *new_surface, *tmp;

	tmp = SDL_CreateRGBSurface(src->flags, width, height,
			src->format->BitsPerPixel, src->format->Rmask, src->format->Gmask,
			src->format->Bmask, src->format->Amask);

	new_surface = SDL_DisplayFormatAlpha(tmp);

	memset(new_surface->pixels, 0,
	        new_surface->w * new_surface->h * new_surface->format->BytesPerPixel);

	SDL_FreeSurface(tmp);

	return (new_surface);
}

double sdl_get_ms(void) {
	return (SDL_GetTicks());
}

#if !defined (__WIN32__)
static SDL_Cursor *init_system_cursor(char *xpm[]) {
	int srow, scol, ncol, none;
	int i, row, col;

	sscanf(xpm[0], "%d %d %d %d", &srow, &scol, &ncol, &none);

	Uint8 data[(scol / 8) * srow];
	Uint8 mask[(scol / 8) * srow];

	i = -1;
	for (row = 0; row < srow; ++row) {
		for (col = 0; col < scol; ++col) {
			if (col % 8) {
				data[i] <<= 1;
				mask[i] <<= 1;
			} else {
				++i;
				data[i] = mask[i] = 0;
			}
			switch (xpm[(ncol + 1) + row][col]) {
				case '+': // nero
					data[i] |= 0x01;
					mask[i] |= 0x01;
					break;
				case '.': // bianco
					mask[i] |= 0x01;
					break;
				case ' ':
					break;
			}
		}
	}
	//sscanf(xpm[(ncol + 1) + row], "%d,%d", &hot_x, &hot_y);
	return (SDL_CreateCursor(data, mask, srow, scol, srow / 2, scol / 2));
}
#endif

static BYTE opengl_init(void) {
	memset(&gfx.vp, 0x00, sizeof(gfx.vp));

	opengl.sdl.surface = NULL;

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
		return (EXIT_ERROR);
	}

	// Calculate projection
	opengl_matrix_4x4_ortho(&opengl.mvp, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

	return (EXIT_OK);
}
static BYTE opengl_context_create(SDL_Surface *src) {
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

#if !defined (RELEASE)
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

	w = gfx.w[PASS0];
	h = gfx.h[PASS0];

	opengl.sdl.surface = gfx_create_RGB_surface(src, w, h);

	// devo precalcolarmi il viewport finale
	{
		_viewport *vp = &gfx.vp;

		vp->x = 0;
		vp->y = 0;
		vp->w = src->w;
		vp->h = src->h;

		if (overscan.enabled && (!cfg->oscan_black_borders && !cfg->fullscreen)) {
			vp->x = (-overscan.borders->left * gfx.width_pixel) * gfx.pixel_aspect_ratio;
			vp->y = -overscan.borders->down * cfg->scale;
			vp->w = gfx.w[NO_OVERSCAN] * gfx.pixel_aspect_ratio;
			vp->h = gfx.h[NO_OVERSCAN];
		}

		// configuro l'aspect ratio del fullscreen
		if (cfg->fullscreen && !cfg->stretch) {
			float ratio_surface = (((float) SCR_ROWS * gfx.pixel_aspect_ratio) / (float) (SCR_LINES));
			float ratio_frame = (float) gfx.w[VIDEO_MODE] / (float) gfx.h[VIDEO_MODE];

			if (ratio_frame > ratio_surface) {
				vp->w = (int) ((float) src->h * ratio_surface);
				vp->x = (int) (((float) src->w - (float) vp->w) * 0.5f);
			} else {
				vp->h = (int) ((float) src->w / ratio_surface);
				vp->y = (int) (((float) src->h - (float) vp->h) * 0.5f);
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

	umemcpy(gfx.last_shader_file, cfg->shader_file, usizeof(gfx.last_shader_file));

 	return (EXIT_OK);
}
static void opengl_context_delete(void) {
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

	info.sRGB_FBO_in_use = FALSE;
}
static void opengl_draw_scene(SDL_Surface *surface) {
	static GLuint prev_type = MS_MEM;
	const _texture_simple *scrtex = &opengl.screen.tex[opengl.screen.index];
	GLuint offset_x = 0, offset_y = 0;
	GLuint w = surface->w, h = surface->h;
	GLuint i;

	// screen
	glBindTexture(GL_TEXTURE_2D, scrtex->id);

	if (overscan.enabled) {
		offset_x = overscan.borders->left * gfx.filter.width_pixel;
		offset_y = overscan.borders->up * gfx.filter.factor;
		w -= (overscan.borders->left + overscan.borders->right) * gfx.filter.width_pixel;
		h -= (overscan.borders->up + overscan.borders->down) * gfx.filter.factor;
	}
	glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->w);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, offset_x);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, offset_y);
	glTexSubImage2D(GL_TEXTURE_2D, 0, offset_x, offset_y, w, h, TI_FRM, TI_TYPE, surface->pixels);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	if (opengl.supported_fbo.srgb && !cfg->disable_srgb_fbo) {
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
			if (opengl.supported_fbo.srgb && !cfg->disable_srgb_fbo) {
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
static void opengl_screenshot(void) {
	char *buffer;

	glReadBuffer(GL_FRONT);
	if ((buffer = malloc(opengl.text.rect.w * opengl.text.rect.h * 4)) == NULL) {
		return;
	}
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, opengl.text.rect.w, opengl.text.rect.h, GL_BGRA, GL_UNSIGNED_BYTE, buffer);
	gui_save_screenshot(opengl.text.rect.w, opengl.text.rect.h, buffer, TRUE);
	free(buffer);
}

static BYTE opengl_glew_init(void) {
	GLenum err;

	glewExperimental = GL_TRUE;

	if ((err = glewInit()) != GLEW_OK) {
		fprintf(stderr, "OPENGL: %s\n", glewGetErrorString(err));
	} else {
		fprintf(stderr, "OPENGL: GPU %s (%s, %s)\n", glGetString(GL_RENDERER),
		        glGetString(GL_VENDOR), glGetString(GL_VERSION));
		fprintf(stderr, "OPENGL: GL Version %d.%d %s\n", opengl_integer_get(GL_MAJOR_VERSION),
		        opengl_integer_get(GL_MINOR_VERSION),
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

	if (sp->fbo_srgb && opengl.supported_fbo.srgb) {
		info.sRGB_FBO_in_use = TRUE;
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
	} else if (sp->fbo_srgb && opengl.supported_fbo.srgb && !cfg->disable_srgb_fbo) {
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
static BYTE opengl_shader_glsl_init(GLuint pass, _shader *shd, GLchar *code, const uTCHAR *path) {
	const GLchar *src[3];
	char alias_define[MAX_PASS * 128];
	GLuint i, vrt, frg;
	GLint success = 0;

	if ((code == NULL) && ((path == NULL) || !path[0])) {
		return (EXIT_ERROR_SHADER);
	}

	if (path && path[0]) {
		code = emu_file2string(path);
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
	opengl_shader_uni_texture(&shd->glslp.uni.orig, shd->glslp.prg, "PassPrev%u", pass + 1);

	opengl_shader_uni_texture_clear(&shd->glslp.uni.feedback);
	opengl_shader_uni_texture(&shd->glslp.uni.feedback, shd->glslp.prg, "Feedback");

	for (i = 0; i < pass; i++) {
		opengl_shader_uni_texture_clear(&shd->glslp.uni.passprev[i]);

		opengl_shader_uni_texture(&shd->glslp.uni.passprev[i], shd->glslp.prg, "Pass%u", i + 1);
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
#if !defined (RELEASE)
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
static BYTE opengl_shader_cg_init(GLuint pass, _shader *shd, GLchar *code, const uTCHAR *path) {
	const char *list;
	const char *argv[64];
	char alias[MAX_PASS][128];
	uTCHAR base[LENGTH_FILE_NAME_MID];
	uTCHAR dname[LENGTH_FILE_NAME_MID];
	uTCHAR bname[LENGTH_FILE_NAME_MID];
	GLuint i, argc;

	if ((path != NULL) && path[0]) {
		umemset(base, 0x00, usizeof(base));
		if (ugetcwd(base, usizeof(base)) == NULL) { ; };

		umemset(dname, 0x00, usizeof(dname));
		gui_utf_dirname((uTCHAR *) path, dname, usizeof(dname) - 1);

		umemset(bname, 0x00, usizeof(bname));
		gui_utf_basename((uTCHAR *) path, bname, usizeof(bname) - 1);
	}

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
			if (uchdir(dname) == -1) { ; }
			shd->cgp.prg.f = cgCreateProgramFromFile(opengl.cg.ctx, CG_SOURCE, (const char *) bname,
					opengl.cg.profile.f, "main_fragment", argv);

			if (uchdir(base) == -1) { ; }
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
			if (uchdir(dname)) { ; }
			shd->cgp.prg.v = cgCreateProgramFromFile(opengl.cg.ctx, CG_SOURCE, (const char *) bname,
					opengl.cg.profile.v, "main_vertex", argv);
			if (uchdir(base)) { ; }
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
	opengl_shader_cg_uni_texture(&shd->cgp.uni.orig, &shd->cgp.prg, "PASSPREV%u", pass + 1);

	opengl_shader_cg_uni_texture_clear(&shd->cgp.uni.feedback);
	opengl_shader_cg_uni_texture(&shd->cgp.uni.feedback, &shd->cgp.prg, "FEEDBACK");

	for (i = 0; i < pass; i++) {
		opengl_shader_cg_uni_texture_clear(&shd->cgp.uni.passprev[i]);

		opengl_shader_cg_uni_texture(&shd->cgp.uni.passprev[i], &shd->cgp.prg, "PASS%u", i + 1);
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
