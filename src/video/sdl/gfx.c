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
#include "opengl.h"
#include "conf.h"
#include "vs_system.h"
#include "video/effects/pause.h"
#include "video/effects/tv_noise.h"
#if !defined (__WIN32__)
#include "gui/designer/pointers/target_32x32.xpm"
//#include "gui/designer/pointers/target_48x48.xpm"
#endif

#define ntsc_width(wdt, a, flag)\
{\
	wdt = 0;\
	if (filter == NTSC_FILTER) {\
		wdt = NES_NTSC_OUT_WIDTH(SCR_ROWS, a);\
		if (overscan.enabled) {\
			wdt -= (a / nes_ntsc_in_chunk) * (overscan.borders->left + overscan.borders->right);\
		}\
		if (flag) {\
			gfx.w[CURRENT] = wdt;\
			gfx.w[NO_OVERSCAN] = (NES_NTSC_OUT_WIDTH(SCR_ROWS, a));\
		}\
	}\
}

SDL_Surface *framebuffer;
uint32_t software_flags;
static BYTE ntsc_width_pixel[5] = {0, 0, 7, 10, 14};

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

	// casi particolari provenienti dal settings_file_parse() e cmd_line_parse()
	if ((cfg->scale == X1) && (cfg->filter != NO_FILTER)) {
		cfg->scale = X2;
	}

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

	// controllo se e' disponibile l'accelerazione hardware
	if (video_info->hw_available) {
		software_flags = SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_ASYNCBLIT;
	} else {
		software_flags = SDL_SWSURFACE | SDL_ASYNCBLIT;
	}

	// per poter inizializzare il glew devo creare un contesto opengl prima
	if (!(surface_sdl = SDL_SetVideoMode(0, 0, 0, SDL_OPENGL))) {
		opengl.supported = FALSE;

		cfg->render = RENDER_SOFTWARE;
		gfx_set_render(cfg->render);

		if ((cfg->filter >= FLTSHDSTART) && (cfg->filter <= FLTSHDSTOP)) {
			cfg->filter = NO_FILTER;
		}

		fprintf(stderr, "INFO: OpenGL not supported.\n");
	} else {
		opengl.supported = TRUE;
	}

	// casi particolari provenienti dal settings_file_parse() e cmd_line_parse()
	if (cfg->fullscreen == FULLSCR) {
		if (!gfx.opengl) {
			cfg->fullscreen = NO_FULLSCR;
		} else {
			gfx.scale_before_fscreen = cfg->scale;
		}
	}
	opengl_init();

	if (opengl.supported == FALSE) {
		cfg->render = RENDER_SOFTWARE;
		gfx_set_render(cfg->render);

		if ((cfg->filter >= FLTSHDSTART) && (cfg->filter <= FLTSHDSTOP)) {
			cfg->filter = NO_FILTER;
		}
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
		gfx_set_screen(cfg->scale, cfg->filter, NO_FULLSCR, cfg->palette, FALSE, FALSE);
		cfg->fullscreen = NO_FULLSCR;
		cfg->scale = gfx.scale_before_fscreen;
		gui_fullscreen();
	} else {
		gfx_set_screen(cfg->scale, cfg->filter, NO_FULLSCR, cfg->palette, FALSE, FALSE);
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
void gfx_set_render(BYTE render) {
	switch (render) {
		case RENDER_SOFTWARE:
			gfx.opengl = FALSE;
			break;
		case RENDER_GLSL:
			gfx.opengl = TRUE;
			break;
	}
}
void gfx_set_screen(BYTE scale, DBWORD filter, BYTE fullscreen, BYTE palette, BYTE force_scale,
        BYTE force_palette) {
	BYTE set_mode;
	WORD width, height, w_for_pr, h_for_pr;
	DBWORD old_filter = cfg->filter;

	gfx_set_screen_start:
	set_mode = FALSE;
	width = 0, height = 0;
	w_for_pr = 0, h_for_pr = 0;

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
			case NO_FILTER:
			default:
				gfx.filter = scale_surface;
				// se sto passando dal filtro ntsc ad un'altro, devo
				// ricalcolare la larghezza del video mode quindi
				// forzo il controllo del fattore di scala.
				if (cfg->filter == NTSC_FILTER) {
					// devo reimpostare la larghezza del video mode
					scale = cfg->scale;
					// forzo il controllo del fattore di scale
					force_scale = TRUE;
					// indico che devo cambiare il video mode
					set_mode = TRUE;
				}
				break;
			case SCALE2X:
			case SCALE3X:
			case SCALE4X:
			case HQ2X:
			case HQ3X:
			case HQ4X:
			case XBRZ2X:
			case XBRZ3X:
			case XBRZ4X:
			case XBRZ5X:
			case XBRZ6X:
				if ((filter >= SCALE2X) && (filter <= SCALE4X)) {
					gfx.filter = scaleNx;
				} else  if ((filter >= HQ2X) && (filter <= HQ4X)) {
					gfx.filter = hqNx;
				} else  if ((filter >= XBRZ2X) && (filter <= XBRZ6X)) {
					gfx.filter = xBRZ;
				}
				// se sto passando dal filtro ntsc ad un'altro, devo
				// ricalcolare la larghezza del video mode quindi
				// forzo il controllo del fattore di scala.
				if (cfg->filter == NTSC_FILTER) {
					// forzo il controllo del fattore di scale
					force_scale = TRUE;
					// indico che devo cambiare il video mode
					set_mode = TRUE;
				}
				break;
			case NTSC_FILTER:
				gfx.filter = ntsc_surface;
				// il fattore di scala deve essere gia' stato inizializzato almeno una volta
				if (cfg->scale != NO_CHANGE) {
					// devo reimpostare la larghezza del video mode
					scale = cfg->scale;
				} else if (scale == NO_CHANGE) {
					// se scale e new_scale sono uguali a NO_CHANGE, imposto un default
					scale = X2;
				}
				// forzo il controllo del fattore di scale
				force_scale = TRUE;
				// indico che devo cambiare il video mode
				set_mode = TRUE;
				break;
		}
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

#define ctrl_software_filter_scale(scalexf, hqxf, xbrzxf, ntsc)\
	if ((filter >= SCALE2X) && (filter <= SCALE4X)) {\
		filter = scalexf;\
	} else if ((filter >= HQ2X) && (filter <= HQ4X)) {\
		filter = hqxf;\
	} else if ((filter >= XBRZ2X) && (filter <= XBRZ6X)) {\
		filter = xbrzxf;\
	} else if (filter == NTSC_FILTER) {\
		filter = ntsc;\
	}

		switch (scale) {
			case X1:
				// il fattore di scala a 1 e' possibile  solo senza filtro
				if (filter != NO_FILTER) {
					// con un fattore di scala X1 effect deve essere
					// sempre impostato su scale_surface.
					gfx.filter = scale_surface;
					return;
				}
				set_mode = TRUE;
				break;
			case X2:
				ctrl_software_filter_scale(SCALE2X, HQ2X, XBRZ2X, NTSC_FILTER)
				ntsc_width(width, ntsc_width_pixel[scale], TRUE);
				set_mode = TRUE;
				break;
			case X3:
				ctrl_software_filter_scale(SCALE3X, HQ3X, XBRZ3X, NTSC_FILTER)
				ntsc_width(width, ntsc_width_pixel[scale], TRUE);
				set_mode = TRUE;
				break;
			case X4:
				ctrl_software_filter_scale(SCALE4X, HQ4X, XBRZ4X, NTSC_FILTER)
				ntsc_width(width, ntsc_width_pixel[scale], TRUE);
				set_mode = TRUE;
				break;
			case X5:
				ctrl_software_filter_scale(NO_FILTER, NO_FILTER, XBRZ5X, NO_FILTER)
				ntsc_width(width, ntsc_width_pixel[scale], TRUE);
				set_mode = TRUE;
				break;
			case X6:
				ctrl_software_filter_scale(NO_FILTER, NO_FILTER, XBRZ6X, NO_FILTER)
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

	// cfg->scale e cfg->filter posso aggiornarli prima
	// del set_mode, mentre cfg->fullscreen e cfg->palette
	// devo farlo necessariamente dopo.
	// salvo il nuovo fattore di scala
	cfg->scale = scale;
	// salvo ill nuovo filtro
	cfg->filter = filter;

	// devo eseguire un SDL_SetVideoMode?
	if (set_mode) {
		uint32_t flags = software_flags;

		gfx.w[VIDEO_MODE] = width;
		gfx.h[VIDEO_MODE] = height;

		gfx.pixel_aspect_ratio = 1.0f;

		if (gfx.opengl) {
			flags = opengl.sdl.flags;

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

			if (fullscreen) {
				gfx.w[VIDEO_MODE] = gfx.w[MONITOR];
				gfx.h[VIDEO_MODE] = gfx.h[MONITOR];
			}

			// Pixel Aspect Ratio
			{
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
					float brd = 0;

					gfx.w[VIDEO_MODE] = (gfx.w[NO_OVERSCAN] * gfx.pixel_aspect_ratio);

					if (overscan.enabled) {
						brd = (float) gfx.w[VIDEO_MODE] / (float) SCR_ROWS;
						brd *= (overscan.borders->right + overscan.borders->left);
					}

					gfx.w[VIDEO_MODE] -= brd;
				}
			}
		}

		// faccio quello che serve prima del setvideo
		gui_set_video_mode();

		// nella versione a 32 bit (GTK) dopo un gfx_reset_video,
		// se non lo faccio anche qui, crasha tutto.
		//sdl_wid();

		// inizializzo la superfice video
		surface_sdl = SDL_SetVideoMode(gfx.w[VIDEO_MODE], gfx.h[VIDEO_MODE], 0, flags);

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
			if (strlen(cfg->palette_file) != 0) {
				if (palette_load_from_file(cfg->palette_file) == EXIT_ERROR) {
					memset(cfg->palette_file, 0x00, sizeof(cfg->palette_file));
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
			case PALETTE_FRBX_UNSATURED:
				ntsc_set(NULL, cfg->ntsc_format, FALSE, (BYTE *) palette_firebrandx_unsaturated_v6, 0,
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

	// software rendering
	framebuffer = surface_sdl;
	flip = SDL_Flip;

	text.surface = surface_sdl;
	text_clear = gfx_text_clear;
	text_blit = gfx_text_blit;
	text.w = surface_sdl->w;
	text.h = surface_sdl->h;

	w_for_pr = gfx.w[VIDEO_MODE];
	h_for_pr = gfx.h[VIDEO_MODE];

	if (gfx.opengl) {
		DBWORD f = NO_FILTER;
		opengl.scale = cfg->scale;
		gfx.PSS = ((cfg->pixel_aspect_ratio != PAR11) && cfg->PAR_soft_stretch) ? TRUE : FALSE;

		if ((filter == NO_FILTER) || (filter >= FLTSHDSTART)) {
			opengl.scale = X1;
			gfx.filter = scale_surface;
			f = filter;
		}

		if (shaders_set(f) == EXIT_ERROR) {
			memcpy(cfg->shader_file, gfx.last_shader_file, sizeof(cfg->shader_file));
			if (old_filter == filter) {
				filter = NO_FILTER;
			} else {
				filter = old_filter;
			}
			goto gfx_set_screen_start;
		}

		// creo tutto il necessario per il rendering
		switch (opengl_context_create(surface_sdl)) {
			case EXIT_ERROR:
				fprintf(stderr, "OPENGL: Unable to initialize opengl context\n");
				gfx.opengl = FALSE;
				break;
			case EXIT_ERROR_SHADER:
				text_add_line_info(1, "[red]errors[normal] on shader, use [green]'No filter'");
				fprintf(stderr, "OPENGL: Error on loading the shaders, switch to \"No filter\"\n");
				memcpy(cfg->shader_file, gfx.last_shader_file, sizeof(cfg->shader_file));
				filter = NO_FILTER;
				goto gfx_set_screen_start;
			case EXIT_OK:
				// opengl rendering
				framebuffer = opengl.sdl.surface;
				flip = opengl_flip;

				// gestione testo
				text.surface = surface_sdl;
				text_clear = opengl_text_clear;
				text_blit = opengl_text_blit;
				text.w = surface_sdl->w;
				text.h = surface_sdl->h;

				w_for_pr = gfx.vp.w;
				h_for_pr = gfx.vp.h;
				break;
		}
	}

	// questo controllo devo farlo necessariamente dopo il glew_init()
	if (!gfx.opengl && (filter >= FLTSHDSTART)) {
		filter = NO_FILTER;
		force_scale = TRUE;
		goto gfx_set_screen_start;
	}

	gfx_text_reset();

	// calcolo le proporzioni tra il disegnato a video (overscan e schermo
	// con le dimensioni per il filtro NTSC compresi) e quello che dovrebbe
	// essere (256 x 240). Mi serve per calcolarmi la posizione del puntatore
	// dello zapper.
	gfx.w_pr = ((float) w_for_pr / gfx.w[CURRENT]) * ((float) gfx.w[NO_OVERSCAN] / SCR_ROWS);
	gfx.h_pr = ((float) h_for_pr / gfx.h[CURRENT]) * ((float) gfx.h[NO_OVERSCAN] / SCR_LINES);

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
				palette = turn_off.ntsc;
			} else {
				palette = (void *) turn_off.palette;
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
					palette = pause.ntsc;
				} else {
					palette = pause.palette;
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
		if (gfx.opengl) {
			gfx.filter(screen.data,
					screen.line,
					palette,
					framebuffer->format->BitsPerPixel,
					framebuffer->pitch,
					framebuffer->pixels,
					gfx.rows,
					gfx.lines,
					framebuffer->w,
					framebuffer->h,
					opengl.scale);

			text_rendering(TRUE);

			opengl_draw_scene(framebuffer);
		} else {
			gfx.filter(screen.data,
					screen.line,
					palette,
					framebuffer->format->BitsPerPixel,
					framebuffer->pitch,
					framebuffer->pixels,
					gfx.rows,
					gfx.lines,
					framebuffer->w,
					framebuffer->h,
					cfg->scale);

			text_rendering(TRUE);
		}

		// disegno a video
		flip(framebuffer);

		// screenshot
		if (gfx.save_screenshot == TRUE) {
			if (gfx.opengl) {
				opengl_screenshot();
			} else {
				gui_save_screenshot(framebuffer->w, framebuffer->h, framebuffer->pixels, FALSE);
			}
			gfx.save_screenshot = FALSE;
		}
	}
}
void gfx_reset_video(void) {
	if (surface_sdl) {
		SDL_FreeSurface(surface_sdl);
	}

	surface_sdl = framebuffer = NULL;

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
	if (input_zapper_is_connected((_port *) &port) == TRUE) {
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
	return;
}
void gfx_text_blit(_txt_element *ele, _rect *rect) {
	SDL_Rect src_rect;

	if (!cfg->txt_on_screen) {
		return;
	}

	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.w = ele->w;
	src_rect.h = ele->h;

	SDL_BlitSurface(ele->surface, &src_rect, text.surface, rect);
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
			case SDLWIN_SWITCH_RENDERING:
				gfx_SWITCH_RENDERING();
				break;
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
