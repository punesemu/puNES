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

#include <string.h>
#include "d3d9.h"
#include "gui.h"
#include "emu.h"
#include "info.h"
#include "conf.h"
#include "ppu.h"
#include "overscan.h"
#include "clock.h"
#include "palette.h"
#include "paldef.h"

#define D3D9_ADAPTER(i) (_d3d9_adapter *) ((BYTE *) d3d9.array + (i * sizeof(_d3d9_adapter)))
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

static void d3d9_shader_cg_error_handler(void);
static BYTE d3d9_device_create(UINT width, UINT height);
static BYTE d3d9_context_create(void);
static void d3d9_context_delete(void);
static BYTE d3d9_texture_create(_texture *texture, UINT index);
static BYTE d3d9_texture_simple_create(_texture_simple *texture, UINT w, UINT h, BOOL text);
static BYTE d3d9_texture_lut_create(_lut *lut, UINT index);
static void d3d9_surface_clean(LPDIRECT3DSURFACE9 *surface, UINT width, UINT height);
static BYTE d3d9_shader_init(UINT pass, _shader *shd, const char *path, const char *code);
static void d3d9_shader_delete(_shader *shd);
static void d3d9_shader_uniform_ctrl(CGparameter *dst, CGparameter *param, const char *semantic);
static void d3d9_shader_uni_texture_clear(_shader_uniforms_tex *sut);
static void d3d9_shader_uni_texture(_shader_uniforms_tex *sut, _shader_prg_cg *prg, char *fmt, ...);
static CGparameter d3d9_cg_find_param(CGparameter prm, const char *name);
static BYTE d3d9_vertex_declaration_create(_shader *shd);
static void d3d9_vertex_buffer_set(_shader *shd,  _viewport *vp, _texture_rect *prev);
INLINE static void d3d9_viewport_set(DWORD x, DWORD y, DWORD w, DWORD h);
INLINE D3DTEXTUREFILTERTYPE d3d9_shader_filter(UINT type);
INLINE static void d3d9_shader_params_text_set(_shader *shd);
INLINE static void d3d9_shader_param_set(const _texture *texture, UINT fcountmod, UINT fcount);

static const BYTE ntsc_width_pixel[5] = {0, 0, 7, 10, 14};

