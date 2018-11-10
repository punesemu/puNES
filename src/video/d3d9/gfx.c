/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#include "d3d9.h"
#include "video/gfx_thread.h"
#include "fps.h"
#include "gui.h"
#include "info.h"
#include "conf.h"
#include "ppu.h"
#include "clock.h"
#include "palette.h"
#include "paldef.h"
#include "vs_system.h"
#include "video/effects/pause.h"
#include "video/effects/tv_noise.h"

BYTE gfx_init(void) {
	gfx.save_screenshot = FALSE;

	if (gui_create() == EXIT_ERROR) {
		MessageBox(NULL, "Gui initialization failed", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	if (gfx_thread_init() == EXIT_ERROR) {
		MessageBox(NULL, "Unable to allocate the gfx thread", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	if (d3d9_init() == EXIT_ERROR) {
		MessageBox(NULL, "Unable to initiliazed d3d9", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	// inizializzo l'ntsc che utilizzero' non solo
	// come filtro ma anche nel gfx_set_screen() per
	// generare la paletta dei colori.
	if (ntsc_init(0, 0, 0, 0, 0) == EXIT_ERROR) {
		MessageBox(NULL, "Unable to initialize palette", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	// mi alloco una zona di memoria dove conservare la
	// paletta nel formato di visualizzazione.
	if (!(gfx.palette = malloc(NUM_COLORS * sizeof(uint32_t)))) {
		MessageBox(NULL, "Unable to allocate the palette", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	if (pause_init() == EXIT_ERROR) {
		MessageBox(NULL, "pause initialization failed", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	if (tv_noise_init() == EXIT_ERROR) {
		MessageBox(NULL, "tv_noise initialization failed", "Error!", MB_ICONEXCLAMATION | MB_OK);
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

	pause_quit();
	tv_noise_quit();

	ntsc_quit();
	text_quit();

	if (gfx.palette) {
		free(gfx.palette);
		gfx.palette = NULL;
	}

	d3d9_quit();
}
void gfx_set_screen(BYTE scale, DBWORD filter, DBWORD shader, BYTE fullscreen, BYTE palette,
	BYTE force_scale, BYTE force_palette) {
	BYTE set_mode;
	WORD width, height;
	DBWORD old_shader = cfg->shader;

	gfx_thread_pause();

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
			case NO_FILTER:
			default:
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

		// memorizzo i colori della paletta nel formato di visualizzazione
		{
			WORD i;

			for (i = 0; i < NUM_COLORS; i++) {
				gfx.palette[i] = gfx_color(255, palette_RGB[i].r, palette_RGB[i].g, palette_RGB[i].b);
			}
		}
	}

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

					brd = (float) gfx.w[VIDEO_MODE] / (float) SCR_ROWS;
					brd *= (overscan.borders->right + overscan.borders->left);

					gfx.w[VIDEO_MODE] -= brd;
				}
			}

			// faccio quello che serve prima del setvideo
			gui_set_video_mode();
		}

		switch (d3d9_context_create()) {
			case EXIT_ERROR:
				fprintf(stderr, "D3D9: Unable to initialize d3d context\n");
				gfx_thread_continue();
				return;
			case EXIT_ERROR_SHADER:
				text_add_line_info(1, "[red]errors[normal] on shader, use [green]'No shader'");
				fprintf(stderr, "CG: Error on loading the shaders, switch to \"No shader\"\n");
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
		gfx.w_pr = (float) gfx.vp.w / (float) SCR_ROWS;
		gfx.h_pr = (float) gfx.vp.h / (float) SCR_LINES;
	} else {
		gfx.w_pr = (float) (gfx.w[NO_OVERSCAN] * gfx.pixel_aspect_ratio) / (float) SCR_ROWS;
		gfx.h_pr = (float) gfx.h[NO_OVERSCAN] / (float) SCR_LINES;
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
	screen.index = !screen.index;
	screen.wr = &screen.buff[screen.index];

	if (screen.rd->ready == FALSE) {
		screen.rd->ready = TRUE;
	}
}

void gfx_control_changed_adapter(void *monitor) {
	_d3d9_adapter *old_adapter = d3d9.adapter;
	HMONITOR *in_use = monitor;
	int i;

	if ((*in_use) == IDirect3D9_GetAdapterMonitor(d3d9.d3d, d3d9.adapter->id)) {
		return;
	}

	for (i = 0; i < d3d9.adapters_in_use; i++) {
		_d3d9_adapter *adapter = D3D9_ADAPTER(i);

		if ((*in_use) == IDirect3D9_GetAdapterMonitor(d3d9.d3d, adapter->id)) {
			d3d9.adapter = adapter;
			if (d3d9_context_create() == EXIT_OK) {
				return;
			}
			fprintf(stderr, "D3D9 : Unable to initialize new d3d context\n");

			d3d9.adapter = old_adapter;
			if (d3d9_context_create() == EXIT_OK) {
				return;
			}
			fprintf(stderr, "D3D9 : Unable to initialize old d3d context\n");
			break;
		}
	}
}

uint32_t gfx_color(BYTE a, BYTE r, BYTE g, BYTE b) {
	return (D3DCOLOR_ARGB(a, r, g, b));
}

void gfx_cursor_init(void) {
	gui_cursor_init();
	gui_cursor_set();
};
void gfx_cursor_set(void) {
	gui_cursor_set();
};

void gfx_text_create_surface(_txt_element *ele) {
	ele->surface = malloc((ele->h * ele->w) * (gfx.bit_per_pixel / 8));
}
void gfx_text_release_surface(_txt_element *ele) {
	if (ele->surface) {
		free(ele->surface);
		ele->surface = NULL;
	}
}
void gfx_text_rect_fill(_txt_element *ele, _txt_rect *rect, uint32_t color) {
	uint32_t *pbits;
	LONG pitch;
	int w, h;

	pitch = ele->w;
	pbits = (uint32_t *) ele->surface;
	pbits += (rect->y * ele->w) + rect->x;

	for (h = 0; h < rect->h; h++) {
		for (w = 0; w < rect->w; w++) {
			(*(pbits + w)) = color;
		}
		pbits += pitch;
	}
}
void gfx_text_reset(void) {
	txt_table[TXT_NORMAL] = D3DCOLOR_ARGB(0, 0xFF, 0xFF, 0xFF);
	txt_table[TXT_RED]    = D3DCOLOR_ARGB(0, 0xFF, 0x4C, 0x3E);
	txt_table[TXT_YELLOW] = D3DCOLOR_ARGB(0, 0xFF, 0xFF, 0   );
	txt_table[TXT_GREEN]  = D3DCOLOR_ARGB(0, 0   , 0xFF, 0   );
	txt_table[TXT_CYAN]   = D3DCOLOR_ARGB(0, 0   , 0xFF, 0xFF);
	txt_table[TXT_BROWN]  = D3DCOLOR_ARGB(0, 0xEB, 0x89, 0x31);
	txt_table[TXT_BLUE]   = D3DCOLOR_ARGB(0, 0x2D, 0x8D, 0xBD);
	txt_table[TXT_GRAY]   = D3DCOLOR_ARGB(0, 0xA0, 0xA0, 0xA0);
	txt_table[TXT_BLACK]  = D3DCOLOR_ARGB(0, 0   , 0   , 0   );
}
void gfx_text_clear(_txt_element *ele) {
	D3DLOCKED_RECT lock_dst;
	RECT dst;
	uint32_t *pbits;
	int w, h, x, y;

	if (!d3d9.text.data) {
		return;
	}

	text_calculate_real_x_y(ele, &x, &y);

	dst.left = x;
	dst.top = y;
	dst.right = x + ele->w;
	dst.bottom = y + ele->h;

	if (IDirect3DSurface9_LockRect(d3d9.text.offscreen, &lock_dst, &dst, D3DLOCK_DISCARD) != D3D_OK) {
		printf("D3D9 : LockRect text surface error\n");
		return;
	}

	pbits = (uint32_t *) lock_dst.pBits;

	for (h = 0; h < ele->h; h++) {
		for (w = 0; w < ele->w; w++) {
			(*(pbits + w)) = 0;
		}
		pbits += lock_dst.Pitch / (gfx.bit_per_pixel / 8);
	}

	IDirect3DSurface9_UnlockRect(d3d9.text.offscreen);
}
void gfx_text_blit(_txt_element *ele, _txt_rect *rect) {
	D3DLOCKED_RECT lock_dst;
	RECT dst;
	LONG pitch;
	unsigned char *psrc, *pdst;
	int h;

	if (!cfg->txt_on_screen) {
		return;
	}

	dst.left = rect->x;
	dst.top = rect->y;
	dst.right = rect->x + rect->w;
	dst.bottom = rect->y + rect->h;

	if (IDirect3DSurface9_LockRect(d3d9.text.offscreen, &lock_dst, &dst, D3DLOCK_DISCARD) != D3D_OK) {
		printf("D3D9 : LockRect text surface error\n");
		return;
	}

	pitch = rect->w * (gfx.bit_per_pixel / 8);
	psrc = (unsigned char *) ele->surface;
	pdst = (unsigned char *) lock_dst.pBits;

	for (h = 0; h < rect->h; h++) {
		memcpy(pdst, psrc, pitch);
		psrc += pitch;
		pdst += lock_dst.Pitch;
	}

	IDirect3DSurface9_UnlockRect(d3d9.text.offscreen);
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

	{
		const _texture_simple *scrtex = &d3d9.screen.tex[d3d9.screen.index];
		D3DLOCKED_RECT lrect;

		// lock della surface in memoria
		IDirect3DSurface9_LockRect(scrtex->offscreen, &lrect, NULL, D3DLOCK_DISCARD);
		// applico l'effetto
		gfx.filter.func(palette,
			lrect.Pitch,
			lrect.pBits,
			scrtex->rect.base.w,
			scrtex->rect.base.h);
		// unlock della surface in memoria
		IDirect3DSurface9_UnlockRect(scrtex->offscreen);

		// aggiorno la texture dello schermo
		if (overscan.enabled) {
			POINT point;
			RECT rect;

			rect.left = overscan.borders->left * gfx.filter.width_pixel;
			rect.top = overscan.borders->up * gfx.filter.factor;
			rect.right = scrtex->rect.base.w - (overscan.borders->right * gfx.filter.width_pixel);
			rect.bottom = scrtex->rect.base.h - (overscan.borders->down * gfx.filter.factor);

			point.x = rect.left;
			point.y = rect.top;

			IDirect3DDevice9_UpdateSurface(d3d9.adapter->dev, scrtex->offscreen, &rect, scrtex->map0, &point);
		} else {
			IDirect3DDevice9_UpdateSurface(d3d9.adapter->dev, scrtex->offscreen, NULL, scrtex->map0, NULL);
		}
	}

	gfx_thread_unlock();
	gui_screen_update();

	return;
}
