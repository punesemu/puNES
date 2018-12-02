/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#if defined (__unix__)
#include <pthread.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "video/gfx.h"
#include "video/gfx_thread.h"
#include "fps.h"
#include "info.h"
#include "conf.h"
#include "opengl.h"
#include "clock.h"
#include "ppu.h"
#include "gui.h"
#include "palette.h"
#include "paldef.h"
#include "vs_system.h"
#include "settings.h"
#include "video/effects/pause.h"
#include "video/effects/tv_noise.h"

BYTE gfx_init(void) {
	gfx.save_screenshot = FALSE;

	gui_screen_info();

	if (gui_create() == EXIT_ERROR) {
		fprintf(stderr, "gui initialization failed\n");
		return (EXIT_ERROR);
	}

	if (gfx_thread_init() == EXIT_ERROR) {
		fprintf(stderr, "Unable to allocate the gfx thread\n");
		return (EXIT_ERROR);
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

	// casi particolari provenienti dal settings_file_parse() e cmd_line_parse()
	if (cfg->fullscreen == FULLSCR) {
		gfx.scale_before_fscreen = cfg->scale;
	}

	if (cfg->fullscreen) {
		gfx_set_screen(cfg->scale, cfg->filter, cfg->shader, NO_FULLSCR, cfg->palette, FALSE, FALSE);
		cfg->fullscreen = NO_FULLSCR;
		cfg->scale = gfx.scale_before_fscreen;
		gui_fullscreen();
	} else {
		gfx_set_screen(cfg->scale, cfg->filter, cfg->shader, NO_FULLSCR, cfg->palette, FALSE, FALSE);
	}

	return (EXIT_OK);
}
void gfx_quit(void) {
	gfx_thread_quit();

	if (gfx.palette) {
		free(gfx.palette);
		gfx.palette = NULL;
	}

	pause_quit();
	tv_noise_quit();

	opengl_quit();
	ntsc_quit();
	text_quit();
}
void gfx_set_screen(BYTE scale, DBWORD filter, DBWORD shader, BYTE fullscreen, BYTE palette,
	BYTE force_scale, BYTE force_palette) {
	BYTE set_mode;
	WORD width, height;
	DBWORD old_shader = cfg->shader;

	gfx_thread_pause();

	if (shader_effect.params > 0) {
		settings_shp_save();
	}

	gfx_set_screen_start:
	set_mode = FALSE;
	width = 0, height = 0;

	// l'ordine dei vari controlli non deve essere cambiato:
	// 0) overscan
	// 1) filtro
	// 2) fullscreen
	// 3) fattore di scala
	// 4) tipo di paletta

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

	// filtro
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

	// shader
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
			gfx.filter.width_pixel = (float)nes_ntsc_out_chunk / (float)nes_ntsc_in_chunk;
			if (overscan.enabled) {
				width -= ((float)(overscan.borders->left + overscan.borders->right) * gfx.filter.width_pixel);
			}
			switch (scale) {
				case X2:
					gfx.width_pixel = gfx.filter.width_pixel;
					break;
				default:
					width = ((float)width / 2.0f) * (float)scale;
					gfx.w[NO_OVERSCAN] = ((float)gfx.w[NO_OVERSCAN] / 2.0f) * (float)scale;
					gfx.width_pixel = (gfx.filter.width_pixel / 2.0f) * (float)scale;
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
					ntsc_set(NULL, cfg->ntsc_format, FALSE, (BYTE *)palette_base_file, 0, (BYTE *)palette_RGB.noswap);
				}
			}
		}

		switch (palette) {
			case PALETTE_PAL:
				ntsc_set(NULL, cfg->ntsc_format, FALSE, (BYTE *)palette_base_pal, 0, (BYTE *)palette_RGB.noswap);
				break;
			case PALETTE_NTSC:
				ntsc_set(NULL, cfg->ntsc_format, FALSE, 0, 0, (BYTE *)palette_RGB.noswap);
				break;
			case PALETTE_FRBX_NOSTALGIA:
				ntsc_set(NULL, cfg->ntsc_format, FALSE, (BYTE *)palette_firebrandx_nostalgia_FBX, 0, (BYTE *)palette_RGB.noswap);
				break;
			case PALETTE_FRBX_YUV:
				ntsc_set(NULL, cfg->ntsc_format, FALSE, (BYTE *)palette_firebrandx_YUV_v3, 0, (BYTE *)palette_RGB.noswap);
				break;
			case PALETTE_GREEN:
				rgb_modifier(NULL, palette_RGB.noswap, 0x00, -0x20, 0x20, -0x20);
				break;
			case PALETTE_FILE:
				break;
			default:
				ntsc_set(NULL, cfg->ntsc_format, palette, 0, 0, (BYTE *)palette_RGB.noswap);
				break;
		}

		if (vs_system.enabled) {
			switch (vs_system.ppu) {
				case RP2C03B:
				case RP2C03G:
					break;
				case RP2C04:
					ntsc_set(NULL, cfg->ntsc_format, FALSE, (BYTE *)palette_RP2C04_0001, 0, (BYTE *)palette_RGB.noswap);
					break;
				case RP2C04_0002:
					ntsc_set(NULL, cfg->ntsc_format, FALSE, (BYTE *)palette_RP2C04_0002, 0, (BYTE *)palette_RGB.noswap);
					break;
				case RP2C04_0003:
					ntsc_set(NULL, cfg->ntsc_format, FALSE, (BYTE *)palette_RP2C04_0003, 0, (BYTE *)palette_RGB.noswap);
					break;
				case RP2C04_0004:
					ntsc_set(NULL, cfg->ntsc_format, FALSE, (BYTE *)palette_RP2C04_0004, 0, (BYTE *)palette_RGB.noswap);
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
	}

	gfx_palette_update();

	// salvo il nuovo fattore di scala
	cfg->scale = scale;
	// salvo il nuovo filtro
	cfg->filter = filter;
	// salvo la nuova shader
	cfg->shader = shader;
	// salvo il nuovo stato del fullscreen
	cfg->fullscreen = fullscreen;
	// salvo il nuovo tipo di paletta
	cfg->palette = palette;

	// Pixel Aspect Ratio
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

		if (shader_effect.params > 0) {
			settings_shp_parse();
		}

		if (set_mode) {
			if (fullscreen == TRUE) {
				gfx.w[VIDEO_MODE] = gfx.w[MONITOR];
				gfx.h[VIDEO_MODE] = gfx.h[MONITOR];
			} else if (cfg->oscan_black_borders) {
				gfx.w[VIDEO_MODE] = gfx.w[NO_OVERSCAN];
				gfx.h[VIDEO_MODE] = gfx.h[NO_OVERSCAN];
			} else {
				gfx.w[VIDEO_MODE] = width;
				gfx.h[VIDEO_MODE] = height;
			}

			// Pixel Aspect Ratio
			if (cfg->pixel_aspect_ratio && !fullscreen) {
				gfx.w[VIDEO_MODE] = (gfx.w[NO_OVERSCAN] * gfx.pixel_aspect_ratio);

				if (overscan.enabled && !cfg->oscan_black_borders) {
					float brd = 0;

					brd = (float)gfx.w[VIDEO_MODE] / (float)SCR_ROWS;
					brd *= (overscan.borders->right + overscan.borders->left);

					gfx.w[VIDEO_MODE] -= brd;
				}
			}

			// faccio quello che serve prima del setvideo
			gui_set_video_mode();
		}

		switch (opengl_context_create()) {
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

	// calcolo le proporzioni tra il disegnato a video (overscan e schermo
	// con le dimensioni per il filtro NTSC compresi) e quello che dovrebbe
	// essere (256 x 240). Mi serve per calcolarmi la posizione del puntatore
	// dello zapper.
	if (cfg->fullscreen) {
		gfx.w_pr = (float)gfx.vp.w / (float)SCR_ROWS;
		gfx.h_pr = (float)gfx.vp.h / (float)SCR_LINES;
	} else {
		gfx.w_pr = (float)(gfx.w[NO_OVERSCAN] * gfx.pixel_aspect_ratio) / (float)SCR_ROWS;
		gfx.h_pr = (float)gfx.h[NO_OVERSCAN] / (float)SCR_LINES;
	}

	gfx_thread_continue();

	// setto il titolo della finestra
	gui_update();

	if (info.on_cfg == TRUE) {
		info.on_cfg = FALSE;
	}
}
void gfx_draw_screen(void) {
	if (gfx_thread_public.filtering == TRUE) {
		fps.frames_skipped++;
		return;
	}

	screen.rd = screen.wr;

	if (info.doublebuffer == TRUE) {
		screen.index = !screen.index;
		screen.wr = &screen.buff[screen.index];
	}

	if (screen.rd->ready == FALSE) {
		screen.rd->ready = TRUE;
	}
}

uint32_t gfx_color(BYTE a, BYTE r, BYTE g, BYTE b) {
	return (gui_color(a, r, g, b));
}
void gfx_palette_update(void) {
	WORD i;

	if ((cfg->disable_swap_emphasis_pal == FALSE) && ((machine.type == PAL) || (machine.type == DENDY))) {
		palette_RGB.in_use = &palette_RGB.swapped[0];
	} else {
		palette_RGB.in_use = &palette_RGB.noswap[0];
	}

	// inizializzo in ogni caso la tabella YUV dell'hqx
	hqx_init();

	// memorizzo i colori della paletta nel formato di visualizzazione
	for (i = 0; i < NUM_COLORS; i++) {
		gfx.palette[i] = gfx_os_color(palette_RGB.in_use[i].r, palette_RGB.in_use[i].g, palette_RGB.in_use[i].b);
	}
}

void gfx_cursor_init(void) {
	gui_cursor_init();
	gui_cursor_set();
}
void gfx_cursor_set(void) {
	gui_cursor_set();
}

void gfx_text_create_surface(_txt_element *ele) {
	uint32_t size = (ele->h * ele->w) * sizeof(uint32_t);

	ele->surface = malloc(size);
	memset(ele->surface, 0x00, size);

	ele->blank = malloc(size);
	memset(ele->blank, 0x00, size);
}
void gfx_text_release_surface(_txt_element *ele) {
	if (ele->surface) {
		free(ele->surface);
		ele->surface = NULL;
	}
	if (ele->blank) {
		free(ele->blank);
		ele->blank = NULL;
	}
}
void gfx_text_rect_fill(_txt_element *ele, _txt_rect *rect, uint32_t color) {
	uint32_t *pbits;
	uint32_t pitch;
	int w, h;

	pitch = ele->w;
	pbits = (uint32_t *)ele->surface;
	pbits += (rect->y * ele->w) + rect->x;

	for (h = 0; h < rect->h; h++) {
		for (w = 0; w < rect->w; w++) {
			(*(pbits + w)) = color;
		}
		pbits += pitch;
	}
}
void gfx_text_reset(void) {
	txt_table[TXT_NORMAL] = gfx_color(0, 0xFF, 0xFF, 0xFF);
	txt_table[TXT_RED]    = gfx_color(0, 0xFF, 0x4C, 0x3E);
	txt_table[TXT_YELLOW] = gfx_color(0, 0xFF, 0xFF, 0   );
	txt_table[TXT_GREEN]  = gfx_color(0, 0   , 0xFF, 0   );
	txt_table[TXT_CYAN]   = gfx_color(0, 0   , 0xFF, 0xFF);
	txt_table[TXT_BROWN]  = gfx_color(0, 0xEB, 0x89, 0x31);
	txt_table[TXT_BLUE]   = gfx_color(0, 0x2D, 0x8D, 0xBD);
	txt_table[TXT_GRAY]   = gfx_color(0, 0xA0, 0xA0, 0xA0);
	txt_table[TXT_BLACK]  = gfx_color(0, 0   , 0   , 0   );
}
void gfx_text_clear(_txt_element *ele) {
	int x, y;

	if (!ele->blank) {
		return;
	}

	text_calculate_real_x_y(ele, &x, &y);

	glBindTexture(GL_TEXTURE_2D, opengl.text.id);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, ele->w);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, ele->w, ele->h, TI_FRM, TI_TYPE, ele->blank);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}