BYTE gfx_init(void) {
	gfx.save_screenshot = FALSE;

	// casi particolari provenienti dal settings_file_parse() e cmd_line_parse()
	if ((cfg->scale == X1) && (cfg->filter != NO_FILTER)) {
		cfg->scale = X2;
	}

	if (gui_create() == EXIT_ERROR) {
		MessageBox(NULL, "Gui initialization failed", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	memset(&d3d9, 0x00, sizeof(d3d9));

	if ((d3d9.d3d = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) {
		MessageBox(NULL, "Unable to create d3d object", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	// mi passo in rassegna tutti gli adapter presenti sul sistema
	d3d9.adapters_on_system = IDirect3D9_GetAdapterCount(d3d9.d3d);

	if (!(d3d9.array = malloc(d3d9.adapters_on_system * sizeof(_d3d9_adapter)))) {
		MessageBox(NULL, "Unable to create devices array", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	{
		int adapt;

#define dev_error(s) fprintf(stderr, "D3D9 adapter %d : "s, dev->id)
#define dev_info(s) printf("D3D9 adapter %d : "s, dev->id)
#define dev_info_args(s, ...) printf("D3D9 adapter %d : "s, dev->id, __VA_ARGS__)

		for (adapt = 0; adapt < d3d9.adapters_on_system; adapt++) {
			_d3d9_adapter *dev = D3D9_ADAPTER(d3d9.adapters_in_use);
			D3DADAPTER_IDENTIFIER9 info;
			D3DCAPS9 d3dcaps;

			memset(dev, 0x00, sizeof(_d3d9_adapter));

			dev->id = adapt;

			if (IDirect3D9_GetAdapterIdentifier(d3d9.d3d, dev->id, 0, &info)!= D3D_OK) {
				dev_error("unable to get adapter display info\n");
				continue;
			}

			dev_info_args("%s\n", info.Description);

			if (IDirect3D9_GetAdapterDisplayMode(d3d9.d3d, dev->id, &dev->display_mode)
					!= D3D_OK) {
				dev_error("unable to get adapter display mode\n");
				continue;
			}

			if ((dev->display_mode.Format == D3DFMT_X8R8G8B8)
					|| (dev->display_mode.Format == D3DFMT_A8R8G8B8)) {
				dev->bit_per_pixel = 32;
			}
			if (dev->bit_per_pixel < 32) {
				dev_error("video mode < 32 bits are not supported\n");
				continue;
			}

			// Check for hardware T&L
			if (IDirect3D9_GetDeviceCaps(d3d9.d3d,
					dev->id,
					D3DDEVTYPE_HAL,
					&d3dcaps) != D3D_OK) {
				dev_error("unable to get device caps\n");
				continue;
			}

			if (d3dcaps.Caps2 & D3DCAPS2_DYNAMICTEXTURES) {
				dev->dynamic_texture = TRUE;
			} else {
				dev->dynamic_texture = FALSE;
				dev_info("don't support dynamic texture\n");
			}

			if (d3dcaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) {
				dev->texture_square_only = TRUE;
				dev_info("support only square texture\n");
			} else {
				dev->texture_square_only = FALSE;
			}

		 	// Device can accelerate a memory copy from system memory to local video memory.
		 	// This cap guarantees that UpdateSurface and UpdateTexture calls will be hardware
		 	// accelerated. If this cap is absent, these calls will succeed but will be slower.
			if (!(d3dcaps.Caps3 & D3DCAPS3_COPY_TO_VIDMEM)) {
				dev_info("don't support accelerated texture update\n");
			}

			if (d3dcaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
				dev->flags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
				// se abilito il PURE DEVICE, non posso utilizzare il
				// IDirect3DDevice9_GetTransform quando uso le shaders.
				if (d3dcaps.DevCaps & D3DDEVCAPS_PUREDEVICE) {
					dev->flags |= D3DCREATE_PUREDEVICE;
				}
			} else {
				dev_info("don't support hardware accelaration\n");
				dev->flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
			}

			if (!(d3dcaps.MaxSimultaneousTextures > 1)) { //number of textures
				dev_info("single pass multitexturing not supported\n");
			} else {
				dev_info_args("MaxSimultaneousTextures %ld\n", d3dcaps.MaxSimultaneousTextures);
			}

			dev->number_of_monitors = d3dcaps.NumberOfAdaptersInGroup;

			//if (dev->number_of_monitors > 1) {
				dev_info_args("MasterAdapterOrdinal    %d\n", d3dcaps.MasterAdapterOrdinal);
				dev_info_args("AdapterOrdinalInGroup   %d\n", d3dcaps.AdapterOrdinalInGroup);
				dev_info_args("NumberOfAdaptersInGroup %d\n", dev->number_of_monitors);
				//dev->flags |= D3DCREATE_ADAPTERGROUP_DEVICE;
			//}

			{
				d3d9.adapter = dev;

				// per poter verificare se le shaders sono utilizzabili devo creare il dev d3d
				if (d3d9_device_create(1, 1) != EXIT_OK) {
					continue;
				}

				dev->hlsl_compliant = FALSE;

				if (d3dcaps.PixelShaderVersion < D3DPS_VERSION(2, 0) ||
						(d3dcaps.VertexShaderVersion < D3DVS_VERSION(2, 0))) {
					dev_info("don't support shaders >= 2.0\n");
				} else {
					dev->hlsl_compliant = TRUE;
				}

				if (dev->hlsl_compliant == FALSE) {
					dev_info("shaders are not supported\n");
				}

				d3d9.adapter = NULL;
			}

			d3d9.adapters_in_use++;
		}

#undef dev_error
#undef dev_info
#undef dev_info_args
	}

	if (d3d9.adapters_in_use == 0) {
		MessageBox(NULL, "Unable find usable adapter", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	d3d9.adapter = D3D9_ADAPTER(0);
	if (d3d9.adapter->hlsl_compliant == FALSE) {
		MessageBox(NULL, "Adapter is not hlsl compliant", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	gfx.bit_per_pixel = d3d9.adapter->bit_per_pixel;

	// inizializzo l'ntsc che utilizzero' non solo
	// come filtro ma anche nel gfx_set_screen() per
	// generare la paletta dei colori.
	if (ntsc_init(0, 0, 0, 0, 0) == EXIT_ERROR) {
		MessageBox(NULL, "Unable to initialize palette", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	// mi alloco una zona di memoria dove conservare la
	// paletta nel formato di visualizzazione.
	if (!(d3d9.palette = malloc(NUM_COLORS * sizeof(uint32_t)))) {
		MessageBox(NULL, "Unable to allocate the palette", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	// casi particolari provenienti dal settings_file_parse() e cmd_line_parse()
	if (cfg->fullscreen == FULLSCR) {
		gfx.scale_before_fscreen = cfg->scale;
	}

	if (cfg->fullscreen) {
		gfx_set_screen(cfg->scale, cfg->filter, NO_FULLSCR, cfg->palette, FALSE, FALSE);
		cfg->fullscreen = NO_FULLSCR;
		cfg->scale = gfx.scale_before_fscreen;
		gui_fullscreen();
	} else {
		gfx_set_screen(cfg->scale, cfg->filter, NO_FULLSCR, cfg->palette, FALSE, FALSE);
	}

	return (EXIT_OK);
}
void gfx_set_screen(BYTE scale, DBWORD filter, BYTE fullscreen, BYTE palette, BYTE force_scale,
		BYTE force_palette) {
	BYTE set_mode;
	WORD width, height;
	WORD w_for_pr, h_for_pr;
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
				gfx.filter = scale_surface;
				// se sto passando dal filtro ntsc ad un'altro, devo
				// ricalcolare la larghezza del video mode quindi
				// forzo il controllo del fattore di scala.
				if (cfg->filter == NTSC_FILTER) {
					// devo reimpostare la larghezza del video mode
					scale = cfg->scale;
				}
				break;
			case SCALE2X:
			case SCALE3X:
			case SCALE4X:
				gfx.filter = scaleNx;
				break;
			case HQ2X:
			case HQ3X:
			case HQ4X:
				gfx.filter = hqNx;
				break;
			case XBRZ2X:
			case XBRZ3X:
			case XBRZ4X:
			case XBRZ5X:
			case XBRZ6X:
				gfx.filter = xBRZ;
				break;
			case NTSC_FILTER:
				gfx.filter = ntsc_surface;
				// il fattore di scala deve essere gia' stato
				// inizializzato almeno una volta.
				if (cfg->scale != NO_CHANGE) {
					// devo reimpostare la larghezza del video mode
					scale = cfg->scale;
				} else if (scale == NO_CHANGE) {
					// se scale e new_scale sono uguali a NO_CHANGE,
					// imposto un default.
					scale = X2;
				}
				break;
		}
		// forzo il controllo del fattore di scale
		force_scale = TRUE;
		// indico che devo cambiare il video mode
		set_mode = TRUE;
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
				// il fattore di scala a 1 e' possibile solo senza filtro
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
					ntsc_set(cfg->ntsc_format, FALSE, (BYTE *) palette_base_file, 0,
							(BYTE *) palette_RGB);
				}
			}
		}

		switch (palette) {
			case PALETTE_PAL:
				ntsc_set(cfg->ntsc_format, FALSE, (BYTE *) palette_base_pal, 0,
						(BYTE *) palette_RGB);
				break;
			case PALETTE_NTSC:
				ntsc_set(cfg->ntsc_format, FALSE, 0, 0, (BYTE *) palette_RGB);
				break;
			case PALETTE_FRBX_UNSATURED:
				ntsc_set(cfg->ntsc_format, FALSE, (BYTE *) palette_firebrandx_unsaturated_v5, 0,
						(BYTE *) palette_RGB);
				break;
			case PALETTE_FRBX_YUV:
				ntsc_set(cfg->ntsc_format, FALSE, (BYTE *) palette_firebrandx_YUV_v3, 0,
						(BYTE *) palette_RGB);
				break;
			case PALETTE_GREEN:
				rgb_modifier(-0x20, 0x20, -0x20);
				break;
			case PALETTE_FILE:
				break;
			default:
				ntsc_set(cfg->ntsc_format, palette, 0, 0, (BYTE *) palette_RGB);
				break;
		}

		// inizializzo in ogni caso la tabella YUV dell'hqx
		hqx_init();

		// memorizzo i colori della paletta nel formato di visualizzazione
		{
			WORD i;

			for (i = 0; i < NUM_COLORS; i++) {
				d3d9.palette[i] =
						D3DCOLOR_ARGB(255, palette_RGB[i].r, palette_RGB[i].g, palette_RGB[i].b);
			}
		}
	}

	// salvo il nuovo fattore di scala
	cfg->scale = scale;
	// salvo ill nuovo filtro
	cfg->filter = filter;
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
		DBWORD f = NO_FILTER;
		d3d9.scale = cfg->scale;
		gfx.PSS = ((cfg->pixel_aspect_ratio != PAR11) && cfg->PAR_soft_stretch) ? TRUE : FALSE;

		if ((filter == NO_FILTER) || (filter >= FLTSHDSTART)) {
			d3d9.scale = X1;
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

		if (set_mode) {
			gfx.w[VIDEO_MODE] = width;
			gfx.h[VIDEO_MODE] = height;

			if (fullscreen == TRUE) {
				gfx.w[VIDEO_MODE] = gfx.w[MONITOR];
				gfx.h[VIDEO_MODE] = gfx.h[MONITOR];
			}

			// Pixel Aspect Ratio
			if (cfg->pixel_aspect_ratio && !fullscreen) {
				float brd = 0;

				gfx.w[VIDEO_MODE] = (gfx.w[NO_OVERSCAN] * gfx.pixel_aspect_ratio);

				if (overscan.enabled) {
					brd = (float) gfx.w[VIDEO_MODE] / (float) SCR_ROWS;
					brd *= (overscan.borders->right + overscan.borders->left);
				}

				gfx.w[VIDEO_MODE] -= brd;
			}

			// faccio quello che serve prima del setvideo
			gui_set_video_mode();
		}

		switch (d3d9_context_create()) {
			case EXIT_ERROR:
				fprintf(stderr, "D3D9: Unable to initialize d3d context\n");
				return;
			case EXIT_ERROR_SHADER:
				text_add_line_info(1, "[red]errors[normal] on shader, use [green]'No filter'");
				fprintf(stderr, "CG: Error on loading the shaders, switch to \"No filter\"\n");
				memcpy(cfg->shader_file, gfx.last_shader_file, sizeof(cfg->shader_file));
				filter = NO_FILTER;
				goto gfx_set_screen_start;
		}
	}

	text.w = gfx.w[VIDEO_MODE];
	text.h = gfx.h[VIDEO_MODE];

	w_for_pr = gfx.vp.w;
	h_for_pr = gfx.vp.h;

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
	if (!forced && (info.no_rom || info.pause)) {
		if (++info.pause_frames_drawscreen == 4) {
			info.pause_frames_drawscreen = 0;
			forced = TRUE;
		} else {
			text_rendering(FALSE);
			return;
		}
	}

	// filtro e aggiornamento texture
	if (forced || !ppu.skip_draw) {
		const _texture_simple *scrtex = &d3d9.screen.tex[d3d9.screen.index];
		LPDIRECT3DSURFACE9 back_buffer;
		UINT i;

		// screen
		{
			D3DLOCKED_RECT lrect;

			// lock della surface in memoria
			IDirect3DSurface9_LockRect(scrtex->offscreen, &lrect, NULL, D3DLOCK_DISCARD);
			// applico l'effetto
			gfx.filter(screen.data,
					screen.line,
					d3d9.palette,
					gfx.bit_per_pixel,
					lrect.Pitch,
					lrect.pBits,
					gfx.rows,
					gfx.lines,
					scrtex->rect.base.w,
					scrtex->rect.base.h,
					d3d9.scale);
			// unlock della surface in memoria
			IDirect3DSurface9_UnlockRect(scrtex->offscreen);
			// aggiorno la texture dello schermo
			IDirect3DDevice9_UpdateSurface(d3d9.adapter->dev, scrtex->offscreen, NULL,
					scrtex->map0, NULL);
		}

		IDirect3DDevice9_GetRenderTarget(d3d9.adapter->dev, 0, &back_buffer);

		// texture
		for (i = 0; i < shader_effect.pass; i++) {
			const _texture *texture = &d3d9.texture[i];
			const _shader_pass *sp = &shader_effect.sp[i];
			D3DTEXTUREFILTERTYPE filter = d3d9_shader_filter(sp->linear);

			shader_effect.running_pass = i;

			if (i == shader_effect.last_pass) {
				IDirect3DDevice9_SetRenderTarget(d3d9.adapter->dev, 0, back_buffer);
				// pulisco l'intero schermo
				d3d9_viewport_set(0, 0, gfx.w[VIDEO_MODE], gfx.h[VIDEO_MODE]);
			} else {
				IDirect3DDevice9_SetRenderTarget(d3d9.adapter->dev, 0, texture->map0);
				// pulisco l'fbo
				d3d9_viewport_set(0, 0, texture->rect.w, texture->rect.h);
			}
			IDirect3DDevice9_Clear(d3d9.adapter->dev, 0, NULL, D3DCLEAR_TARGET,
					D3DCOLOR_ARGB(255, 0, 0, 0), 1.0f, 0);

			// ora setto il viewport corretto
			d3d9_viewport_set(texture->vp.x, texture->vp.y, texture->vp.w, texture->vp.h);

			cgD3D9BindProgram(texture->shader.prg.f);
			cgD3D9BindProgram(texture->shader.prg.v);

			if (i == 0) {
				IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0,
						(IDirect3DBaseTexture9 * ) scrtex->data);
			} else {
				IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0,
						(IDirect3DBaseTexture9 * ) d3d9.texture[i - 1].data);
			}

			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_MAGFILTER, filter);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_MINFILTER, filter);

			IDirect3DDevice9_SetVertexDeclaration(d3d9.adapter->dev, texture->shader.vd);

			d3d9_shader_param_set(texture, sp->frame_count_mod, ppu.frames);

			IDirect3DDevice9_BeginScene(d3d9.adapter->dev);
			IDirect3DDevice9_DrawPrimitive(d3d9.adapter->dev, D3DPT_TRIANGLESTRIP, 0, 2);
			IDirect3DDevice9_EndScene(d3d9.adapter->dev);
		}

		IDirect3DSurface9_Release(back_buffer);

		d3d9.screen.index = ((d3d9.screen.index + 1) % d3d9.screen.in_use);

		if (d3d9.feedback.in_use) {
			LPDIRECT3DTEXTURE9 data = d3d9.feedback.tex.data;
			LPDIRECT3DSURFACE9 map0 = d3d9.feedback.tex.map0;

			d3d9.feedback.tex.data = d3d9.texture[shader_effect.feedback_pass].data;
			d3d9.feedback.tex.map0 = d3d9.texture[shader_effect.feedback_pass].map0;
			d3d9.texture[shader_effect.feedback_pass].data = data;
			d3d9.texture[shader_effect.feedback_pass].map0 = map0;
		}

		// rendering del testo
		text_rendering(TRUE);

		// text
		if (cfg->txt_on_screen && text.on_screen) {
			// aggiorno la texture del testo
			IDirect3DDevice9_UpdateSurface(d3d9.adapter->dev, d3d9.text.offscreen, NULL,
					d3d9.text.map0, NULL);

			d3d9_viewport_set(0, 0, d3d9.text.rect.w, d3d9.text.rect.h);

			cgD3D9BindProgram(d3d9.text.shader.prg.f);
			cgD3D9BindProgram(d3d9.text.shader.prg.v);

			IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0,
					(IDirect3DBaseTexture9 *) d3d9.text.data);

			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_MAGFILTER,
					D3DTEXF_POINT);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_MINFILTER,
					D3DTEXF_POINT);

			IDirect3DDevice9_SetVertexDeclaration(d3d9.adapter->dev, d3d9.text.shader.vd);

			d3d9_shader_params_text_set(&d3d9.text.shader);

			IDirect3DDevice9_SetRenderState(d3d9.adapter->dev, D3DRS_ALPHABLENDENABLE, TRUE);
			IDirect3DDevice9_BeginScene(d3d9.adapter->dev);
			IDirect3DDevice9_DrawPrimitive(d3d9.adapter->dev, D3DPT_TRIANGLESTRIP, 0, 2);
			IDirect3DDevice9_EndScene(d3d9.adapter->dev);
			IDirect3DDevice9_SetRenderState(d3d9.adapter->dev, D3DRS_ALPHABLENDENABLE, FALSE);
		}

		IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, NULL);

		// swap buffers
		if (IDirect3DDevice9_Present(d3d9.adapter->dev, NULL, NULL, NULL, NULL)
				== D3DERR_DEVICELOST) {
			if (IDirect3DDevice9_TestCooperativeLevel(d3d9.adapter->dev)
					== D3DERR_DEVICENOTRESET) {
				emu_pause(TRUE);

				if (d3d9_context_create() == EXIT_ERROR) {
					fprintf(stderr, "D3D9 : Unable to initialize d3d context\n");
				}

				emu_pause(FALSE);
			}
		}

		// screenshot
		if (gfx.save_screenshot == TRUE) {
			IDirect3DSurface9 *back_buffer, *surface;

			if (IDirect3DDevice9_GetBackBuffer(d3d9.adapter->dev, 0, 0, D3DBACKBUFFER_TYPE_MONO,
					&back_buffer) == D3D_OK) {
				D3DSURFACE_DESC sd;

				IDirect3DSurface9_GetDesc(back_buffer, &sd);

				if (IDirect3DDevice9_CreateOffscreenPlainSurface(d3d9.adapter->dev, sd.Width,
						sd.Height, sd.Format, D3DPOOL_SYSTEMMEM, &surface, NULL) == D3D_OK) {
					if (IDirect3DDevice9_GetRenderTargetData(d3d9.adapter->dev, back_buffer,
							surface) == D3D_OK) {
						D3DLOCKED_RECT lrect;

						IDirect3DSurface9_LockRect(surface, &lrect, NULL, D3DLOCK_DISCARD);
						gui_save_screenshot(sd.Width, sd.Height, lrect.pBits, FALSE);
						IDirect3DSurface9_UnlockRect(surface);
					}
					IDirect3DSurface9_Release(surface);
				}
				IDirect3DSurface9_Release(back_buffer);
			}
			gfx.save_screenshot = FALSE;
		}
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
void gfx_quit(void) {
	ntsc_quit();
	text_quit();

	if (d3d9.palette) {
		free(d3d9.palette);
		d3d9.palette = NULL;
	}

	d3d9_context_delete();

	{
		int i;

		for (i = 0; i < d3d9.adapters_in_use; i++) {
			_d3d9_adapter *dev = D3D9_ADAPTER(i);

			if (dev->dev) {
				IDirect3DDevice9_Release(dev->dev);
				dev->dev = NULL;
			}

		}
	}

	if (d3d9.d3d) {
		IDirect3D9_Release(d3d9.d3d);
		d3d9.d3d = NULL;
	}

	if (d3d9.array) {
		free(d3d9.array);
		d3d9.array = d3d9.adapter = NULL;
	}
}

void gfx_cursor_init(void) {
	gui_cursor_init();
	gui_cursor_set();
};
void gfx_cursor_quit(void) {};
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
void gfx_text_rect_fill(_txt_element *ele, _rect *rect, uint32_t color) {
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
	int w, h;

	if (!d3d9.text.data) {
		return;
	}

	dst.left = ele->x;
	dst.top = ele->y;
	dst.right = ele->x + ele->w;
	dst.bottom = ele->y + ele->h;

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
void gfx_text_blit(_txt_element *ele, _rect *rect) {
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

static void d3d9_shader_cg_error_handler(void) {
	CGerror error = cgGetError();

	if (error == (CGerror) cgD3D9Failed) {
		fprintf(stderr, "D3D9: Error '%s' occurred.\n",
				cgD3D9TranslateHRESULT(cgD3D9GetLastError()));
	} else {
		fprintf(stderr, "CG: Error '%s' occurred.\n", cgD3D9TranslateCGerror(error));
	}
}
static BYTE d3d9_device_create(UINT width, UINT height) {
	D3DPRESENT_PARAMETERS d3dpp;

	if (d3d9.adapter->dev) {
		IDirect3DDevice9_Release(d3d9.adapter->dev);
		d3d9.adapter->dev = NULL;
	}

	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = gui_screen_id();
	d3dpp.BackBufferCount = 2;
	d3dpp.BackBufferFormat = d3d9.adapter->display_mode.Format;
	d3dpp.BackBufferWidth = width;
	d3dpp.BackBufferHeight = height;
	if (cfg->vsync == TRUE) {
		//d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	} else {
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	if (IDirect3D9_CreateDevice(d3d9.d3d,
			d3d9.adapter->id,
			D3DDEVTYPE_HAL,
			gui_screen_id(),
			d3d9.adapter->flags | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
			&d3dpp,
			&d3d9.adapter->dev) != D3D_OK) {
		MessageBox(NULL, "Unable to create d3d device", "Error!",
				MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
static BYTE d3d9_context_create(void) {
	D3DXMATRIX identity;
	WORD w, h;
	UINT i;

	d3d9_context_delete();

	if (d3d9_device_create(gfx.w[VIDEO_MODE], gfx.h[VIDEO_MODE]) == EXIT_ERROR) {
		d3d9_context_delete();
		return (EXIT_ERROR);
	}

	if ((d3d9.cgctx = cgCreateContext()) == NULL) {
		d3d9_context_delete();
		return (EXIT_ERROR);
	}

	cgSetErrorCallback(d3d9_shader_cg_error_handler);

	cgD3D9SetDevice(d3d9.adapter->dev);

	if ((cfg->filter == NO_FILTER) || (cfg->filter >= FLTSHDSTART)) {
		w = gfx.rows;
		h = gfx.lines;
	} else {
		w = gfx.w[CURRENT];
		h = gfx.h[CURRENT];
	}

	D3DXMatrixIdentity(&identity);
	IDirect3DDevice9_SetTransform(d3d9.adapter->dev, D3DTS_WORLD, &identity);
	IDirect3DDevice9_SetTransform(d3d9.adapter->dev, D3DTS_VIEW, &identity);

	// screen
	d3d9_texture_simple_create(&d3d9.screen.tex[0], w, h, FALSE);

	// lut (devo farlo prima di elaborare le shaders)
	for (i = 0; i < shader_effect.luts; i++) {
		if (d3d9_texture_lut_create(&d3d9.lut[i], i) == EXIT_ERROR) {
			d3d9_context_delete();
			return (EXIT_ERROR_SHADER);
		}
	}

	// devo precalcolarmi il viewport finale
	{
		_viewport *vp = &gfx.vp;

		vp->x = 0;
		vp->y = 0;
		vp->w = gfx.w[VIDEO_MODE];
		vp->h = gfx.h[VIDEO_MODE];

		// configuro l'aspect ratio del fullscreen
		if (cfg->fullscreen && !cfg->stretch) {
			FLOAT ratio_surface = (((FLOAT)(gfx.w[CURRENT] / cfg->scale) * gfx.pixel_aspect_ratio)
					/ (FLOAT)(gfx.h[CURRENT] / cfg->scale));
			FLOAT ratio_frame = (FLOAT) gfx.w[VIDEO_MODE] / (FLOAT) gfx.h[VIDEO_MODE];

			if (ratio_frame > ratio_surface) {
				vp->w = (int) ((FLOAT) gfx.h[VIDEO_MODE] * ratio_surface);
				vp->x = (int) (((FLOAT) gfx.w[VIDEO_MODE] - (FLOAT) vp->w) * 0.5f);
			} else {
				vp->h = (int) ((FLOAT) gfx.w[VIDEO_MODE] * ratio_surface);
				vp->y = (int) (((FLOAT) gfx.h[VIDEO_MODE] - (FLOAT) vp->h) * 0.5f);
			}
		}
	}

	// texture
	for (i = 0; i < shader_effect.pass; i++) {
		fprintf(stderr, "D3D9: Setting pass %d.\n", i);

		if (d3d9_texture_create(&d3d9.texture[i], i) == EXIT_ERROR) {
			d3d9_context_delete();
			return (EXIT_ERROR);
		}

		if (d3d9_shader_init(i, &d3d9.texture[i].shader, shader_effect.sp[i].path,
				shader_effect.sp[i].code) == EXIT_ERROR) {
			d3d9_context_delete();
			return (EXIT_ERROR_SHADER);
		}
	}

	// PREV (calcolo il numero di screen da utilizzare)
	// deve essere fatto dopo il d3d9_shader_init().
	for (i = 0; i < shader_effect.pass; i++) {
		UINT a;

		for (a = 0; a < LENGTH(d3d9.texture[i].shader.uni.prev); a++) {
			if (d3d9.texture[i].shader.uni.prev[a].f.texture) {
				if (d3d9.screen.in_use < (a + 1)) {
					d3d9.screen.in_use = (a + 1);
				}
			}
		}
	}

	d3d9.screen.in_use++;

	// PREV
	for (i = 1; i < d3d9.screen.in_use; i++) {
		d3d9_texture_simple_create(&d3d9.screen.tex[i], w, h, FALSE);
	}

	// FEEDBACK
	if ((shader_effect.feedback_pass >= 0) && (shader_effect.feedback_pass < shader_effect.pass)) {
		d3d9.feedback.in_use = TRUE;

		if (d3d9_texture_create(&d3d9.feedback.tex, shader_effect.feedback_pass) == EXIT_ERROR) {
			d3d9_context_delete();
			return (EXIT_ERROR);
		}
	}

	// testo
	{
		// setto il necessario per il blending
		IDirect3DDevice9_SetRenderState(d3d9.adapter->dev, D3DRS_BLENDOP, D3DBLENDOP_ADD);
		IDirect3DDevice9_SetRenderState(d3d9.adapter->dev, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		IDirect3DDevice9_SetRenderState(d3d9.adapter->dev, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		d3d9_texture_simple_create(&d3d9.text, gfx.w[VIDEO_MODE], gfx.h[VIDEO_MODE], TRUE);

		fprintf(stderr, "D3D9: Setting text pass.\n");

		if (d3d9_shader_init(0, &d3d9.text.shader, NULL, shader_code_blend()) == EXIT_ERROR) {
			d3d9_context_delete();
			return (EXIT_ERROR);
		}
	}

	// setto tutto quello che mi serve per il rendering
	for (i = 0; i < shader_effect.pass; i++) {
		_texture *texture = &d3d9.texture[i];
		_shader *shd = &texture->shader;
		_texture_rect *prev = NULL;

		if (i == 0) {
			prev = &d3d9.screen.tex[0].rect;
		} else {
			prev = &d3d9.texture[i - 1].rect;
		}

		shd->info.video_size.x = (FLOAT) prev->base.w;
		shd->info.video_size.y = (FLOAT) prev->base.h;
		shd->info.texture_size.x = prev->w,
		shd->info.texture_size.y = prev->h;
		shd->info.output_size.x = (FLOAT) texture->vp.w;
		shd->info.output_size.y = (FLOAT) texture->vp.h;

		d3d9_vertex_buffer_set(shd, &texture->vp, prev);
	}

	memcpy(gfx.last_shader_file, cfg->shader_file, sizeof(gfx.last_shader_file));

	return (EXIT_OK);
}
static void d3d9_context_delete(void) {
	UINT i;

	d3d9.screen.in_use = 0;
	d3d9.screen.index = 0;

	for (i = 0; i < LENGTH(d3d9.screen.tex); i++) {
		_texture_simple *texture = &d3d9.screen.tex[i];
		if (texture->offscreen) {
			IDirect3DSurface9_Release(texture->offscreen);
			texture->offscreen = NULL;
		}
		if (texture->map0) {
			IDirect3DSurface9_Release(texture->map0);
			texture->map0 = NULL;
		}
		if (texture->data) {
			IDirect3DTexture9_Release(texture->data);
			texture->data = NULL;
		}
		if (texture->shader.vd) {
			IDirect3DVertexDeclaration9_Release(texture->shader.vd);
			texture->shader.vd = NULL;
		}
		if (texture->shader.quad) {
			IDirect3DVertexBuffer9_Release(texture->shader.quad);
			texture->shader.quad = NULL;
		}
	}

	{
		if (d3d9.text.offscreen) {
			IDirect3DSurface9_Release(d3d9.text.offscreen);
			d3d9.text.offscreen = NULL;
		}
		if (d3d9.text.map0) {
			IDirect3DSurface9_Release(d3d9.text.map0);
			d3d9.text.map0 = NULL;
		}
		if (d3d9.text.data) {
			IDirect3DTexture9_Release(d3d9.text.data);
			d3d9.text.data = NULL;
		}
		if (d3d9.text.shader.vd) {
			IDirect3DVertexDeclaration9_Release(d3d9.text.shader.vd);
			d3d9.text.shader.vd = NULL;
		}
		if (d3d9.text.shader.quad) {
			IDirect3DVertexBuffer9_Release(d3d9.text.shader.quad);
			d3d9.text.shader.quad = NULL;
		}
		d3d9_shader_delete(&d3d9.text.shader);
	}

	{
		d3d9.feedback.in_use = FALSE;

		if (d3d9.feedback.tex.map0) {
			IDirect3DSurface9_Release(d3d9.feedback.tex.map0);
			d3d9.feedback.tex.map0 = NULL;
		}
		if (d3d9.feedback.tex.data) {
			IDirect3DTexture9_Release(d3d9.feedback.tex.data);
			d3d9.feedback.tex.data = NULL;
		}
		if (d3d9.feedback.tex.shader.quad) {
			IDirect3DVertexBuffer9_Release(d3d9.feedback.tex.shader.quad);
			d3d9.feedback.tex.shader.quad = NULL;
		}
		if (d3d9.feedback.tex.shader.vd) {
			IDirect3DVertexDeclaration9_Release(d3d9.feedback.tex.shader.vd);
			d3d9.feedback.tex.shader.vd = NULL;
		}
		d3d9_shader_delete(&d3d9.feedback.tex.shader);
	}

	for (i = 0; i < LENGTH(d3d9.texture); i++) {
		_texture *texture = &d3d9.texture[i];

		if (texture->map0) {
			IDirect3DSurface9_Release(texture->map0);
			texture->map0 = NULL;
		}
		if (texture->data) {
			IDirect3DTexture9_Release(texture->data);
			texture->data = NULL;
		}
		if (texture->shader.quad) {
			IDirect3DVertexBuffer9_Release(texture->shader.quad);
			texture->shader.quad = NULL;
		}
		if (texture->shader.vd) {
			IDirect3DVertexDeclaration9_Release(texture->shader.vd);
			texture->shader.vd = NULL;
		}
		d3d9_shader_delete(&d3d9.texture[i].shader);
	}

	for (i = 0; i < LENGTH(d3d9.lut); i++) {
		if (d3d9.lut[i].data) {
			IDirect3DTexture9_Release(d3d9.lut[i].data);
			d3d9.lut[i].data = NULL;
		}
	}

	if (d3d9.cgctx) {
		cgD3D9UnloadAllPrograms();
		cgD3D9SetDevice(NULL);
		cgDestroyContext(d3d9.cgctx);
		d3d9.cgctx = NULL;
	}

	if (d3d9.adapter && d3d9.adapter->dev) {
		IDirect3DDevice9_Release(d3d9.adapter->dev);
		d3d9.adapter->dev = NULL;
	}
}
static BYTE d3d9_texture_create(_texture *texture, UINT index) {
	_shader_pass *sp = &shader_effect.sp[index];
	_shader_scale *sc = &sp->sc;
	const _texture_rect *prev;
	_texture_rect *rect = &texture->rect;
	_viewport *vp = &texture->vp;

	if (index == 0) {
		prev = &d3d9.screen.tex[0].rect;
	} else {
		prev = &d3d9.texture[index - 1].rect;
	}

	if (index == shader_effect.last_pass) {
		sc->scale.x = 1.0f;
		sc->scale.y = 1.0f;
		sc->type.x = SHADER_SCALE_VIEWPORT;
		sc->type.y = SHADER_SCALE_VIEWPORT;
	}

#if defined (FH_SHADERS_GEST)
	switch (sc->type.x) {
		case SHADER_SCALE_DEFAULT:
		case SHADER_SCALE_INPUT:
			rect->base.w = (FLOAT) prev->base.w * sc->scale.x;
			break;
		case SHADER_SCALE_ABSOLUTE:
			rect->base.w = sc->abs.x;
			break;
		case SHADER_SCALE_VIEWPORT:
			rect->base.w = (FLOAT) gfx.vp.w * sc->scale.x;
			break;
	}
	switch (sc->type.y) {
		case SHADER_SCALE_DEFAULT:
		case SHADER_SCALE_INPUT:
			rect->base.h = (FLOAT) prev->base.h * sc->scale.y;
			break;
		case SHADER_SCALE_ABSOLUTE:
			rect->base.h = sc->abs.y;
			break;
		case SHADER_SCALE_VIEWPORT:
			rect->base.h = (FLOAT) gfx.vp.h * sc->scale.y;
			break;
	}

	rect->w = emu_power_of_two(rect->base.w);
	rect->h = emu_power_of_two(rect->base.h);
#else
	switch (sc->type.x) {
		case SHADER_SCALE_DEFAULT:
		case SHADER_SCALE_INPUT:
			rect->w = (FLOAT) prev->w * sc->scale.x;
			rect->base.w = (FLOAT) prev->base.w * sc->scale.x;
			break;
		case SHADER_SCALE_ABSOLUTE:
			rect->w = rect->base.w = sc->abs.x;
			break;
		case SHADER_SCALE_VIEWPORT:
			rect->w = rect->base.w = (FLOAT) gfx.vp.w * sc->scale.x;
			break;
	}
	switch (sc->type.y) {
		case SHADER_SCALE_DEFAULT:
		case SHADER_SCALE_INPUT:
			rect->h = (FLOAT) prev->h * sc->scale.y;
			rect->base.h = (FLOAT) prev->base.h * sc->scale.y;
			break;
		case SHADER_SCALE_ABSOLUTE:
			rect->h = rect->base.h = sc->abs.y;
			break;
		case SHADER_SCALE_VIEWPORT:
			rect->h = rect->base.h = (FLOAT) gfx.vp.h * sc->scale.y;
			break;
	}

	rect->w = emu_power_of_two(rect->w);
	rect->h = emu_power_of_two(rect->h);
#endif

	// se la scheda video supporta solo texture quadre allore devo crearle quadre
	if (d3d9.adapter->texture_square_only == TRUE) {
		if (rect->w < rect->h) {
			rect->w = rect->h;
		} else {
			rect->h = rect->w;
		}
	}

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

	if (IDirect3DDevice9_CreateVertexBuffer(d3d9.adapter->dev, sizeof(_vertex_buffer) * 4, 0, 0,
			D3DPOOL_DEFAULT, &texture->shader.quad, NULL) != D3D_OK) {
		MessageBox(NULL, "Unable to create the vertex buffer", "Error!",
				MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	if (IDirect3DDevice9_CreateTexture(d3d9.adapter->dev,
			(UINT) rect->w, (UINT) rect->h, 1,
			D3DUSAGE_RENDERTARGET,
			sp->fbo_flt ? D3DFMT_A32B32G32R32F : D3DFMT_A8R8G8B8,
			D3DPOOL_DEFAULT,
			&texture->data,
			NULL) != D3D_OK) {
		MessageBox(NULL, "Unable to create the texture", "Error!",
				MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	IDirect3DTexture9_GetSurfaceLevel(texture->data, 0, &texture->map0);
	// cancello la superficie map0 perche' alcuni driver (tipo intel) nella
	// versione per windows XP non mi passano una superficia "pulita".
	d3d9_surface_clean(&texture->map0, rect->w, rect->h);

	IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, (IDirect3DBaseTexture9 *) texture->data);
	IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
	IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
	IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, NULL);

	return (EXIT_OK);
}
static BYTE d3d9_texture_simple_create(_texture_simple *texture, UINT w, UINT h, BOOL text) {
	_texture_rect *rect = &texture->rect;
	_shader *shd = &texture->shader;
	_viewport vp = { 0, 0, w, h };
	UINT flt;

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
		flt = (cfg->interpolation || gfx.PSS) ? D3DTEXF_LINEAR : D3DTEXF_POINT;
	} else {
		rect->w = rect->base.w;
		rect->h = rect->base.h;
		flt = D3DTEXF_POINT;
	}

	// se la scheda video supporta solo texture quadre allore devo crearle quadre
	if (d3d9.adapter->texture_square_only == TRUE) {
		if (rect->w < rect->h) {
			rect->w = rect->h;
		} else {
			rect->h = rect->w;
		}
	}

	shd->info.video_size.x = (FLOAT) rect->base.w;
	shd->info.video_size.y = (FLOAT) rect->base.h;
	shd->info.texture_size.x = rect->w;
	shd->info.texture_size.y = rect->h;

	if (IDirect3DDevice9_CreateVertexBuffer(d3d9.adapter->dev,
			sizeof(_vertex_buffer) * 4,
			D3DUSAGE_WRITEONLY,
			0,
			D3DPOOL_DEFAULT,
			&texture->shader.quad,
			NULL) != D3D_OK) {
		MessageBox(NULL, "Unable to create the vertex buffer", "Error!",
				MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	if (IDirect3DDevice9_CreateTexture(d3d9.adapter->dev,
			(UINT) rect->w, (UINT) rect->h, 1,
			d3d9.adapter->dynamic_texture ? D3DUSAGE_DYNAMIC : 0,
			D3DFMT_A8R8G8B8,
			D3DPOOL_DEFAULT,
			&texture->data,
			NULL) != D3D_OK) {
		MessageBox(NULL, "Unable to create the texture", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	IDirect3DTexture9_GetSurfaceLevel(texture->data, 0, &texture->map0);
	// cancello la superficie map0 perche' alcuni driver (tipo intel) nella
	// versione per windows XP non mi passano una superficia "pulita".
	d3d9_surface_clean(&texture->map0, rect->w, rect->h);

	// creo la superficie temporanea le cui dimensioni non devono essere "POWerate"
	if (IDirect3DDevice9_CreateOffscreenPlainSurface(d3d9.adapter->dev,
			rect->base.w,
			rect->base.h,
			D3DFMT_A8R8G8B8,
			D3DPOOL_SYSTEMMEM,
			&texture->offscreen,
			NULL) != D3D_OK) {
		MessageBox(NULL, "Unable to create the memory surface", "Error!",
				MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	// cancello la superficie
	d3d9_surface_clean(&texture->offscreen, rect->base.w, rect->base.h);

	IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, (IDirect3DBaseTexture9 *) texture->data);
	IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
	IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
	IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_MINFILTER, flt);
	IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_MAGFILTER, flt);
	IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, NULL);

	d3d9_vertex_buffer_set(shd, &vp, rect);

	return (EXIT_OK);
}
static BYTE d3d9_texture_lut_create(_lut *lut, UINT index) {
	_lut_pass *lp = &shader_effect.lp[index];
	LPDIRECT3DSURFACE9 map0, offscreen;
	D3DLOCKED_RECT lrect;
	UINT width, height;

	if (gui_load_lut(lut, lp->path) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	width = lut->w;
	height = lut->h;

	lut->name = lp->name;
	lut->filter = d3d9_shader_filter(lp->linear);

	if (d3d9.adapter->texture_square_only == TRUE) {
		if (width < height) {
			width = height;
		} else {
			height = width;
		}
	}

	if (IDirect3DDevice9_CreateTexture(d3d9.adapter->dev,
			width, height, 1,
			0,
			D3DFMT_A8R8G8B8,
			D3DPOOL_DEFAULT,
			&lut->data,
			NULL) != D3D_OK) {
		MessageBox(NULL, "Unable to create the texture", "Error!",
				MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	if (IDirect3DDevice9_CreateOffscreenPlainSurface(d3d9.adapter->dev,
			lut->w, lut->h,
			D3DFMT_A8R8G8B8,
			D3DPOOL_SYSTEMMEM,
			&offscreen,
			NULL) != D3D_OK) {
		MessageBox(NULL, "Unable to create the memory surface", "Error!",
				MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	IDirect3DTexture9_GetSurfaceLevel(lut->data, 0, &map0);

	if (IDirect3DSurface9_LockRect(offscreen, &lrect, NULL, D3DLOCK_DISCARD) != D3D_OK) {
		if (map0) {
			IDirect3DSurface9_Release(map0);
		}
		if (offscreen) {
			IDirect3DSurface9_Release(offscreen);
		}
		return (EXIT_ERROR);
	} else {
		UINT w = 0, h = 0;
		uint32_t *sbits = (uint32_t *) lut->bits;
		uint32_t *dbits = (uint32_t *) lrect.pBits;

		for (h = 0; h < lut->h; h++) {
			for (w = 0; w < lut->w; w++) {
				(*(dbits + w)) = (*(sbits + w));
			}
			sbits += lrect.Pitch / (gfx.bit_per_pixel / 8);
			dbits += lrect.Pitch / (gfx.bit_per_pixel / 8);
		}

		IDirect3DSurface9_UnlockRect(offscreen);
		IDirect3DDevice9_UpdateSurface(d3d9.adapter->dev, offscreen, NULL, map0, NULL);

		IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, (IDirect3DBaseTexture9 *) lut->data);
		IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
		IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
		IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, NULL);

		IDirect3DSurface9_Release(map0);
		IDirect3DSurface9_Release(offscreen);
	}

	return (EXIT_OK);
}
static void d3d9_surface_clean(LPDIRECT3DSURFACE9 *surface, UINT width, UINT height) {
	D3DLOCKED_RECT lock_dst;

	if (IDirect3DSurface9_LockRect((*surface), &lock_dst, NULL, D3DLOCK_DISCARD) == D3D_OK) {
		uint32_t *pbits;
		int w, h;

		pbits = (uint32_t *) lock_dst.pBits;

		for (h = 0; h < height; h++) {
			for (w = 0; w < width; w++) {
				(*(pbits + w)) = 0;
			}
			pbits += lock_dst.Pitch / (gfx.bit_per_pixel / 8);
		}

		IDirect3DSurface9_UnlockRect((*surface));
	}
}
static BYTE d3d9_shader_init(UINT pass, _shader *shd, const char *path, const char *code) {
	const char *list;
	const char *argv[128];
	const char **fopts = cgD3D9GetOptimalOptions(cgD3D9GetLatestPixelProfile());
	const char **vopts = cgD3D9GetOptimalOptions(cgD3D9GetLatestVertexProfile());
	char alias[MAX_PASS][128];
	UINT i, argc;

	// fragment
	{
		memset(alias, 0x00, sizeof(alias));
		memset(argv, 0x00, sizeof(argv));

		argc = 0;
		argv[argc++] = "-DPARAMETER_UNIFORM";
		for (i = 0; i < pass; i++) {
			_shader_pass *sp = &shader_effect.sp[i];

			if (sp->alias[0]) {
				snprintf(alias[i], 128, "-D%s_ALIAS", sp->alias);
				argv[argc++] = alias[i];
			}
		}
		for (i = 0; i < 128; i++) {
			if (!fopts[i]) {
				break;
			}
			argv[argc] = fopts[i];

			if (argc++ == 126) {
				break;
			}
		}
		argv[argc] = NULL;

		if ((path == NULL) || !path[0]) {
			shd->prg.f = cgCreateProgram(d3d9.cgctx, CG_SOURCE, code,
					cgD3D9GetLatestPixelProfile(), "main_fragment", argv);
		} else {
			shd->prg.f = cgCreateProgramFromFile(d3d9.cgctx, CG_SOURCE, path,
					cgD3D9GetLatestPixelProfile(), "main_fragment", argv);
		}
		if (!shd->prg.f && (list = cgGetLastListing(d3d9.cgctx))) {
			printf("CG: fragment program errors :\n%s\n", list);
		}
	}

	// vertex
	{
		memset(alias, 0x00, sizeof(alias));
		memset(argv, 0x00, sizeof(argv));

		argc = 0;
		argv[argc++] = "-DPARAMETER_UNIFORM";
		for (i = 0; i < pass; i++) {
			_shader_pass *sp = &shader_effect.sp[i];

			if (sp->alias[0]) {
				snprintf(alias[i], 128, "-D%s_ALIAS", sp->alias);
				argv[argc++] = alias[i];
			}
		}
		for (i = 0; i < 128; i++) {
			if (!vopts[i]) {
				break;
			}
			argv[argc] = vopts[i];

			if (argc++ == 126) {
				break;
			}
		}
		argv[argc] = NULL;

		if ((path == NULL) || !path[0]) {
			shd->prg.v = cgCreateProgram(d3d9.cgctx, CG_SOURCE, code,
					cgD3D9GetLatestVertexProfile(), "main_vertex", argv);
		} else {
			shd->prg.v = cgCreateProgramFromFile(d3d9.cgctx, CG_SOURCE, path,
					cgD3D9GetLatestVertexProfile(), "main_vertex", argv);
		}
		if (!shd->prg.v && (list = cgGetLastListing(d3d9.cgctx))) {
			printf("CG: vertex program errors :\n%s\n", list);
		}
	}

	if (!shd->prg.f || !shd->prg.v) {
		fprintf(stderr, "CG: %s\n", cgGetErrorString(cgGetError()));
		return (EXIT_ERROR);
	}

	if (cgD3D9LoadProgram(shd->prg.f, TRUE, 0) != D3D_OK) {
		fprintf(stderr, "CG: Error on loading fragment program\n");
		return (EXIT_ERROR);
	}
	if (cgD3D9LoadProgram(shd->prg.v, TRUE, 0) != D3D_OK) {
		fprintf(stderr, "CG: Error on loading vertex program\n");
		return (EXIT_ERROR);
	}

	shd->uni.mvp = cgGetNamedParameter(shd->prg.v, "modelViewProj");
	if (!shd->uni.mvp) {
		shd->uni.mvp = cgGetNamedParameter(shd->prg.v, "IN.mvp_matrix");
	}

	shd->uni.v.video_size = cgGetNamedParameter(shd->prg.v, "IN.video_size");
	shd->uni.f.video_size = cgGetNamedParameter(shd->prg.f, "IN.video_size");
	shd->uni.v.texture_size = cgGetNamedParameter(shd->prg.v, "IN.texture_size");
	shd->uni.f.texture_size = cgGetNamedParameter(shd->prg.f, "IN.texture_size");
	shd->uni.v.output_size = cgGetNamedParameter(shd->prg.v, "IN.output_size");
	shd->uni.f.output_size = cgGetNamedParameter(shd->prg.f, "IN.output_size");

	shd->uni.v.frame_count = cgGetNamedParameter(shd->prg.v, "IN.frame_count");
	shd->uni.f.frame_count = cgGetNamedParameter(shd->prg.f, "IN.frame_count");

	shd->uni.v.frame_direction = cgGetNamedParameter(shd->prg.v, "IN.frame_direction");
	shd->uni.f.frame_direction = cgGetNamedParameter(shd->prg.f, "IN.frame_direction");

	if (d3d9_vertex_declaration_create(shd) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	for (i = 0; i < shader_effect.luts; i++) {
		shd->uni.v.lut[i] = cgGetNamedParameter(shd->prg.v, d3d9.lut[i].name);
		shd->uni.f.lut[i] = cgGetNamedParameter(shd->prg.f, d3d9.lut[i].name);
	}

	for (i = 0; i < shader_effect.params; i++) {
		shd->uni.v.param[i] = cgGetNamedParameter(shd->prg.v, shader_effect.param[i].name);
		shd->uni.f.param[i] = cgGetNamedParameter(shd->prg.f, shader_effect.param[i].name);
	}

	d3d9_shader_uni_texture_clear(&shd->uni.orig);
	d3d9_shader_uni_texture(&shd->uni.orig, &shd->prg, "ORIG");
	d3d9_shader_uni_texture(&shd->uni.orig, &shd->prg, "PASSPREV%u", pass + 1);

	d3d9_shader_uni_texture_clear(&shd->uni.feedback);
	d3d9_shader_uni_texture(&shd->uni.feedback, &shd->prg, "FEEDBACK");

	for (i = 0; i < pass; i++) {
		d3d9_shader_uni_texture_clear(&shd->uni.passprev[i]);

		d3d9_shader_uni_texture(&shd->uni.passprev[i], &shd->prg, "PASS%u", i + 1);
		d3d9_shader_uni_texture(&shd->uni.passprev[i], &shd->prg, "PASSPREV%u", pass - i);

		if (shader_effect.sp[i].alias[0]) {
			d3d9_shader_uni_texture(&shd->uni.passprev[i], &shd->prg,
					shader_effect.sp[i].alias);
		}
	}

	d3d9_shader_uni_texture_clear(&shd->uni.prev[0]);
	d3d9_shader_uni_texture(&shd->uni.prev[0], &shd->prg, "PREV");

	for (i = 1; i < LENGTH(shd->uni.prev); i++) {
		d3d9_shader_uni_texture_clear(&shd->uni.prev[i]);
		d3d9_shader_uni_texture(&shd->uni.prev[i], &shd->prg, "PREV%u", i);
	}

	return (EXIT_OK);
}
static void d3d9_shader_delete(_shader *shd) {
	if (shd->prg.f) {
		cgDestroyProgram(shd->prg.f);
		shd->prg.f = NULL;
	}
	if (shd->prg.v) {
		cgDestroyProgram(shd->prg.v);
		shd->prg.v = NULL;
	}
}
static void d3d9_shader_uniform_ctrl(CGparameter *dst, CGparameter *param, const char *semantic) {
	static const FLOAT f2[2] = {1.0f, 1.0f};

	if (!(*param)) {
		return;
	}

	if (cgD3D9SetUniform((*param), &f2) != D3D_OK) {
		(*dst) = 0;
		fprintf(stderr, "CG: Parameter \"%s\" disabled.\n", semantic);
	} else {
		(*dst) = (*param);
	}
}
static void d3d9_shader_uni_texture_clear(_shader_uniforms_tex *sut) {
	sut->f.texture = NULL;
	sut->v.video_size = NULL;
	sut->f.video_size = NULL;
	sut->v.texture_size = NULL;
	sut->f.texture_size = NULL;
	sut->v.tex_coord = NULL;
}
static void d3d9_shader_uni_texture(_shader_uniforms_tex *sut, _shader_prg_cg *prg, char *fmt, ...) {
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
		d3d9_shader_uniform_ctrl(&sut->v.video_size, &param, buff);
	}
	if (!sut->f.video_size) {
		param = cgGetNamedParameter(prg->f, buff);
		d3d9_shader_uniform_ctrl(&sut->f.video_size, &param, buff);
	}
	snprintf(buff, sizeof(buff), "%s%s", type, ".texture_size");
	if (!sut->v.texture_size) {
		param = cgGetNamedParameter(prg->v, buff);
		d3d9_shader_uniform_ctrl(&sut->v.texture_size, &param, buff);
	}
	if (!sut->f.texture_size) {
		param = cgGetNamedParameter(prg->f, buff);
		d3d9_shader_uniform_ctrl(&sut->f.texture_size, &param, buff);
	}
	snprintf(buff, sizeof(buff), "%s%s", type, ".tex_coord");
	if (!sut->v.tex_coord) {
		sut->v.tex_coord = cgGetNamedParameter(prg->v, buff);
	}
}
static BYTE d3d9_vertex_declaration_create(_shader *shd) {
	UINT i, count, index, tex_index = 0;
	CGparameter param;
	BYTE texcoord[2] = { FALSE };
	BYTE stream[4] = { FALSE };
	BYTE indices[LENGTH(shd->attribs.attrib)] = { FALSE };
	static const D3DVERTEXELEMENT9 end = D3DDECL_END();
	D3DVERTEXELEMENT9 decl[MAXD3DDECLLENGTH];

	if (cgD3D9GetVertexDeclaration(shd->prg.v, decl) == CG_FALSE) {
		return (EXIT_ERROR);
	}

	for (count = 0; count < MAXD3DDECLLENGTH; count++) {
		if (memcmp(&end, &decl[count], sizeof(end)) == 0) {
			break;
		}
	}

	if (!(param = d3d9_cg_find_param(cgGetFirstParameter(shd->prg.v, CG_PROGRAM), "POSITION"))) {
		param = d3d9_cg_find_param(cgGetFirstParameter(shd->prg.v, CG_PROGRAM), "POSITION0");
	}
	if (param && (indices[cgGetParameterResourceIndex(param)] == FALSE)) {
		stream[0] = TRUE;
		index = cgGetParameterResourceIndex(param);
		indices[index] = TRUE;
		decl[index] = (D3DVERTEXELEMENT9) {
			0, 0,
			D3DDECLTYPE_FLOAT3,
			D3DDECLMETHOD_DEFAULT,
			D3DDECLUSAGE_POSITION, 0
		};

		fprintf(stderr, "CG: semantic POSITION found (%d)\n", index);
	}

	if (!(param = d3d9_cg_find_param(cgGetFirstParameter(shd->prg.v, CG_PROGRAM), "TEXCOORD"))) {
		param = d3d9_cg_find_param(cgGetFirstParameter(shd->prg.v, CG_PROGRAM), "TEXCOORD0");
	}
	if (param && (indices[cgGetParameterResourceIndex(param)] == FALSE)) {
		stream[1] = TRUE;
		texcoord[0] = TRUE;
		index = cgGetParameterResourceIndex(param);
		indices[index]  = TRUE;
		decl[index] = (D3DVERTEXELEMENT9) {
			1, sizeof(float) * 3,
			D3DDECLTYPE_FLOAT2,
			D3DDECLMETHOD_DEFAULT,
			D3DDECLUSAGE_TEXCOORD, 0
		};

		fprintf(stderr, "CG: semantic TEXCOORD0 found (%d)\n", index);
	}

	param = d3d9_cg_find_param(cgGetFirstParameter(shd->prg.v, CG_PROGRAM), "TEXCOORD1");
	if (param && (indices[cgGetParameterResourceIndex(param)] == FALSE)) {
		stream[2] = TRUE;
		texcoord[1] = TRUE;
		index = cgGetParameterResourceIndex(param);
		indices[index]  = TRUE;
		decl[index] = (D3DVERTEXELEMENT9) {
			2, sizeof(float) * 5,
			D3DDECLTYPE_FLOAT2,
			D3DDECLMETHOD_DEFAULT,
			D3DDECLUSAGE_TEXCOORD, 1
		};

		fprintf(stderr, "CG: semantic TEXCOORD1 found (%d)\n", index);
	}

	if (!(param = d3d9_cg_find_param(cgGetFirstParameter(shd->prg.v, CG_PROGRAM), "COLOR"))) {
		param = d3d9_cg_find_param(cgGetFirstParameter(shd->prg.v, CG_PROGRAM), "COLOR0");
	}
	if (param && (indices[cgGetParameterResourceIndex(param)] == FALSE)) {
		stream[3] = TRUE;
		index = cgGetParameterResourceIndex(param);
		indices[index] = TRUE;
		decl[index] = (D3DVERTEXELEMENT9) {
			3, sizeof(float) * 7,
					D3DDECLTYPE_FLOAT4,
					D3DDECLMETHOD_DEFAULT,
					D3DDECLUSAGE_COLOR, 0
		};

		fprintf(stderr, "CG: semantic COLOR found (%d)\n", index);
	}

	// Stream {0, 1, 2, 3} might be already taken. Find first vacant stream
	for (index = 0; index < 4; index++) {
		if (stream[index] == FALSE) {
			break;
		}
	}

	// Find first vacant texcoord declaration
	if (texcoord[0] && texcoord[1]) {
		tex_index = 2;
	} else if (texcoord[1] && !texcoord[0]) {
		tex_index = 0;
	} else if (texcoord[0] && !texcoord[1]) {
		tex_index = 1;
	}

	shd->attribs.count = 0;

	for (i = 0; i < count; i++) {
		if (indices[i]) {
			shd->attribs.attrib[shd->attribs.count++] = 0;
		} else {
			fprintf(stderr, "CG: attrib found (%d %d %d %d)\n", i, shd->attribs.count, index,
					tex_index);

			shd->attribs.attrib[shd->attribs.count++] = index;
			decl[i] = (D3DVERTEXELEMENT9) {
				index, sizeof(float) * 3,
				D3DDECLTYPE_FLOAT2,
				D3DDECLMETHOD_DEFAULT,
				D3DDECLUSAGE_TEXCOORD, tex_index
			};
			while ((++index < 4) && stream[index]) {
				index++;
			}
			if ((++tex_index == 1) && texcoord[1]) {
				tex_index++;
			}
		}
	}

	if (IDirect3DDevice9_CreateVertexDeclaration(d3d9.adapter->dev, decl, &shd->vd)) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
static void d3d9_vertex_buffer_set(_shader *shd, _viewport *vp, _texture_rect *prev) {
	D3DXMATRIX proj, ortho, rot;
	FLOAT u = (FLOAT) prev->base.w / prev->w;
	FLOAT v = (FLOAT) prev->base.h / prev->h;
	void *buffer;
	const UINT rotation = 0;
	UINT i;

	shd->vb[0].x = 0.0f;
	shd->vb[0].y = vp->h;
	shd->vb[0].z = 0.5f;
	shd->vb[0].u = 0.0f;
	shd->vb[0].v = 0.0f;
	shd->vb[0].lut_u = 0.0f;
	shd->vb[0].lut_v = 0.0f;
	shd->vb[0].r = 1.0f;
	shd->vb[0].g = 1.0f;
	shd->vb[0].b = 1.0f;
	shd->vb[0].a = 1.0f;

	shd->vb[1].x = vp->w;
	shd->vb[1].y = vp->h;
	shd->vb[1].z = 0.5f;
	shd->vb[1].u = u;
	shd->vb[1].v = 0.0f;
	shd->vb[1].lut_u = 1.0f;
	shd->vb[1].lut_v = 0.0f;
	shd->vb[1].r = 1.0f;
	shd->vb[1].g = 1.0f;
	shd->vb[1].b = 1.0f;
	shd->vb[1].a = 1.0f;

	shd->vb[2].x = 0.0f;
	shd->vb[2].y = 0.0f;
	shd->vb[2].z = 0.5f;
	shd->vb[2].u = 0.0f;
	shd->vb[2].v = v;
	shd->vb[2].lut_u = 0.0f;
	shd->vb[2].lut_v = 1.0f;
	shd->vb[2].r = 1.0f;
	shd->vb[2].g = 1.0f;
	shd->vb[2].b = 1.0f;
	shd->vb[2].a = 1.0f;

	shd->vb[3].x = vp->w;
	shd->vb[3].y = 0.0f;
	shd->vb[3].z = 0.5f;
	shd->vb[3].u = u;
	shd->vb[3].v = v;
	shd->vb[3].lut_u = 1.0f;
	shd->vb[3].lut_v = 1.0f;
	shd->vb[3].r = 1.0f;
	shd->vb[3].g = 1.0f;
	shd->vb[3].b = 1.0f;
	shd->vb[3].a = 1.0f;

	// problema dell'infamous half-texel offset of D3D9 (corretto dalle D3D10 in poi) :
	// http://msdn.microsoft.com/en-us/library/bb219690%28VS.85%29.aspx.
	for (i = 0; i < 4; i++) {
		shd->vb[i].x -= 0.5f;
		shd->vb[i].y += 0.5f;
	}

	IDirect3DVertexBuffer9_Lock(shd->quad, 0, 0, (void**) &buffer, 0);
	memcpy(buffer, shd->vb, sizeof(shd->vb));
	IDirect3DVertexBuffer9_Unlock(shd->quad);

	D3DXMatrixOrthoOffCenterLH(&ortho, 0, vp->w, 0, vp->h, 0, 1);
	D3DXMatrixIdentity(&rot);
	D3DXMatrixRotationZ(&rot, rotation * (M_PI / 2.0));

	D3DXMatrixMultiply(&proj, &ortho, &rot);
	D3DXMatrixTranspose(&shd->mvp, &proj);
}
static CGparameter d3d9_cg_find_param(CGparameter prm, const char *name) {
	UINT i;
	static const char *illegal[] = {
		"IN.",
		"ORIG.",
		"PASS",
		"PREV.",
		"PREV1.", "PREV2.",
		"PREV3.", "PREV4.",
		"PREV5.", "PREV6.",
	};

	for (; prm; prm = cgGetNextParameter(prm)) {
		const char *semantic = NULL;

		if (cgGetParameterType(prm) == CG_STRUCT) {
			CGparameter ret;

			if ((ret = d3d9_cg_find_param(cgGetFirstStructParameter(prm), name))) {
				return (ret);
			}
		}
		if (cgGetParameterDirection(prm) != CG_IN || cgGetParameterVariability(prm) != CG_VARYING) {
			continue;
		}
		if (!(semantic = cgGetParameterSemantic(prm))) {
			continue;
		}
		if (strcmp(name, semantic) == 0) {
			const char *pname = cgGetParameterName(prm);

			if (!pname) {
				return (NULL);
			}

			for (i = 0; i < LENGTH(illegal); i++) {
				if (strstr(pname, illegal[i]) == pname) {
					return (NULL);
				}
			}

			return (prm);
		}
	}
	return (NULL);
}
INLINE static void d3d9_viewport_set(DWORD x, DWORD y, DWORD w, DWORD h) {
	static D3DVIEWPORT9 vp;

	vp.X = x;
	vp.Y = y;
	vp.Width = w;
	vp.Height = h;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	IDirect3DDevice9_SetViewport(d3d9.adapter->dev, &vp);
}
INLINE D3DTEXTUREFILTERTYPE d3d9_shader_filter(UINT type) {
	switch (type) {
		case TEXTURE_LINEAR_ENAB:
			return D3DTEXF_LINEAR;
		case TEXTURE_LINEAR_DISAB:
			return D3DTEXF_POINT;
		default:
			if (cfg->interpolation || gfx.PSS) {
				return D3DTEXF_LINEAR;
			}
			return D3DTEXF_POINT;
	}
}
INLINE static void d3d9_shader_params_text_set(_shader *shd) {
	UINT i;

	if (shd->uni.mvp) {
		// posso tranquillamente utilizzare l'mvp dell'ultimo pass
		cgD3D9SetUniformMatrix(shd->uni.mvp, &shd->mvp);
	}

	for (i = 0; i < 4; i++) {
		IDirect3DDevice9_SetStreamSource(d3d9.adapter->dev, i, shd->quad, 0,
				sizeof(_vertex_buffer));
	}
}
INLINE static void d3d9_shader_param_set(const _texture *texture, UINT fcountmod, UINT fcount) {
	const _shader *shd = &texture->shader;
	UINT i, index;

	if (shd->uni.mvp) {
		cgD3D9SetUniformMatrix(shd->uni.mvp, &shd->mvp);
	}

	for (i = 0; i < 4; i++) {
		IDirect3DDevice9_SetStreamSource(d3d9.adapter->dev, i, shd->quad, 0,
				sizeof(_vertex_buffer));
	}

	// IN
	{
		// IN.video_size
		if (shd->uni.v.video_size) {
			cgD3D9SetUniform(shd->uni.v.video_size, &shd->info.video_size);
		}
		if (shd->uni.f.video_size) {
			cgD3D9SetUniform(shd->uni.f.video_size, &shd->info.video_size);
		}
		// IN.texture_size
		if (shd->uni.v.texture_size) {
			cgD3D9SetUniform(shd->uni.v.texture_size, &shd->info.texture_size);
		}
		if (shd->uni.f.texture_size) {
			cgD3D9SetUniform(shd->uni.f.texture_size, &shd->info.texture_size);
		}
		// IN.output_size
		if (shd->uni.v.output_size) {
			cgD3D9SetUniform(shd->uni.v.output_size, &shd->info.output_size);
		}
		if (shd->uni.f.output_size) {
			cgD3D9SetUniform(shd->uni.f.output_size, &shd->info.output_size);
		}
		// IN.frame_count
		{
			FLOAT fc = (FLOAT) fcount;

			if (fcountmod) {
				fc = (FLOAT) (fcount % fcountmod);
			}

			if (shd->uni.v.frame_count) {
				cgD3D9SetUniform(shd->uni.v.frame_count, &fc);
			}
			if (shd->uni.f.frame_count) {
				cgD3D9SetUniform(shd->uni.f.frame_count, &fc);
			}
		}
		// IN.frame_direction
		{
			FLOAT frame_direction = -1;

			if (shd->uni.v.frame_direction) {
				cgD3D9SetUniform(shd->uni.v.frame_direction, &frame_direction);
			}
			if (shd->uni.f.frame_direction) {
				cgD3D9SetUniform(shd->uni.f.frame_direction, &frame_direction);
			}
		}
	}

	// param
	for (i = 0; i < shader_effect.params; i++) {
		if (shd->uni.v.param[i]) {
			cgD3D9SetUniform(shd->uni.v.param[i], &shader_effect.param[i].value);
		}
		if (shd->uni.f.param[i]) {
			cgD3D9SetUniform(shd->uni.f.param[i], &shader_effect.param[i].value);
		}
	}

	// lut
	for (i = 0; i < shader_effect.luts; i++) {
		_lut *lut = &d3d9.lut[i];
		int bound_index = -1;

		if (shd->uni.v.lut[i]) {
			index = cgGetParameterResourceIndex(shd->uni.v.lut[i]);
			bound_index = index;
			IDirect3DDevice9_SetTexture(d3d9.adapter->dev, index,
					(IDirect3DBaseTexture9 * ) lut->data);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MAGFILTER,
					lut->filter);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MINFILTER,
					lut->filter);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSU,
			        D3DTADDRESS_BORDER);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSV,
			        D3DTADDRESS_BORDER);
		}
		if (shd->uni.f.lut[i]) {
			index = cgGetParameterResourceIndex(shd->uni.f.lut[i]);
			if (index == bound_index) {
				continue;
			}
			IDirect3DDevice9_SetTexture(d3d9.adapter->dev, index,
					(IDirect3DBaseTexture9 * ) lut->data);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MAGFILTER,
					lut->filter);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MINFILTER,
					lut->filter);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSU,
			        D3DTADDRESS_BORDER);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSV,
			        D3DTADDRESS_BORDER);
		}
	}

	// ORIG
	{
		// ORIG.texture
		if (shd->uni.orig.f.texture) {
			index = cgGetParameterResourceIndex(shd->uni.orig.f.texture);
			IDirect3DDevice9_SetTexture(d3d9.adapter->dev, index,
					(IDirect3DBaseTexture9 * ) d3d9.screen.tex[d3d9.screen.index].data);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MAGFILTER,
					D3DTEXF_POINT);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MINFILTER,
					D3DTEXF_POINT);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSU,
					D3DTADDRESS_BORDER);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSV,
					D3DTADDRESS_BORDER);
		}
		// ORIG.video_size
		if (shd->uni.orig.v.video_size) {
			cgD3D9SetUniform(shd->uni.orig.v.video_size,
					&d3d9.screen.tex[d3d9.screen.index].shader.info.video_size);
		}
		if (shd->uni.orig.f.video_size) {
			cgD3D9SetUniform(shd->uni.orig.f.video_size,
					&d3d9.screen.tex[d3d9.screen.index].shader.info.video_size);
		}
		// ORIG.texture_size
		if (shd->uni.orig.v.texture_size) {
			cgD3D9SetUniform(shd->uni.orig.v.texture_size,
					&d3d9.screen.tex[d3d9.screen.index].shader.info.texture_size);
		}
		if (shd->uni.orig.f.texture_size) {
			cgD3D9SetUniform(shd->uni.orig.f.texture_size,
					&d3d9.screen.tex[d3d9.screen.index].shader.info.texture_size);
		}
		// ORIG.tex_coord
		if (shd->uni.orig.v.tex_coord) {
			IDirect3DDevice9_SetStreamSource(d3d9.adapter->dev,
					shd->attribs.attrib[cgGetParameterResourceIndex(shd->uni.orig.v.tex_coord)],
					d3d9.screen.tex[d3d9.screen.index].shader.quad, 0, sizeof(_vertex_buffer));
		}
	}

	// FEEDBACK
	if (d3d9.feedback.in_use) {
		// FEEDBACK.texture
		if (shd->uni.feedback.f.texture) {
			D3DTEXTUREFILTERTYPE filter = d3d9_shader_filter(
					shader_effect.sp[shader_effect.feedback_pass].linear);

			index = cgGetParameterResourceIndex(shd->uni.feedback.f.texture);
			IDirect3DDevice9_SetTexture(d3d9.adapter->dev, index,
					(IDirect3DBaseTexture9 * ) d3d9.feedback.tex.data);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MAGFILTER,
					filter);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MINFILTER,
					filter);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSU,
					D3DTADDRESS_BORDER);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSV,
					D3DTADDRESS_BORDER);
		}
		// FEEDBACK.video_size
		if (shd->uni.feedback.v.video_size) {
			cgD3D9SetUniform(shd->uni.feedback.v.video_size,
					&d3d9.texture[shader_effect.feedback_pass].shader.info.video_size);
		}
		if (shd->uni.feedback.f.video_size) {
			cgD3D9SetUniform(shd->uni.feedback.f.video_size,
					&d3d9.texture[shader_effect.feedback_pass].shader.info.video_size);
		}
		// FEEDBACK.texture_size
		if (shd->uni.feedback.v.texture_size) {
			cgD3D9SetUniform(shd->uni.feedback.v.texture_size,
					&d3d9.texture[shader_effect.feedback_pass].shader.info.texture_size);
		}
		if (shd->uni.feedback.f.texture_size) {
			cgD3D9SetUniform(shd->uni.feedback.f.texture_size,
					&d3d9.texture[shader_effect.feedback_pass].shader.info.texture_size);
		}
		// FEEDBACK.tex_coord
		if (shd->uni.feedback.v.tex_coord) {
			IDirect3DDevice9_SetStreamSource(d3d9.adapter->dev,
					shd->attribs.attrib[cgGetParameterResourceIndex(shd->uni.feedback.v.tex_coord)],
					d3d9.texture[shader_effect.feedback_pass].shader.quad, 0,
					sizeof(_vertex_buffer));
		}
	}

	// PREV
	{
		INT circle_index = d3d9.screen.index - 1;

		for (i = 0; i < (d3d9.screen.in_use - 1); i++) {
			if (circle_index < 0) {
				circle_index = d3d9.screen.in_use - 1;
			}
			// PREV[x].texture
			if (shd->uni.prev[i].f.texture) {
				index = cgGetParameterResourceIndex(shd->uni.prev[i].f.texture);
				IDirect3DDevice9_SetTexture(d3d9.adapter->dev, index,
						(IDirect3DBaseTexture9 *) d3d9.screen.tex[circle_index].data);
				IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MAGFILTER,
						D3DTEXF_POINT);
				IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MINFILTER,
						D3DTEXF_POINT);
				IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSU,
						D3DTADDRESS_BORDER);
				IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSV,
						D3DTADDRESS_BORDER);
			}
			// PREV[x].tex_coord
			if (shd->uni.prev[i].v.tex_coord) {
				IDirect3DDevice9_SetStreamSource(d3d9.adapter->dev,
						shd->attribs.attrib[cgGetParameterResourceIndex(shd->uni.prev[i].v.tex_coord)],
						d3d9.screen.tex[circle_index].shader.quad, 0, sizeof(_vertex_buffer));
			}
			circle_index--;
		}
	}

	// PASS
	for (i = 0; i < shader_effect.running_pass; i++) {
		UINT next = i + 1;

		// PASS[x].texture
		if (shd->uni.passprev[i].f.texture) {
			D3DTEXTUREFILTERTYPE filter = d3d9_shader_filter(shader_effect.sp[i].linear);
			UINT resind = cgGetParameterResourceIndex(shd->uni.passprev[i].f.texture);

			IDirect3DDevice9_SetTexture(d3d9.adapter->dev, resind,
					(IDirect3DBaseTexture9 *) d3d9.texture[i].data);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, resind, D3DSAMP_MAGFILTER, filter);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, resind, D3DSAMP_MINFILTER, filter);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, resind, D3DSAMP_ADDRESSU,
					D3DTADDRESS_BORDER);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, resind, D3DSAMP_ADDRESSV,
					D3DTADDRESS_BORDER);
		}
		// PASS[x].video_size
		if (shd->uni.passprev[i].v.video_size) {
			cgD3D9SetUniform(shd->uni.passprev[i].v.video_size,
					&d3d9.texture[next].shader.info.video_size);
		}
		if (shd->uni.passprev[i].f.video_size) {
			cgD3D9SetUniform(shd->uni.passprev[i].f.video_size,
					&d3d9.texture[next].shader.info.video_size);
		}
		// PASS[x].texture_size
		if (shd->uni.passprev[i].v.texture_size) {
			cgD3D9SetUniform(shd->uni.passprev[i].v.texture_size,
					&d3d9.texture[next].shader.info.texture_size);
		}
		if (shd->uni.passprev[i].f.texture_size) {
			cgD3D9SetUniform(shd->uni.passprev[i].f.texture_size,
					&d3d9.texture[next].shader.info.texture_size);
		}
		// PASS[x].tex_coord
		if (shd->uni.passprev[i].v.tex_coord) {
			IDirect3DDevice9_SetStreamSource(d3d9.adapter->dev,
					shd->attribs.attrib[cgGetParameterResourceIndex(shd->uni.passprev[i].v.tex_coord)],
					d3d9.texture[next].shader.quad, 0, sizeof(_vertex_buffer));
		}
	}
}
