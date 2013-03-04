/*
 * gfx.c
 *
 *  Created on: 07/apr/2010
 *      Author: fhorse
 */

#include "emu.h"
#include "cpu.h"
#include "gfx.h"
#include "overscan.h"
#include "clock.h"
#include "input.h"
#include "ppu.h"
#include "version.h"
#include "gui.h"
#include "text.h"
#define __STATICPAL__
#include "palette.h"
#include "opengl.h"
#include "cfg_file.h"

#if defined MINGW64
#define sdl_wid()\
	if (info.gui) {\
		char SDL_windowhack[50];\
		sprintf(SDL_windowhack,"SDL_WINDOWID=%I64u", (uint64_t) guiWindowID());\
		SDL_putenv(SDL_windowhack);\
	}
#else
#define sdl_wid()\
	if (info.gui) {\
		char SDL_windowhack[50];\
		sprintf(SDL_windowhack,"SDL_WINDOWID=%i", (int) guiWindowID());\
		SDL_putenv(SDL_windowhack);\
	}
#endif
#define ntsc_width(wdt, a, flag)\
{\
	wdt = 0;\
	if (filter == NTSC_FILTER) {\
		wdt = NES_NTSC_OUT_WIDTH(gfx.rows, a);\
		if (overscan.enabled) {\
			wdt -= (a - nes_ntsc_in_chunk);\
		}\
		if (flag) {\
			gfx.w[CURRENT] = wdt;\
			gfx.w[NO_OVERSCAN] = (NES_NTSC_OUT_WIDTH(SCR_ROWS, a));\
		}\
	}\
}
#define change_color(index, color, operation)\
	tmp = palette_RGB[index].color + operation;\
	palette_RGB[index].color = (tmp < 0 ? 0 : (tmp > 0xFF ? 0xFF : tmp))
#define rgb_modifier(red, green, blue)\
	/* prima ottengo la paletta monocromatica */\
	ntsc_set(cfg->ntsc_format, PALETTE_MONO, 0, 0, (BYTE *) palette_RGB);\
	/* quindi la modifico */\
	{\
		WORD i;\
		SWORD tmp;\
		for (i = 0; i < NUM_COLORS; i++) {\
			/* rosso */\
			change_color(i, r, red);\
			/* green */\
			change_color(i, g, green);\
			/* blue */\
			change_color(i, b, blue);\
		}\
	}\
	/* ed infine utilizzo la nuova */\
	ntsc_set(cfg->ntsc_format, FALSE, 0, (BYTE *) palette_RGB,(BYTE *) palette_RGB)

SDL_Surface *framebuffer;
uint32_t *palette_win, software_flags;
static BYTE ntsc_width_pixel[5] = {0, 0, 7, 10, 14};