void gfx_text_blit(_txt_element *ele, _txt_rect *rect) {
	if (!cfg->txt_on_screen) {
		return;
	}
	glBindTexture(GL_TEXTURE_2D, opengl.text.id);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, rect->w);
	glTexSubImage2D(GL_TEXTURE_2D, 0, rect->x, rect->y, rect->w, rect->h, TI_FRM, TI_TYPE, ele->surface);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

void gfx_apply_filter(void) {
	void *palette = (void *)gfx.palette;

	//applico la paletta adeguata.
	if (cfg->filter == NTSC_FILTER) {
		palette = NULL;
	}
	if (info.no_rom | info.turn_off) {
		if (cfg->filter == NTSC_FILTER) {
			palette = turn_off_effect.ntsc;
		} else {
			palette = (void *)turn_off_effect.palette;
		}
	} else if (info.pause) {
		if (!cfg->disable_sepia_color) {
			if (cfg->filter == NTSC_FILTER) {
				palette = pause_effect.ntsc;
			} else {
				palette = pause_effect.palette;
			}
		}
	}

	gfx_thread_lock();

	// applico l'effetto desiderato
	gfx.filter.func(palette,
		opengl.surface.pitch,
		opengl.surface.pixels,
		opengl.surface.w,
		opengl.surface.h);

	gfx_thread_unlock();
	gui_screen_update();
}