BYTE gfx_init(void) {
	const SDL_VideoInfo *video_info;

	/* casi particolari provenienti dal cfg_file_parse() e cmd_line_parse() */
	if ((cfg->scale == X1) && (cfg->filter != NO_FILTER)) {
		cfg->scale = X2;
	}

	overscan.left = 8;
	overscan.right = 9;
	overscan.up = 8;
	overscan.down = 8;

	if (guiCreate()) {
		fprintf(stderr, "gui initialization failed\n");
		return (EXIT_ERROR);
	}

	sdl_wid();

	/* inizializzazione SDL */
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
		return (EXIT_ERROR);
	}

	video_info = SDL_GetVideoInfo();

	/*
	 * modalita' video con profondita' di colore
	 * inferiori a 15 bits non sono supportate.
	 */
	if (video_info->vfmt->BitsPerPixel < 15) {
		fprintf(stderr, "Sorry but video mode at 256 color are not supported\n");
		return (EXIT_ERROR);
	}

	/* il filtro hqx supporta solo i 32 bit di profondita' di colore */
	if (((cfg->filter >= HQ2X) || (cfg->filter <= HQ4X)) && (video_info->vfmt->BitsPerPixel < 32)) {
		cfg->filter = NO_FILTER;
	}

	/* controllo se e' disponibile l'accelerazione hardware */
	if (video_info->hw_available) {
		software_flags = SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_ASYNCBLIT;
	} else {
		software_flags = SDL_SWSURFACE | SDL_ASYNCBLIT;
	}

	/* per poter inizializzare il glew devo creare un contesto opengl prima */
	if (!(surface_sdl = SDL_SetVideoMode(0, 0, 0, SDL_OPENGL))) {
		opengl.supported = FALSE;

		cfg->render = RENDER_SOFTWARE;
		gfx_set_render(cfg->render);

		if ((cfg->filter >= POSPHOR) && (cfg->filter <= CRT_NO_CURVE)) {
			cfg->filter = NO_FILTER;
		}

		fprintf(stderr, "INFO: OpenGL not supported.\n");
	} else {
		opengl.supported = TRUE;
	}
	/* casi particolari provenienti dal cfg_file_parse() e cmd_line_parse()*/
	if (cfg->fullscreen == FULLSCR) {
		if (!gfx.opengl) {
			cfg->fullscreen = NO_FULLSCR;
		} else {
			gfx.scale_before_fscreen = cfg->scale;
		}
	}
	sdl_init_gl();

	/*
	 * inizializzo l'ntsc che utilizzero' non solo
	 * come filtro ma anche nel gfx_set_screen() per
	 * generare la paletta dei colori.
	 */
	if (ntsc_init(0, 0, 0, 0, 0)) {
		return (EXIT_ERROR);
	}

	/*
	 * mi alloco una zona di memoria dove conservare la
	 * paletta nel formato di visualizzazione.
	 */
	if (!(palette_win = malloc(NUM_COLORS * sizeof(uint32_t)))) {
		fprintf(stderr, "Out of memory\n");
		return (EXIT_ERROR);
	}

	if (cfg->fullscreen) {
		gfx_set_screen(cfg->scale, cfg->filter, NO_FULLSCR, cfg->palette, FALSE);
		cfg->fullscreen = NO_FULLSCR;
		cfg->scale = gfx.scale_before_fscreen;
		guiFullscreen();
	} else {
		gfx_set_screen(cfg->scale, cfg->filter, NO_FULLSCR, cfg->palette, FALSE);
	}

	if (!surface_sdl) {
		fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
void gfx_set_render(BYTE render) {
	switch (render) {
		case RENDER_SOFTWARE:
			gfx.opengl = FALSE;
			opengl.rotation = FALSE;
			opengl.glsl.enabled = FALSE;
			break;
		case RENDER_OPENGL:
			gfx.opengl = TRUE;
			opengl.rotation = TRUE;
			opengl.glsl.enabled = FALSE;
			break;
		case RENDER_GLSL:
			gfx.opengl = TRUE;
			opengl.rotation = TRUE;
			opengl.glsl.enabled = TRUE;
			break;
	}
}
void gfx_set_screen(BYTE scale, BYTE filter, BYTE fullscreen, BYTE palette, BYTE force_scale) {
	BYTE set_mode;
	WORD width, height, w_for_pr, h_for_pr;

	gfx_set_screen_start:
	set_mode = FALSE;
	width = 0, height = 0;
	w_for_pr = 0, h_for_pr = 0;

	/*
	 * l'ordine dei vari controlli non deve essere cambiato:
	 * 0) overscan
	 * 1) filtro
	 * 2) fullscreen
	 * 3) fattore di scala
	 * 4) tipo di paletta (IMPORTANTE: dopo il SDL_SetVideoMode)
	 */

	/* overscan */
	{
		overscan.enabled = cfg->oscan;

		gfx.rows = SCR_ROWS;
		gfx.lines = SCR_LINES;

		if (overscan.enabled == OSCAN_DEFAULT) {
			overscan.enabled = cfg->oscan_default;
		}

		if (overscan.enabled) {
			gfx.rows -= (overscan.left + overscan.right);
			gfx.lines -= (overscan.up + overscan.down);
		}
	}

	/* filtro */
	if (filter == NO_CHANGE) {
		filter = cfg->filter;
	}
	if ((filter != cfg->filter) || info.on_cfg) {
		switch (filter) {
			case POSPHOR:
			case SCANLINE:
			case DBL:
			case CRT_CURVE:
			case CRT_NO_CURVE:
			case NO_FILTER:
				effect = scale_surface;
				/*
				 * se sto passando dal filtro ntsc ad un'altro, devo
				 * ricalcolare la larghezza del video mode quindi
				 * forzo il controllo del fattore di scala.
				 */
				if (cfg->filter == NTSC_FILTER) {
					/* devo reimpostare la larghezza del video mode */
					scale = cfg->scale;
					/* forzo il controllo del fattore di scale */
					force_scale = TRUE;
					/* indico che devo cambiare il video mode */
					set_mode = TRUE;
				}
				break;
			case BILINEAR:
				effect = bilinear;
				/*
				 * se sto passando dal filtro ntsc ad un'altro, devo
				 * ricalcolare la larghezza del video mode quindi
				 * forzo il controllo del fattore di scala.
				 */
				if (cfg->filter == NTSC_FILTER) {
					/* forzo il controllo del fattore di scale */
					force_scale = TRUE;
					/* indico che devo cambiare il video mode */
					set_mode = TRUE;
				}
				break;
			case SCALE2X:
			case SCALE3X:
			case SCALE4X:
				effect = scaleNx;
				/*
				 * se sto passando dal filtro ntsc ad un'altro, devo
				 * ricalcolare la larghezza del video mode quindi
				 * forzo il controllo del fattore di scala.
				 */
				if (cfg->filter == NTSC_FILTER) {
					/* forzo il controllo del fattore di scale */
					force_scale = TRUE;
					/* indico che devo cambiare il video mode */
					set_mode = TRUE;
				}
				break;
			case HQ2X:
			case HQ3X:
			case HQ4X:
				effect = hqNx;
				/*
				 * se sto passando dal filtro ntsc ad un'altro, devo
				 * ricalcolare la larghezza del video mode quindi
				 * forzo il controllo del fattore di scala.
				 */
				if (cfg->filter == NTSC_FILTER) {
					/* forzo il controllo del fattore di scale */
					force_scale = TRUE;
					/* indico che devo cambiare il video mode */
					set_mode = TRUE;
				}
				break;
			case NTSC_FILTER:
				effect = ntsc_surface;
				/*
				 * il fattore di scala deve essere gia' stato
				 * inizializzato almeno una volta.
				 */
				if (cfg->scale != NO_CHANGE) {
					/* devo reimpostare la larghezza del video mode */
					scale = cfg->scale;
				} else if (scale == NO_CHANGE) {
					/*
					 * se scale e new_scale sono uguali a NO_CHANGE,
					 * imposto un default.
					 */
					scale = X2;
				}
				/* forzo il controllo del fattore di scale */
				force_scale = TRUE;
				/* indico che devo cambiare il video mode */
				set_mode = TRUE;
				break;
		}
	}

	/* fullscreen */
	if (fullscreen == NO_CHANGE) {
		fullscreen = cfg->fullscreen;
	}
	if ((fullscreen != cfg->fullscreen) || info.on_cfg) {
		/* forzo il controllo del fattore di scale */
		force_scale = TRUE;
		/* indico che devo cambiare il video mode */
		set_mode = TRUE;
	}

	/* fattore di scala */
	if (scale == NO_CHANGE) {
		scale = cfg->scale;
	}
	if ((scale != cfg->scale) || info.on_cfg || force_scale) {

#define ctrl_filter_scale(scalexf, hqxf)\
	if ((filter >= SCALE2X) && (filter <= SCALE4X)) {\
		filter = scalexf;\
	} else  if ((filter >= HQ2X) && (filter <= HQ4X)) {\
		filter = hqxf;\
	}

		switch (scale) {
			case X1:
				/*
				 * il fattore di scala a 1 e' possibile
				 * solo senza filtro.
				 */
				if (filter != NO_FILTER) {
					/*
					 * con un fatto redi scala X1 effect deve essere
					 * sempre impostato su scale_surface.
					 */
					effect = scale_surface;
					return;
				}
				set_mode = TRUE;
				break;
			case X2:
				ctrl_filter_scale(SCALE2X, HQ2X)
				ntsc_width(width, ntsc_width_pixel[scale], TRUE);
				set_mode = TRUE;
				break;
			case X3:
				ctrl_filter_scale(SCALE3X, HQ3X)
				ntsc_width(width, ntsc_width_pixel[scale], TRUE);
				set_mode = TRUE;
				break;
			case X4:
				ctrl_filter_scale(SCALE4X, HQ4X)
				ntsc_width(width, ntsc_width_pixel[scale], TRUE);
				set_mode = TRUE;
				break;
		}
		if (!width) {
			width = gfx.rows * scale;
			gfx.w[CURRENT] = width;
			gfx.w[NO_OVERSCAN] = SCR_ROWS * scale;
		}
		height = gfx.lines * scale;
		gfx.h[CURRENT] = height;
		gfx.h[NO_OVERSCAN] = SCR_LINES * scale;
	}

	/*
	 * se sono a schermo intero in modalita' opengl, non
	 * e' necessario fare un SDL_SetMode visto che
	 * uso una texture ridimensionabile.
	 */
	if (gfx.opengl && set_mode) {
		if ((cfg->fullscreen == FULLSCR) && (fullscreen == cfg->fullscreen)) {
			/* se e' il primo avvio, non devo mai disabilitare il set_mode */
			if (!info.on_cfg) {
				set_mode = FALSE;
			}
		}
	}

	/*
	 * cfg->scale e cfg->filter posso aggiornarli prima
	 * del set_mode, mentre cfg->fullscreen e cfg->palette
	 * devo farlo necessariamente dopo.
	 */
	/* salvo il nuovo fattore di scala */
	cfg->scale = scale;
	/* salvo ill nuovo filtro */
	cfg->filter = filter;

	/* devo eseguire un SDL_SetVideoMode? */
	if (set_mode) {
		uint32_t flags = software_flags;

		gfx.w[VIDEO_MODE] = width;
		gfx.h[VIDEO_MODE] = height;

		if (gfx.opengl) {
			flags = opengl.flags;

			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

			//SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			//SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			//SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);

			/* abilito il doublebuffering */
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, TRUE);
			/* abilito il vsync se e' necessario */
			SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, cfg->vsync);

			if (fullscreen) {
				gfx.w[VIDEO_MODE] = gfx.w[MONITOR];
				gfx.h[VIDEO_MODE] = gfx.h[MONITOR];
			}
		}

		/* faccio quello che serve prima del setvideo */
		guiSetVideoMode();

		/*
		 * nella versione a 32 bit (GTK) dopo un gfx_reset_video,
		 * se non lo faccio anche qui, crasha tutto.
		 */
		sdl_wid();

		/* inizializzo la superfice video */
		surface_sdl = SDL_SetVideoMode(gfx.w[VIDEO_MODE], gfx.h[VIDEO_MODE], 0, flags);

		/* in caso di errore */
		if (!surface_sdl) {
			fprintf(stderr, "SDL_SetVideoMode failed : %s\n", SDL_GetError());
			return;
		}

		gfx.bit_per_pixel = surface_sdl->format->BitsPerPixel;
	}

	/* paletta */
	if (palette == NO_CHANGE) {
		palette = cfg->palette;
	}
	if ((palette != cfg->palette) || info.on_cfg) {
		switch (palette) {
			case PALETTE_PAL:
				ntsc_set(cfg->ntsc_format, FALSE, (BYTE *) palette_base_pal, 0,
				        (BYTE *) palette_RGB);
				break;
			case PALETTE_NTSC:
				ntsc_set(cfg->ntsc_format, FALSE, 0, 0, (BYTE *) palette_RGB);
				break;
			case PALETTE_GREEN:
				rgb_modifier(-0x20, 0x20, -0x20);
				break;
			default:
				ntsc_set(cfg->ntsc_format, palette, 0, 0, (BYTE *) palette_RGB);
				break;
		}

		/* inizializzo in ogni caso la tabella YUV dell'hqx */
		hqx_init();

		/*
		 * memorizzo i colori della paletta nel
		 * formato di visualizzazione.
		 */
		{
			WORD i;

			for (i = 0; i < NUM_COLORS; i++) {
				palette_win[i] = SDL_MapRGBA(surface_sdl->format, palette_RGB[i].r,
						palette_RGB[i].g, palette_RGB[i].b, 255);
			}
		}
	}

	/* salvo il nuovo stato del fullscreen */
	cfg->fullscreen = fullscreen;
	/* salvo il nuovo tipo di paletta */
	cfg->palette = palette;

	/* software rendering */
	framebuffer = surface_sdl;
	flip = SDL_Flip;

	text.surface = surface_sdl;
	text_clear = sdl_text_clear;
	text_blit = sdl_text_blit;
	text.w = surface_sdl->w;
	text.h = surface_sdl->h;

	w_for_pr = gfx.w[VIDEO_MODE];
	h_for_pr = gfx.h[VIDEO_MODE];

	if (gfx.opengl) {
		BYTE use_txt_texture;

		opengl.scale_force = FALSE;
		opengl.scale = cfg->scale;
		opengl.factor = 1;
		opengl.glsl.shader_used = FALSE;
		shader.id = SHADER_NONE;
		opengl.effect = effect;
		opengl.interpolation = FALSE;
		use_txt_texture = FALSE;

		if (opengl.glsl.compliant && opengl.glsl.enabled) {

			glsl_delete_shaders(&shader);

			switch (cfg->filter) {
				case NO_FILTER:
					opengl.scale_force = TRUE;
					opengl.scale = X1;
					opengl.factor = cfg->scale;
					opengl.glsl.shader_used = TRUE;
					shader.id = SHADER_NO_FILTER;
					opengl.effect = scale_surface;
					use_txt_texture = TRUE;
					break;
				case BILINEAR:
					opengl.scale_force = TRUE;
					opengl.scale = X1;
					opengl.factor = cfg->scale;
					opengl.glsl.shader_used = TRUE;
					shader.id = SHADER_NO_FILTER;
					opengl.effect = scale_surface;
					opengl.interpolation = TRUE;
					use_txt_texture = TRUE;
					break;
				case POSPHOR:
					opengl.scale_force = TRUE;
					opengl.scale = X1;
					opengl.factor = cfg->scale;
					opengl.glsl.shader_used = TRUE;
					shader.id = SHADER_POSPHOR;
					opengl.effect = scale_surface;
					use_txt_texture = TRUE;
					break;
				case SCANLINE:
					opengl.scale_force = TRUE;
					opengl.scale = X1;
					opengl.factor = cfg->scale;
					opengl.glsl.shader_used = TRUE;
					shader.id = SHADER_SCANLINE;
					opengl.effect = scale_surface;
					use_txt_texture = TRUE;
					break;
				case DBL:
					opengl.scale_force = TRUE;
					opengl.scale = X1;
					opengl.factor = cfg->scale;
					opengl.glsl.shader_used = TRUE;
					shader.id = SHADER_DONTBLOOM;
					opengl.effect = scale_surface;
					use_txt_texture = TRUE;
					break;
				case CRT_CURVE:
					opengl.scale_force = TRUE;
					opengl.scale = X1;
					opengl.factor = cfg->scale;
					opengl.glsl.shader_used = TRUE;
					shader.id = SHADER_CRT;
					opengl.effect = scale_surface;
					use_txt_texture = TRUE;
					break;
				case CRT_NO_CURVE:
					opengl.scale_force = TRUE;
					opengl.scale = X1;
					opengl.factor = cfg->scale;
					opengl.glsl.shader_used = TRUE;
					shader.id = SHADER_CRT4;
					opengl.effect = scale_surface;
					use_txt_texture = TRUE;
					break;
				case SCALE2X:
					opengl.scale_force = TRUE;
					opengl.scale = X1;
					opengl.factor = cfg->scale;
					opengl.glsl.shader_used = TRUE;
					shader.id = SHADER_SCALE2X;
					opengl.effect = scale_surface;
					use_txt_texture = TRUE;
					break;
				case SCALE3X:
					opengl.scale_force = TRUE;
					opengl.scale = X1;
					opengl.factor = cfg->scale;
					opengl.glsl.shader_used = TRUE;
					shader.id = SHADER_SCALE3X;
					opengl.effect = scale_surface;
					use_txt_texture = TRUE;
					break;
				case SCALE4X:
					/*
					opengl.scale_force = TRUE;
					opengl.scale = X2;
					opengl.factor = 2;
					opengl.glsl.shader_used = TRUE;
					shader.id = SHADER_SCALE2X;
					opengl.effect = scaleNx;
					use_txt_texture = TRUE;
					*/
					opengl.scale_force = TRUE;
					opengl.scale = X1;
					opengl.factor = cfg->scale;
					opengl.glsl.shader_used = TRUE;
					shader.id = SHADER_SCALE4X;
					opengl.effect = scale_surface;
					use_txt_texture = TRUE;
					break;
				case HQ2X:
					opengl.scale_force = TRUE;
					opengl.scale = X1;
					opengl.factor = cfg->scale;
					opengl.glsl.shader_used = TRUE;
					shader.id = SHADER_HQ2X;
					opengl.effect = scale_surface;
					use_txt_texture = TRUE;
					break;
				case HQ4X:
					opengl.scale_force = TRUE;
					opengl.scale = X2;
					opengl.factor = 2;
					opengl.glsl.shader_used = TRUE;
					shader.id = SHADER_HQ2X;
					opengl.effect = hqNx;
					use_txt_texture = TRUE;
					break;
			}

			if (cfg->fullscreen) {
				if ((cfg->filter >= SCALE2X) && (cfg->filter <= SCALE4X)) {
					opengl.scale_force = TRUE;
					opengl.scale = X2;
					opengl.factor = (float) cfg->scale / 2.0;
					opengl.glsl.shader_used = TRUE;
					shader.id = SHADER_NO_FILTER;
					opengl.effect = scaleNx;
					use_txt_texture = TRUE;
				} else if ((cfg->filter >= HQ2X) && (cfg->filter <= HQ4X)) {
					opengl.scale_force = TRUE;
					opengl.scale = X2;
					opengl.factor = (float) cfg->scale / 2.0;
					opengl.glsl.shader_used = TRUE;
					shader.id = SHADER_NO_FILTER;
					opengl.effect = hqNx;
					use_txt_texture = TRUE;
				}
			}
		}

		/* creo la superficie che utilizzero' come texture */
		sdl_create_surface_gl(surface_sdl, gfx.w[CURRENT], gfx.h[CURRENT], cfg->fullscreen);

		/* opengl rendering */
		framebuffer = opengl.surface_gl;
		flip = opengl_flip;

		if (use_txt_texture) {
			text.surface = surface_sdl;
			text_clear = opengl_text_clear;
			text_blit = opengl_text_blit;
		} else {
			text.surface = opengl.surface_gl;
			text_clear = sdl_text_clear;
			text_blit = sdl_text_blit;
 		}
		text.w = gfx.w[CURRENT];
		text.h = gfx.h[CURRENT];

		w_for_pr = opengl.x_texture2 - opengl.x_texture1;
		h_for_pr = opengl.y_texture2 - opengl.y_texture1;
	}

	/* questo controllo devo farlo necessariamente dopo il glew_init() */
	if (!opengl.glsl.compliant || !opengl.glsl.enabled) {
		if ((filter >= POSPHOR) && (filter <= CRT_NO_CURVE)) {
			filter = NO_FILTER;
			goto gfx_set_screen_start;
		}
	}

	text_reset();

	/*
	 * calcolo le proporzioni tra il disegnato a video (overscan e schermo
	 * con le dimensioni per il filtro NTSC compresi) e quello che dovrebbe
	 * essere (256 x 240). Mi serve per calcolarmi la posizione del puntatore
	 * dello zapper.
	 */
	gfx.w_pr = ((float) w_for_pr / gfx.w[CURRENT]) * ((float) gfx.w[NO_OVERSCAN] / SCR_ROWS);
	gfx.h_pr = ((float) h_for_pr / gfx.h[CURRENT]) * ((float) gfx.h[NO_OVERSCAN] / SCR_LINES);

	/* setto il titolo della finestra */
	guiUpdate();

	if (info.on_cfg == TRUE) {
		info.on_cfg = FALSE;
	}
}
void gfx_draw_screen(BYTE forced) {
	if (!forced && (info.no_rom || info.pause)) {
		if (++info.pause_frames_drawscreen == 4) {
			info.pause_frames_drawscreen = 0;
			forced = TRUE;
		} else {
			text_rendering(FALSE);
			return;
		}
	}

	/* se il frameskip me lo permette (o se forzato), disegno lo screen */
	if (forced || !ppu.skip_draw) {
		/* applico l'effetto desiderato */
		if (gfx.opengl) {
			opengl.effect(screen.data, screen.line, palette_win, framebuffer, gfx.rows,
					gfx.lines, opengl.scale);

			text_rendering(TRUE);

			opengl_draw_scene(framebuffer);
		} else {
			effect(screen.data, screen.line, palette_win, framebuffer, gfx.rows, gfx.lines,
					cfg->scale);

			text_rendering(TRUE);
		}

		/* disegno a video */
		flip(framebuffer);
	}
}
void gfx_reset_video(void) {
	SDL_FreeSurface(surface_sdl);
	surface_sdl = framebuffer = NULL;

	if (opengl.surface_gl) {
		SDL_FreeSurface(opengl.surface_gl);
	}
	if (opengl.texture.data) {
		glDeleteTextures(1, &opengl.texture.data);
	}
	opengl.surface_gl = NULL;

	sdl_wid();
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_InitSubSystem(SDL_INIT_VIDEO);
}
void gfx_quit(void) {
	if (palette_win) {
		free(palette_win);
	}

	sdl_quit_gl();
	ntsc_quit();
	text_quit();
	SDL_Quit();
}

SDL_Surface *gfx_create_RGB_surface(SDL_Surface *src, uint32_t width, uint32_t height) {
	SDL_Surface *new_surface;

	new_surface = SDL_DisplayFormatAlpha(SDL_CreateRGBSurface(src->flags, width, height,
			src->format->BitsPerPixel, src->format->Rmask, src->format->Gmask,
			src->format->Bmask, src->format->Amask));

	memset(new_surface->pixels, 0,
	        new_surface->w * new_surface->h * new_surface->format->BytesPerPixel);

	return (new_surface);
}

double sdl_get_ms(void) {
	return (SDL_GetTicks());
}
void sdl_nop(double ms) {
	SDL_Delay(ms);
}
