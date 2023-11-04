/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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
#include "info.h"
#include "gui.h"
#include "conf.h"
#include "ppu.h"
#include "overscan.h"
#include "video/gfx_thread.h"
#include "emu_thread.h"
#include "palette.h"
#if defined (WITH_FFMPEG)
#include "recording.h"
#endif

static void d3d9_shader_cg_error_handler(void);
static BYTE d3d9_device_create(UINT width, UINT height);
INLINE static void d3d9_read_front_buffer(void);
static BYTE d3d9_texture_create(_texture *texture, UINT index);
static BYTE d3d9_texture_simple_create(_texture_simple *texture, UINT w, UINT h, BOOL overlay);
static BYTE d3d9_texture_lut_create(_lut *lut, UINT index);
static void d3d9_surface_clean(LPDIRECT3DSURFACE9 *surface, UINT width, UINT height, uint32_t color);
static BYTE d3d9_shader_init(UINT pass, _shader *shd, const uTCHAR *path, const char *code);
static void d3d9_shader_delete(_shader *shd);
static void d3d9_shader_uniform_ctrl(CGparameter *dst, CGparameter *param, const char *semantic);
static void d3d9_shader_uni_texture_clear(_shader_uniforms_tex *sut);
static void d3d9_shader_uni_texture(_shader_uniforms_tex *sut, _shader_prg_cg *prg, char *fmt, ...);
static CGparameter d3d9_cg_find_param(CGparameter prm, const char *name);
static BYTE d3d9_vertex_declaration_create(_shader *shd);
static void d3d9_vertex_buffer_set(_shader *shd,  _viewport *vp, _texture_rect *prev, BYTE last_pass);
INLINE static void d3d9_viewport_set(DWORD x, DWORD y, DWORD w, DWORD h);
INLINE D3DTEXTUREFILTERTYPE d3d9_shader_filter(UINT type);
INLINE static void d3d9_shader_params_overlay_set(_shader *shd);
INLINE static void d3d9_shader_param_set(const _texture *texture, UINT sindex, UINT fcountmod, UINT fcount);

_d3d9 d3d9;

BYTE d3d9_init(void) {
	memset(&d3d9, 0x00, sizeof(d3d9));

	d3d9.d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (d3d9.d3d == NULL) {
		gui_critical(uL("Unable to create d3d object."));
		return (EXIT_ERROR);
	}

	// mi passo in rassegna tutti gli adapter presenti sul sistema
	d3d9.adapters_on_system = IDirect3D9_GetAdapterCount(d3d9.d3d);

	d3d9.array = malloc(d3d9.adapters_on_system * sizeof(_d3d9_adapter));
	if (!d3d9.array) {
		gui_critical(uL("Unable to create devices array."));
		return (EXIT_ERROR);
	}

	{
		unsigned int adapt = 0;

		for (adapt = 0; adapt < d3d9.adapters_on_system; adapt++) {
			_d3d9_adapter *dev = D3D9_ADAPTER(d3d9.adapters_in_use);
			D3DADAPTER_IDENTIFIER9 d3dinfo;
			D3DCAPS9 d3dcaps;

			memset(dev, 0x00, sizeof(_d3d9_adapter));

			dev->id = adapt;

			if (IDirect3D9_GetAdapterIdentifier(d3d9.d3d, dev->id, 0, &d3dinfo)!= D3D_OK) {
				log_error(uL("d3d9 adptr %d;unable to get adapter display info"));
				continue;
			}

			log_info(uL("d3d9 adptr %d;%s"), dev->id, d3dinfo.Description);

			if (IDirect3D9_GetAdapterDisplayMode(d3d9.d3d, dev->id, &dev->display_mode) != D3D_OK) {
				log_error_box(uL("unable to get adapter display mode"));
				continue;
			}

			if ((dev->display_mode.Format == D3DFMT_X8R8G8B8) ||
				(dev->display_mode.Format == D3DFMT_A8R8G8B8)) {
				dev->bit_per_pixel = 32;
			}
			if (dev->bit_per_pixel < 32) {
				log_error_box(uL("video mode < 32 bits are not supported"));
				continue;
			}

			// Check for hardware T&L
			if (IDirect3D9_GetDeviceCaps(d3d9.d3d, dev->id, D3DDEVTYPE_HAL, &d3dcaps) != D3D_OK) {
				log_error_box(uL("unable to get device caps"));
				continue;
			}

			if (d3dcaps.Caps2 & D3DCAPS2_DYNAMICTEXTURES) {
				dev->dynamic_texture = TRUE;
			} else {
				dev->dynamic_texture = FALSE;
				log_info_box(uL("don't support dynamic texture"));
			}

			if (d3dcaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) {
				dev->texture_square_only = TRUE;
				log_info_box(uL("support only square texture"));
			} else {
				dev->texture_square_only = FALSE;
			}

		 	// Device can accelerate a memory copy from system memory to local video memory.
		 	// This cap guarantees that UpdateSurface and UpdateTexture calls will be hardware
		 	// accelerated. If this cap is absent, these calls will succeed but will be slower.
			if (!(d3dcaps.Caps3 & D3DCAPS3_COPY_TO_VIDMEM)) {
				log_info_box(uL("don't support accelerated texture update"));
			}

			if (d3dcaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
				dev->flags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
				// se abilito il PURE DEVICE, non posso utilizzare il
				// IDirect3DDevice9_GetTransform quando uso le shaders.
				if (d3dcaps.DevCaps & D3DDEVCAPS_PUREDEVICE) {
					dev->flags |= D3DCREATE_PUREDEVICE;
				}
			} else {
				log_info_box(uL("don't support hardware accelaration"));
				dev->flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
			}

			if (d3dcaps.MaxSimultaneousTextures <= 1) { //number of textures
				log_info_box(uL("single pass multitexturing not supported"));
			} else {
				log_info_box(uL("MaxSimultaneousTextures %ld"), d3dcaps.MaxSimultaneousTextures);
			}

			dev->number_of_monitors = d3dcaps.NumberOfAdaptersInGroup;

			//if (dev->number_of_monitors > 1) {
				log_info_box(uL("MasterAdapterOrdinal    %d"), d3dcaps.MasterAdapterOrdinal);
				log_info_box(uL("AdapterOrdinalInGroup   %d"), d3dcaps.AdapterOrdinalInGroup);
				log_info_box(uL("NumberOfAdaptersInGroup %d"), dev->number_of_monitors);
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
					log_info_box(uL("don't support shaders >= 2.0"));
				} else {
					dev->hlsl_compliant = TRUE;
				}

				if (!dev->hlsl_compliant) {
					log_info_box(uL("shaders are not supported"));
				}

				d3d9.adapter = NULL;
			}

			d3d9.adapters_in_use++;
		}
	}

	if (d3d9.adapters_in_use == 0) {
		gui_critical(uL("Unable find usable adapter"));
		return (EXIT_ERROR);
	}

	{
		HMONITOR monitor = MonitorFromWindow(gui_win_id(), MONITOR_DEFAULTTOPRIMARY);
		UINT i = 0;

		d3d9.adapter = D3D9_ADAPTER(0);

		for (i = 0; i < d3d9.adapters_in_use; i++) {
			_d3d9_adapter *adapter = D3D9_ADAPTER(i);

			if (monitor == IDirect3D9_GetAdapterMonitor(d3d9.d3d, adapter->id)) {
				d3d9.adapter = adapter;
			}
		}
	}

	if (!d3d9.adapter->hlsl_compliant) {
		gui_critical(uL("Adapter is not hlsl compliant."));
		return (EXIT_ERROR);
	}

	gfx.bit_per_pixel = d3d9.adapter->bit_per_pixel;

	return (EXIT_OK);
}
void d3d9_quit(void) {
	d3d9_context_delete(TRUE);

	{
		UINT i = 0;

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

	if (d3d9.screenshot.srfc.s) {
		IDirect3DSurface9_Release(d3d9.screenshot.srfc.s);
		d3d9.screenshot.srfc.s = NULL;
		d3d9.screenshot.zone.w = 0;
		d3d9.screenshot.zone.h = 0;
	}

	if (d3d9.screenshot.zone.s) {
		IDirect3DSurface9_Release(d3d9.screenshot.zone.s);
		d3d9.screenshot.zone.s = NULL;
		d3d9.screenshot.zone.w = 0;
		d3d9.screenshot.zone.h = 0;
	}
}
BYTE d3d9_context_create(void) {
	D3DXMATRIX identity;
	WORD w = 0, h = 0;
	UINT i = 0;

	gfx_thread_lock();

	d3d9_context_delete(FALSE);

	if (!cfg->fullscreen && ((cfg->screen_rotation == ROTATE_90) || (cfg->screen_rotation == ROTATE_270))) {
		d3d9.video_mode.w = gfx.h[VIDEO_MODE];
		d3d9.video_mode.h = gfx.w[VIDEO_MODE];
	} else {
		d3d9.video_mode.w = gfx.w[VIDEO_MODE];
		d3d9.video_mode.h = gfx.h[VIDEO_MODE];
	}

	if (overscan.enabled && (!cfg->oscan_black_borders && !cfg->fullscreen)) {
		// visto che lavorero' con texture piu' grandi del video mode
		// setto un backbuffer piu' grande.
		w = d3d9.video_mode.w * 2;
		h = d3d9.video_mode.h * 2;
	} else {
		w = d3d9.video_mode.w;
		h = d3d9.video_mode.h;
	}

	if (d3d9_device_create((UINT)((float)w * gfx.device_pixel_ratio), (UINT)((float)h * gfx.device_pixel_ratio)) == EXIT_ERROR) {
		d3d9_context_delete(FALSE);
		gfx_thread_unlock();
		return (EXIT_ERROR);
	}

	d3d9.cgctx = cgCreateContext();
	if (d3d9.cgctx == NULL) {
		d3d9_context_delete(FALSE);
		gfx_thread_unlock();
		return (EXIT_ERROR);
	}

	cgSetErrorCallback(d3d9_shader_cg_error_handler);

	cgD3D9SetDevice(d3d9.adapter->dev);

	w = gfx.w[PASS0];
	h = gfx.h[PASS0];

	D3DXMatrixIdentity(&identity);
	IDirect3DDevice9_SetTransform(d3d9.adapter->dev, D3DTS_WORLD, &identity);
	IDirect3DDevice9_SetTransform(d3d9.adapter->dev, D3DTS_VIEW, &identity);

	// screen
	if (d3d9_texture_simple_create(&d3d9.screen.tex[0], w, h, FALSE) == EXIT_ERROR) {
		d3d9_context_delete(FALSE);
		gfx_thread_unlock();
		return (EXIT_ERROR);
	}

	// lut (devo farlo prima di elaborare le shaders)
	for (i = 0; i < shader_effect.luts; i++) {
		if (d3d9_texture_lut_create(&d3d9.lut[i], i) == EXIT_ERROR) {
			d3d9_context_delete(FALSE);
			gfx_thread_unlock();
			return (EXIT_ERROR_SHADER);
		}
	}

	// devo precalcolarmi il viewport finale
	{
		_viewport *vp = &gfx.vp;
		float vmw = (float)d3d9.video_mode.w * gfx.device_pixel_ratio;
		float vmh = (float)d3d9.video_mode.h * gfx.device_pixel_ratio;

		// configuro l'aspect ratio del fullscreen
		if (cfg->fullscreen) {
			float mw = (cfg->screen_rotation == ROTATE_90) || (cfg->screen_rotation == ROTATE_270) ?
				_SCR_ROWS_NOBRD : _SCR_COLUMNS_NOBRD;
			float mh = (cfg->screen_rotation == ROTATE_90) || (cfg->screen_rotation == ROTATE_270) ?
				_SCR_COLUMNS_NOBRD : _SCR_ROWS_NOBRD;
			float ratio = mw / mh;

			vp->x = 0;
			vp->y = 0;
			vp->w = (float)gfx.w[VIDEO_MODE] * gfx.device_pixel_ratio;
			vp->h = (float)gfx.h[VIDEO_MODE] * gfx.device_pixel_ratio;

			if (!cfg->stretch) {
				if (cfg->integer_scaling) {
					int factor = vmw > vmh ? vmh / mh : vmw / mw;

					vp->w = mw * (float)factor;
					vp->h = mh * (float)factor;
				} else if (vmw > vmh) {
					vp->w = vmh * ratio;
				} else {
					vp->h = vmw / ratio;
				}
				vp->x += (vmw - vp->w) / 2.0f;
				vp->y += (vmh - vp->h) / 2.0f;
			}

			if (overscan.enabled && !cfg->oscan_black_borders_fscr) {
				float left = cfg->hflip_screen ? (float)overscan.borders->right : (float)overscan.borders->left;
				float right = cfg->hflip_screen ? (float)overscan.borders->left : (float)overscan.borders->right;
				float brd_l_x = 0, brd_r_x = 0, brd_u_y = 0, brd_d_y = 0;
				float ratio_x = 0, ratio_y = 0;

				ratio_x = vp->w / mw;
				ratio_y = vp->h / mh;

				switch (cfg->screen_rotation) {
					default:
					case ROTATE_0:
						brd_l_x = left * ratio_x;
						brd_r_x = right * ratio_x;
						brd_u_y = (float)overscan.borders->up * ratio_y;
						brd_d_y = (float)overscan.borders->down * ratio_y;
						break;
					case ROTATE_90:
						brd_l_x = (float)overscan.borders->down * ratio_y;
						brd_r_x = (float)overscan.borders->up * ratio_y;
						brd_u_y = left * ratio_x;
						brd_d_y = right * ratio_x;
						break;
					case ROTATE_180:
						brd_l_x = right * ratio_x;
						brd_r_x = left * ratio_x;
						brd_u_y = (float)overscan.borders->down * ratio_y;
						brd_d_y = (float)overscan.borders->up * ratio_y;
						break;
					case ROTATE_270:
						brd_l_x = (float)overscan.borders->up * ratio_y;
						brd_r_x = (float)overscan.borders->down * ratio_y;
						brd_u_y = right * ratio_x;
						brd_d_y = left * ratio_x;
						break;
				}

				d3d9.viewp.left = (LONG)brd_l_x;
				d3d9.viewp.top = (LONG)brd_u_y;
				d3d9.viewp.right = (LONG)(vmw - brd_r_x);
				d3d9.viewp.bottom = (LONG)(vmh - brd_d_y);
			} else {
				d3d9.viewp.left = 0;
				d3d9.viewp.top = 0;
				d3d9.viewp.right = (LONG)vmw;
				d3d9.viewp.bottom = (LONG)vmh;
			}
		} else {
			vp->x = 0;
			vp->y = 0;
			vp->w = (float)gfx.w[VIDEO_MODE];
			vp->h = (float)gfx.h[VIDEO_MODE];

			if (overscan.enabled && !cfg->oscan_black_borders) {
				float hrz = (float)((cfg->screen_rotation == ROTATE_180) || (cfg->screen_rotation == ROTATE_270) ?
					cfg->hflip_screen ? overscan.borders->left : overscan.borders->right :
					cfg->hflip_screen ? overscan.borders->right : overscan.borders->left);
				float vrt = (float)((cfg->screen_rotation == ROTATE_90) || (cfg->screen_rotation == ROTATE_180) ?
					overscan.borders->down : overscan.borders->up);

				vp->x = ((-hrz * gfx.width_pixel) * gfx.pixel_aspect_ratio);
				vp->y = -vrt * (float)cfg->scale;
				vp->w = (float)gfx.w[NO_OVERSCAN] * gfx.pixel_aspect_ratio;
				vp->h = (float)gfx.h[NO_OVERSCAN];
			}

			if ((cfg->screen_rotation == ROTATE_90) || (cfg->screen_rotation == ROTATE_270)) {
				float vpx = vp->x, vpw = vp->w;

				vp->x = vp->y;
				vp->y = vpx;
				vp->w = vp->h;
				vp->h = vpw;
			}

			vp->x *= gfx.device_pixel_ratio;
			vp->y *= gfx.device_pixel_ratio;
			vp->w *= gfx.device_pixel_ratio;
			vp->h *= gfx.device_pixel_ratio;

			d3d9.viewp.left = (LONG)-vp->x;
			d3d9.viewp.top = (LONG)-vp->y;
			d3d9.viewp.right = (LONG)(vmw + (float)d3d9.viewp.left);
			d3d9.viewp.bottom = (LONG)(vmh + (float)d3d9.viewp.top);
		}
	}

	// texture
	for (i = 0; i < shader_effect.pass; i++) {
		log_info(uL("d3d9;setting pass %d"), i);

		if (d3d9_texture_create(&d3d9.texture[i], i) == EXIT_ERROR) {
			d3d9_context_delete(FALSE);
			gfx_thread_unlock();
			return (EXIT_ERROR);
		}

		if (d3d9_shader_init(i, &d3d9.texture[i].shader, shader_effect.sp[i].path, shader_effect.sp[i].code) == EXIT_ERROR) {
			d3d9_context_delete(FALSE);
			gfx_thread_unlock();
			return (EXIT_ERROR_SHADER);
		}
	}

	// PREV (calcolo il numero di screen da utilizzare)
	// deve essere fatto dopo il d3d9_shader_init().
	for (i = 0; i < shader_effect.pass; i++) {
		UINT a = 0;

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
		if (d3d9_texture_simple_create(&d3d9.screen.tex[i], w, h, FALSE) == EXIT_ERROR) {
			d3d9_context_delete(FALSE);
			gfx_thread_unlock();
			return (EXIT_ERROR);
		}
	}

	// FEEDBACK
	if ((shader_effect.feedback_pass >= 0) && (shader_effect.feedback_pass < shader_effect.pass)) {
		d3d9.feedback.in_use = TRUE;

		if (d3d9_texture_create(&d3d9.feedback.tex, shader_effect.feedback_pass) == EXIT_ERROR) {
			d3d9_context_delete(FALSE);
			gfx_thread_unlock();
			return (EXIT_ERROR);
		}
	}

	// overlay
	{
		BYTE rotate = FALSE;
		float ow = 0, oh = 0;
		float vmw = (float)gfx.w[VIDEO_MODE];
		float vmh = (float)gfx.h[VIDEO_MODE];

		// setto il necessario per il blending
		IDirect3DDevice9_SetRenderState(d3d9.adapter->dev, D3DRS_BLENDOP, D3DBLENDOP_ADD);
		IDirect3DDevice9_SetRenderState(d3d9.adapter->dev, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		IDirect3DDevice9_SetRenderState(d3d9.adapter->dev, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		if (cfg->fullscreen) {
			float div = 0;

			if (!gfx.is_wayland && !cfg->fullscreen_in_window) {
				vmw *= gfx.device_pixel_ratio;
				vmh *= gfx.device_pixel_ratio;
			}
			div = vmw / 1024.0f;

			if (div < 1.0f) {
				div = 1.0f;
			}

			ow = vmw / div;
			oh = vmh / div;
		} else {
			ow = _SCR_COLUMNS_NOBRD * 2;
			oh = _SCR_ROWS_NOBRD * 2;
		}

		if (vmw < ow) {
			ow = vmw;
		}
		if (vmh < oh) {
			oh = vmh;
		}

		if ((cfg->screen_rotation == ROTATE_90) || (cfg->screen_rotation == ROTATE_270)) {
			if (cfg->text_rotation) {
				if (cfg->fullscreen) {
					rotate = TRUE;
				}
			} else if (!cfg->fullscreen) {
				rotate = TRUE;
			}
		}

		if (rotate) {
			float tmp = ow;

			ow = oh;
			oh = tmp;
		}

		if (d3d9_texture_simple_create(&d3d9.overlay, (UINT)ow, (UINT)oh, TRUE) == EXIT_ERROR) {
			d3d9_context_delete(FALSE);
			gfx_thread_unlock();
			return (EXIT_ERROR);
		}

		gui_overlay_set_size((int)ow, (int)oh);

		log_info(uL("d3d9;setting overlay pass"));

		if (d3d9_shader_init(0, &d3d9.overlay.shader, NULL, shader_code_blend()) == EXIT_ERROR) {
			d3d9_context_delete(FALSE);
			gfx_thread_unlock();
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

		shd->info.video_size.x = (FLOAT)prev->base.w;
		shd->info.video_size.y = (FLOAT)prev->base.h;
		shd->info.texture_size.x = prev->w,
		shd->info.texture_size.y = prev->h;
		shd->info.output_size.x = (FLOAT)texture->vp.w;
		shd->info.output_size.y = (FLOAT)texture->vp.h;

		d3d9_vertex_buffer_set(shd, &texture->vp, prev, i == shader_effect.last_pass);
	}

	umemcpy(gfx.last_shader_file, cfg->shader_file, usizeof(gfx.last_shader_file));

	gfx_thread_unlock();

	return (EXIT_OK);
}
void d3d9_draw_scene(void) {
	const _texture_simple *scrtex = NULL;
	LPDIRECT3DSURFACE9 back_buffer = NULL;
	UINT sindex = 0, i = 0;

	if (!gui.start || (gfx.frame.in_draw == gfx.frame.filtered)) {
		return;
	}

	gfx_thread_lock();

	sindex = d3d9.screen.index;
	scrtex = &d3d9.screen.tex[sindex];

	// aggiorno la texture dello schermo
	if (overscan.enabled) {
		POINT point;
		RECT rect;

		rect.left = (LONG)((float)overscan.borders->left * gfx.filter.width_pixel);
		rect.top = overscan.borders->up * gfx.filter.factor;
		rect.right = (LONG)((float)scrtex->rect.base.w - ((float)overscan.borders->right * gfx.filter.width_pixel));
		rect.bottom = (LONG)(scrtex->rect.base.h - (overscan.borders->down * gfx.filter.factor));

		point.x = rect.left;
		point.y = rect.top;

		IDirect3DDevice9_UpdateSurface(d3d9.adapter->dev, scrtex->offscreen, &rect, scrtex->map0, &point);
	} else {
		IDirect3DDevice9_UpdateSurface(d3d9.adapter->dev, scrtex->offscreen, NULL, scrtex->map0, NULL);
	}

	gfx.frame.in_draw = gfx.frame.filtered;
	d3d9.screen.index = ((d3d9.screen.index + 1) % d3d9.screen.in_use);

	gfx_thread_unlock();

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
			d3d9_viewport_set(0, 0, (DWORD)((float)d3d9.video_mode.w * gfx.device_pixel_ratio), (DWORD)((float)d3d9.video_mode.h * gfx.device_pixel_ratio));
		} else {
			IDirect3DDevice9_SetRenderTarget(d3d9.adapter->dev, 0, texture->map0);
			// pulisco l'fbo
			d3d9_viewport_set(0, 0, (DWORD)texture->rect.w, (DWORD)texture->rect.h);
		}
		IDirect3DDevice9_Clear(d3d9.adapter->dev, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 0, 0, 0), 1.0f, 0);

		// ora setto il viewport corretto
		if (cfg->fullscreen) {
			d3d9_viewport_set((DWORD)texture->vp.x, (DWORD)texture->vp.y, (DWORD)texture->vp.w, (DWORD)texture->vp.h);
		} else {
			d3d9_viewport_set(0, 0, (DWORD)texture->vp.w, (DWORD)texture->vp.h);
		}

		cgD3D9BindProgram(texture->shader.prg.f);
		cgD3D9BindProgram(texture->shader.prg.v);

		if (i == 0) {
			IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, (IDirect3DBaseTexture9 *)scrtex->data);
		} else {
			IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, (IDirect3DBaseTexture9 *)d3d9.texture[i - 1].data);
		}

		IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_MAGFILTER, filter);
		IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_MINFILTER, filter);

		IDirect3DDevice9_SetVertexDeclaration(d3d9.adapter->dev, texture->shader.vd);

		d3d9_shader_param_set(texture, sindex, sp->frame_count_mod, nes[0].p.ppu.frames);

		IDirect3DDevice9_BeginScene(d3d9.adapter->dev);
		IDirect3DDevice9_DrawPrimitive(d3d9.adapter->dev, D3DPT_TRIANGLESTRIP, 0, 2);
		IDirect3DDevice9_EndScene(d3d9.adapter->dev);
	}

	IDirect3DSurface9_Release(back_buffer);

	if (d3d9.feedback.in_use) {
		LPDIRECT3DTEXTURE9 data = d3d9.feedback.tex.data;
		LPDIRECT3DSURFACE9 map0 = d3d9.feedback.tex.map0;

		d3d9.feedback.tex.data = d3d9.texture[shader_effect.feedback_pass].data;
		d3d9.feedback.tex.map0 = d3d9.texture[shader_effect.feedback_pass].map0;
		d3d9.texture[shader_effect.feedback_pass].data = data;
		d3d9.texture[shader_effect.feedback_pass].map0 = map0;
	}

	// rendering dell'overlay
	gui_overlay_blit();

	// overlay
	if (cfg->txt_on_screen && gui_overlay_is_updated()) {
		DWORD vpx = 0, vpy = 0, vpw = 0, vph = 0;

		vpx = (DWORD)d3d9.viewp.left;
		vpy = (DWORD)d3d9.viewp.top;
		vpw = (DWORD)(d3d9.viewp.right - d3d9.viewp.left);
		vph = (DWORD)(d3d9.viewp.bottom - d3d9.viewp.top);

		d3d9_viewport_set(vpx, vpy, vpw, vph);

		cgD3D9BindProgram(d3d9.overlay.shader.prg.f);
		cgD3D9BindProgram(d3d9.overlay.shader.prg.v);

		IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, (IDirect3DBaseTexture9 *)d3d9.overlay.data);

		IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

		IDirect3DDevice9_SetVertexDeclaration(d3d9.adapter->dev, d3d9.overlay.shader.vd);

		d3d9_shader_params_overlay_set(&d3d9.overlay.shader);

		IDirect3DDevice9_SetRenderState(d3d9.adapter->dev, D3DRS_ALPHABLENDENABLE, TRUE);
		IDirect3DDevice9_BeginScene(d3d9.adapter->dev);
		IDirect3DDevice9_DrawPrimitive(d3d9.adapter->dev, D3DPT_TRIANGLESTRIP, 0, 2);
		IDirect3DDevice9_EndScene(d3d9.adapter->dev);
		IDirect3DDevice9_SetRenderState(d3d9.adapter->dev, D3DRS_ALPHABLENDENABLE, FALSE);
	}

	IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, NULL);

	// swap buffers
	if (IDirect3DDevice9_Present(d3d9.adapter->dev, &d3d9.viewp, NULL, NULL, NULL) == D3DERR_DEVICELOST) {
		if (IDirect3DDevice9_TestCooperativeLevel(d3d9.adapter->dev) == D3DERR_DEVICENOTRESET) {
			emu_thread_pause();
			if (d3d9_context_create() == EXIT_ERROR) {
				log_error(uL("d3d9;unable to initialize d3d context"));
			}
			emu_thread_continue();
		}
	}

#if defined (WITH_FFMPEG)
	// screenshot e video recording
	if (info.recording_is_a_video | info.screenshot) {
#else
	// screenshot
	if (info.screenshot) {
#endif
		d3d9_read_front_buffer();
	}
}

static void d3d9_shader_cg_error_handler(void) {
	CGerror error = cgGetError();

	if (error == (CGerror) cgD3D9Failed) {
		log_error(uL("d3d9;error '%s' occurred"), cgD3D9TranslateHRESULT(cgD3D9GetLastError()));
	} else {
		log_error(uL("cg;error '%s' occurred"), cgD3D9TranslateCGerror(error));
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
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	d3dpp.hDeviceWindow = gui_screen_id();
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferFormat = d3d9.adapter->display_mode.Format;
	d3dpp.BackBufferWidth = width;
	d3dpp.BackBufferHeight = height;
	if (cfg->vsync) {
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
		gui_critical(uL("Unable to create d3d device."));
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
void d3d9_context_delete(BYTE lock) {
	UINT i = 0;

	if (lock) {
		gfx_thread_lock();
	}

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
		if (d3d9.overlay.offscreen) {
			IDirect3DSurface9_Release(d3d9.overlay.offscreen);
			d3d9.overlay.offscreen = NULL;
		}
		if (d3d9.overlay.map0) {
			IDirect3DSurface9_Release(d3d9.overlay.map0);
			d3d9.overlay.map0 = NULL;
		}
		if (d3d9.overlay.data) {
			IDirect3DTexture9_Release(d3d9.overlay.data);
			d3d9.overlay.data = NULL;
		}
		if (d3d9.overlay.shader.vd) {
			IDirect3DVertexDeclaration9_Release(d3d9.overlay.shader.vd);
			d3d9.overlay.shader.vd = NULL;
		}
		if (d3d9.overlay.shader.quad) {
			IDirect3DVertexBuffer9_Release(d3d9.overlay.shader.quad);
			d3d9.overlay.shader.quad = NULL;
		}
		d3d9_shader_delete(&d3d9.overlay.shader);
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

	if (lock) {
		gfx_thread_unlock();
	}
}
INLINE static void d3d9_read_front_buffer(void) {
	int w = 0, h = 0;

	if (info.screenshot == SCRSH_ORIGINAL_SIZE) {
		void *buffer = NULL;
		int stride = 0;

		w = SCR_COLUMNS;
		h = SCR_ROWS;
		stride = w * (int)sizeof(uint32_t);

		buffer = malloc(stride * SCR_ROWS);
		if (buffer) {
			emu_thread_pause();
			scale_surface_screenshoot_1x(emu_active_nidx(), stride, buffer);
			gui_save_screenshot(SCR_COLUMNS, SCR_ROWS, stride, buffer, FALSE);
			free(buffer);
			emu_thread_continue();
		}
		info.screenshot = SCRSH_NONE;
	}

#if defined (WITH_FFMPEG)
	if (info.recording_is_a_video | info.screenshot) {
#else
	if (info.screenshot) {
#endif
		IDirect3DSurface9 *bbuf = NULL, *srflock = NULL;
		RECT *viewp = NULL;
		D3DSURFACE_DESC sd;
		BYTE use_zone = FALSE;

		if (IDirect3DDevice9_GetBackBuffer(d3d9.adapter->dev, 0, 0, D3DBACKBUFFER_TYPE_MONO, &bbuf) != D3D_OK) {
			goto d3d9_read_front_buffer_end;
		}

		IDirect3DSurface9_GetDesc(bbuf, &sd);

		w = (int)sd.Width;
		h = (int)sd.Height;

		if ((d3d9.screenshot.srfc.s == NULL) || (d3d9.screenshot.srfc.w != w) || (d3d9.screenshot.srfc.h != h)) {
			if (d3d9.screenshot.srfc.s) {
				IDirect3DSurface9_Release(d3d9.screenshot.srfc.s);
				d3d9.screenshot.srfc.s = NULL;
			}
			if (d3d9.screenshot.zone.s) {
				IDirect3DSurface9_Release(d3d9.screenshot.zone.s);
				d3d9.screenshot.zone.s = NULL;
			}

			d3d9.screenshot.srfc.w = w;
			d3d9.screenshot.srfc.h = h;

			d3d9.screenshot.walign32 = w;
			if (d3d9.screenshot.walign32 % 32) {
				d3d9.screenshot.walign32 = (w / 32) + 1;
				d3d9.screenshot.walign32 *= 32;
			}
			d3d9.screenshot.stride = d3d9.screenshot.walign32 * (int)sizeof(uint32_t);

			if (IDirect3DDevice9_CreateOffscreenPlainSurface(d3d9.adapter->dev,
				w, h, sd.Format, D3DPOOL_SYSTEMMEM, &d3d9.screenshot.srfc.s, NULL) != D3D_OK) {
				goto d3d9_read_front_buffer_end;
			}
		}

		if (IDirect3DDevice9_GetRenderTargetData(d3d9.adapter->dev, bbuf, d3d9.screenshot.srfc.s) != D3D_OK) {
			goto d3d9_read_front_buffer_end;
		}

		if (overscan.enabled && ((!cfg->fullscreen && !cfg->oscan_black_borders) ||
			(cfg->fullscreen && !cfg->oscan_black_borders_fscr))) {
			use_zone = TRUE;

			w = d3d9.viewp.right - d3d9.viewp.left;
			h = d3d9.viewp.bottom - d3d9.viewp.top;

			viewp = &d3d9.viewp;
		} else if (w != d3d9.screenshot.walign32) {
			use_zone = TRUE;
		} else {
			srflock = d3d9.screenshot.srfc.s;
		}

		if (use_zone) {
			if ((d3d9.screenshot.zone.s == NULL) || (d3d9.screenshot.zone.h != h)) {
				if (d3d9.screenshot.zone.s) {
					IDirect3DSurface9_Release(d3d9.screenshot.zone.s);
					d3d9.screenshot.zone.s = NULL;
				}

				d3d9.screenshot.zone.h = h;

				if (IDirect3DDevice9_CreateOffscreenPlainSurface(d3d9.adapter->dev,
					d3d9.screenshot.walign32, h, sd.Format, D3DPOOL_DEFAULT, &d3d9.screenshot.zone.s, NULL) != D3D_OK) {
					goto d3d9_read_front_buffer_end;
				}
			}
			if (IDirect3DDevice9_UpdateSurface(d3d9.adapter->dev,
				d3d9.screenshot.srfc.s, viewp, d3d9.screenshot.zone.s, NULL) != D3D_OK) {
				goto d3d9_read_front_buffer_end;
			}
			srflock = d3d9.screenshot.zone.s;
		}

		{
			D3DLOCKED_RECT lrect;

			IDirect3DSurface9_LockRect(srflock, &lrect, NULL, 0);
#if defined (WITH_FFMPEG)
			if (info.recording_is_a_video) {
				recording_video_frame(w, h, d3d9.screenshot.stride, lrect.pBits);
			}
			if (info.screenshot == SCRSH_STANDARD) {
				gui_save_screenshot(w, h, d3d9.screenshot.stride, lrect.pBits, FALSE);
			}
#else
			gui_save_screenshot(w, h, d3d9.screenshot.stride, lrect.pBits, FALSE);
#endif
			IDirect3DSurface9_UnlockRect(srflock);
		}

		d3d9_read_front_buffer_end:
		if (bbuf) {
			IDirect3DSurface9_Release(bbuf);
		}
		info.screenshot = SCRSH_NONE;
	}
}
static BYTE d3d9_texture_create(_texture *texture, UINT index) {
	_shader_pass *sp = &shader_effect.sp[index];
	_shader_scale *sc = &sp->sc;
	const _texture_rect *prev = NULL;
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
			rect->base.w = (FLOAT)prev->base.w * sc->scale.x;
			break;
		case SHADER_SCALE_ABSOLUTE:
			rect->base.w = sc->abs.x;
			break;
		case SHADER_SCALE_VIEWPORT:
			rect->base.w = (FLOAT)gfx.vp.w * sc->scale.x;
			break;
	}
	switch (sc->type.y) {
		case SHADER_SCALE_DEFAULT:
		case SHADER_SCALE_INPUT:
			rect->base.h = (FLOAT)prev->base.h * sc->scale.y;
			break;
		case SHADER_SCALE_ABSOLUTE:
			rect->base.h = sc->abs.y;
			break;
		case SHADER_SCALE_VIEWPORT:
			rect->base.h = (FLOAT)gfx.vp.h * sc->scale.y;
			break;
	}

	rect->w = (FLOAT)emu_power_of_two(rect->base.w);
	rect->h = (FLOAT)emu_power_of_two(rect->base.h);
#else
	switch (sc->type.x) {
		case SHADER_SCALE_DEFAULT:
		case SHADER_SCALE_INPUT:
			rect->base.w = (unsigned int)((FLOAT)prev->base.w * sc->scale.x);
			rect->w = (FLOAT)prev->w * sc->scale.x;
			break;
		case SHADER_SCALE_ABSOLUTE:
			rect->base.w = sc->abs.x;
			rect->w = (FLOAT)rect->base.w;
			break;
		case SHADER_SCALE_VIEWPORT:
			rect->base.w = (unsigned int)(gfx.vp.w * sc->scale.x);
			rect->w = (FLOAT)rect->base.w;
			break;
	}
	switch (sc->type.y) {
		case SHADER_SCALE_DEFAULT:
		case SHADER_SCALE_INPUT:
			rect->base.h = (unsigned int)((FLOAT)prev->base.h * sc->scale.y);
			rect->h = (FLOAT)prev->h * sc->scale.y;
			break;
		case SHADER_SCALE_ABSOLUTE:
			rect->base.h = sc->abs.y;
			rect->h = (FLOAT)rect->base.h;
			break;
		case SHADER_SCALE_VIEWPORT:
			rect->base.h = (unsigned int)(gfx.vp.h * sc->scale.y);
			rect->h = (FLOAT)rect->base.h;
			break;
	}

	rect->w = (FLOAT)emu_power_of_two((unsigned int)rect->w);
	rect->h = (FLOAT)emu_power_of_two((unsigned int)rect->h);
#endif

	// se la scheda video supporta solo texture quadre allore devo crearle quadre
	if (d3d9.adapter->texture_square_only) {
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
		vp->w = (float)rect->base.w;
		vp->h = (float)rect->base.h;
	}

	if (IDirect3DDevice9_CreateVertexBuffer(d3d9.adapter->dev, sizeof(_vertex_buffer) * 4, 0, 0,
		D3DPOOL_DEFAULT, &texture->shader.quad, NULL) != D3D_OK) {
		gui_critical(uL("Unable to create the vertex buffer."));
		return (EXIT_ERROR);
	}

	if (IDirect3DDevice9_CreateTexture(d3d9.adapter->dev,
		(UINT)rect->w, (UINT)rect->h, 1,
		D3DUSAGE_RENDERTARGET,
		sp->fbo_flt ? D3DFMT_A32B32G32R32F : D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&texture->data,
		NULL) != D3D_OK) {
		gui_critical(uL("Unable to create the texture."));
		return (EXIT_ERROR);
	}

	IDirect3DTexture9_GetSurfaceLevel(texture->data, 0, &texture->map0);
	// cancello la superficie map0 perche' alcuni driver (tipo intel) nella
	// versione per windows XP non mi passano una superficia "pulita".
	d3d9_surface_clean(&texture->map0, (UINT)rect->w, (UINT)rect->h, 0);

	IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, (IDirect3DBaseTexture9 *)texture->data);
	IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
	IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
	IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, NULL);

	return (EXIT_OK);
}
static BYTE d3d9_texture_simple_create(_texture_simple *texture, UINT w, UINT h, BOOL overlay) {
	uint32_t clean_color = (cfg->palette == PALETTE_RAW) && !overlay ? 0x00FF0000 : 0x00;
	_texture_rect *rect = &texture->rect;
	_shader *shd = &texture->shader;
	_viewport vp = { 0, 0, (float)w, (float)h };
	UINT flt = 0;

	rect->base.w = w;
	rect->base.h = h;

	if (!overlay) {
#if defined (FH_SHADERS_GEST)
		rect->w = (FLOAT)emu_power_of_two(rect->base.w);
		rect->h = (FLOAT)emu_power_of_two(rect->base.h);
#else
		// rect->w = 1024 e rect->h = 1024 sono
		// le dimensioni che imposta retroarch
		// ma su alcune shader l'effetto e' piu'
		// sgranato ("mudlord/emboss.h" e
		// "antialiasing/fx-aa.h" sono un esempio)
		if ((w > 1024) || (h > 1024)) {
			rect->w = (FLOAT)emu_power_of_two(rect->base.w);
			rect->h = (FLOAT)emu_power_of_two(rect->base.h);
		} else {
			rect->w = 1024;
			rect->h = 1024;
		}
#endif
		flt = (cfg->interpolation || gfx.PSS) ? D3DTEXF_LINEAR : D3DTEXF_POINT;
	} else {
		rect->w = (FLOAT)rect->base.w;
		rect->h = (FLOAT)rect->base.h;
		flt = D3DTEXF_POINT;
	}

	// se la scheda video supporta solo texture quadre allore devo crearle quadre
	if (d3d9.adapter->texture_square_only) {
		if (rect->w < rect->h) {
			rect->w = rect->h;
		} else {
			rect->h = rect->w;
		}
	}

	shd->info.video_size.x = (FLOAT)rect->base.w;
	shd->info.video_size.y = (FLOAT)rect->base.h;
	shd->info.texture_size.x = rect->w;
	shd->info.texture_size.y = rect->h;

	if (IDirect3DDevice9_CreateVertexBuffer(d3d9.adapter->dev,
		sizeof(_vertex_buffer) * 4,
		D3DUSAGE_WRITEONLY,
		0,
		D3DPOOL_DEFAULT,
		&texture->shader.quad,
		NULL) != D3D_OK) {
		gui_critical(uL("Unable to create the vertex buffer."));
		return (EXIT_ERROR);
	}

	if (IDirect3DDevice9_CreateTexture(d3d9.adapter->dev,
		(UINT)rect->w, (UINT)rect->h, 1,
		d3d9.adapter->dynamic_texture ? D3DUSAGE_DYNAMIC : 0,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&texture->data,
		NULL) != D3D_OK) {
		gui_critical(uL("Unable to create the texture."));
		return (EXIT_ERROR);
	}

	IDirect3DTexture9_GetSurfaceLevel(texture->data, 0, &texture->map0);
	// cancello la superficie map0 perche' alcuni driver (tipo intel) nella
	// versione per windows XP non mi passano una superficia "pulita".
	d3d9_surface_clean(&texture->map0, (UINT)rect->w, (UINT)rect->h, clean_color);

	if (!overlay) {
		// creo la superficie temporanea le cui dimensioni non devono essere "POWerate"
		if (IDirect3DDevice9_CreateOffscreenPlainSurface(d3d9.adapter->dev,
			rect->base.w,
			rect->base.h,
			D3DFMT_A8R8G8B8,
			D3DPOOL_SYSTEMMEM,
			&texture->offscreen,
			NULL) != D3D_OK) {
			gui_critical(uL("Unable to create the memory surface."));
			return (EXIT_ERROR);
		}

		// cancello la superficie
		d3d9_surface_clean(&texture->offscreen, rect->base.w, rect->base.h, clean_color);
	} else {
		texture->offscreen = NULL;
	}

	IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, (IDirect3DBaseTexture9 *)texture->data);
	IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
	IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
	IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_MINFILTER, flt);
	IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_MAGFILTER, flt);
	IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, NULL);

	if (overlay && cfg->text_rotation) {
		d3d9_vertex_buffer_set(shd, &vp, rect, TRUE);
	} else {
		d3d9_vertex_buffer_set(shd, &vp, rect, FALSE);
	}

	return (EXIT_OK);
}
static BYTE d3d9_texture_lut_create(_lut *lut, UINT index) {
	_lut_pass *lp = &shader_effect.lp[index];
	LPDIRECT3DSURFACE9 map0 = NULL, offscreen = NULL;
	D3DLOCKED_RECT lrect;
	UINT width = 0, height = 0;

	if (gui_load_lut(lut, lp->path) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	width = lut->w;
	height = lut->h;

	lut->name = lp->name;
	lut->filter = d3d9_shader_filter(lp->linear);

	if (d3d9.adapter->texture_square_only) {
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
		gui_critical(uL("Unable to create the texture."));
		return (EXIT_ERROR);
	}

	if (IDirect3DDevice9_CreateOffscreenPlainSurface(d3d9.adapter->dev,
		lut->w, lut->h,
		D3DFMT_A8R8G8B8,
		D3DPOOL_SYSTEMMEM,
		&offscreen,
		NULL) != D3D_OK) {
		gui_critical(uL("Unable to create the memory surface."));
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
		uint32_t *sbits = (uint32_t *)lut->bits;
		uint32_t *dbits = (uint32_t *)lrect.pBits;

		for (h = 0; h < lut->h; h++) {
			for (w = 0; w < lut->w; w++) {
				(*(dbits + w)) = (*(sbits + w));
			}
			sbits += lrect.Pitch / (gfx.bit_per_pixel / 8);
			dbits += lrect.Pitch / (gfx.bit_per_pixel / 8);
		}

		IDirect3DSurface9_UnlockRect(offscreen);
		IDirect3DDevice9_UpdateSurface(d3d9.adapter->dev, offscreen, NULL, map0, NULL);

		IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, (IDirect3DBaseTexture9 *)lut->data);
		IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
		IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
		IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, NULL);

		IDirect3DSurface9_Release(map0);
		IDirect3DSurface9_Release(offscreen);
	}

	return (EXIT_OK);
}
static void d3d9_surface_clean(LPDIRECT3DSURFACE9 *surface, UINT width, UINT height, uint32_t color) {
	D3DLOCKED_RECT lock_dst;

	if (IDirect3DSurface9_LockRect((*surface), &lock_dst, NULL, D3DLOCK_DISCARD) == D3D_OK) {
		uint32_t *pbits = NULL;
		UINT w = 0, h = 0;

		pbits = (uint32_t *)lock_dst.pBits;

		for (h = 0; h < height; h++) {
			for (w = 0; w < width; w++) {
				(*(pbits + w)) = color;
			}
			pbits += lock_dst.Pitch / (gfx.bit_per_pixel / 8);
		}

		IDirect3DSurface9_UnlockRect((*surface));
	}
}
static BYTE d3d9_shader_init(UINT pass, _shader *shd, const uTCHAR *path, const char *code) {
	const char *list = NULL;
	const char *argv[128];
	const char **fopts = cgD3D9GetOptimalOptions(cgD3D9GetLatestPixelProfile());
	const char **vopts = cgD3D9GetOptimalOptions(cgD3D9GetLatestVertexProfile());
	char alias[MAX_PASS][128];
	uTCHAR base[LENGTH_FILE_NAME_MID];
	uTCHAR dname[LENGTH_FILE_NAME_MID];
	char bname[LENGTH_FILE_NAME_MID];
	UINT i = 0, argc = 0;

	if ((path != NULL) && path[0]) {
		uTCHAR buffer[LENGTH_FILE_NAME_MID];

		umemset(base, 0x00, usizeof(base));
		if (ugetcwd(base, usizeof(base)) == NULL) {}

		umemset(dname, 0x00, usizeof(dname));
		gui_utf_dirname((uTCHAR *)path, dname, usizeof(buffer) - 1);

		umemset(buffer, 0x00, usizeof(buffer));
		gui_utf_basename((uTCHAR *)path, buffer, usizeof(buffer) - 1);
		memset(bname, 0x00, sizeof(bname));
		wcstombs(bname, buffer, sizeof(bname) - 1);
	}

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
			shd->prg.f = cgCreateProgram(d3d9.cgctx, CG_SOURCE, code, cgD3D9GetLatestPixelProfile(), "main_fragment", argv);
		} else {
			if (uchdir(dname) == -1) {}
			shd->prg.f = cgCreateProgramFromFile(d3d9.cgctx, CG_SOURCE, bname, cgD3D9GetLatestPixelProfile(), "main_fragment", argv);
			if (uchdir(base) == -1) {}
		}
		if (!shd->prg.f && (list = cgGetLastListing(d3d9.cgctx))) {
			log_error(uL("cg;fragment program errors, %s"), list);
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
			shd->prg.v = cgCreateProgram(d3d9.cgctx, CG_SOURCE, code, cgD3D9GetLatestVertexProfile(), "main_vertex", argv);
		} else {
			if (uchdir(dname) == -1) {}
			shd->prg.v = cgCreateProgramFromFile(d3d9.cgctx, CG_SOURCE, bname, cgD3D9GetLatestVertexProfile(), "main_vertex", argv);
			if (uchdir(base) == -1) {}
		}
		if (!shd->prg.v && (list = cgGetLastListing(d3d9.cgctx))) {
			log_error(uL("cg;vertex program errors, %s"), list);
		}
	}

	if (!shd->prg.f || !shd->prg.v) {
		log_error(uL("cg;%s"), cgGetErrorString(cgGetError()));
		return (EXIT_ERROR);
	}

	if (cgD3D9LoadProgram(shd->prg.f, TRUE, 0) != D3D_OK) {
		log_error(uL("cg;error on loading fragment program"));
		return (EXIT_ERROR);
	}
	if (cgD3D9LoadProgram(shd->prg.v, TRUE, 0) != D3D_OK) {
		log_error(uL("cg;error on loading vertex program"));
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
			d3d9_shader_uni_texture(&shd->uni.passprev[i], &shd->prg, shader_effect.sp[i].alias);
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
		log_error(uL("cg;parameter '%s' disabled"), semantic);
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
	CGparameter param = NULL;
	char type[50], buff[100];
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
	UINT i = 0, count = 0, index = 0, tex_index = 0;
	CGparameter param = NULL;
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

	param = d3d9_cg_find_param(cgGetFirstParameter(shd->prg.v, CG_PROGRAM), "POSITION");
	if (!param) {
		param = d3d9_cg_find_param(cgGetFirstParameter(shd->prg.v, CG_PROGRAM), "POSITION0");
	}
	if (param && !indices[cgGetParameterResourceIndex(param)]) {
		stream[0] = TRUE;
		index = cgGetParameterResourceIndex(param);
		indices[index] = TRUE;
		decl[index] = (D3DVERTEXELEMENT9) {
			0, 0,
			D3DDECLTYPE_FLOAT3,
			D3DDECLMETHOD_DEFAULT,
			D3DDECLUSAGE_POSITION, 0
		};
#if !defined (RELEASE)
		log_info(uL("cg;semantic POSITION found (%d)"), index);
#endif
	}

	param = d3d9_cg_find_param(cgGetFirstParameter(shd->prg.v, CG_PROGRAM), "TEXCOORD");
	if (!param) {
		param = d3d9_cg_find_param(cgGetFirstParameter(shd->prg.v, CG_PROGRAM), "TEXCOORD0");
	}
	if (param && !indices[cgGetParameterResourceIndex(param)]) {
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
#if !defined (RELEASE)
		log_info(uL("cg;semantic TEXCOORD0 found (%d)"), index);
#endif
	}

	param = d3d9_cg_find_param(cgGetFirstParameter(shd->prg.v, CG_PROGRAM), "TEXCOORD1");
	if (param && !indices[cgGetParameterResourceIndex(param)]) {
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
#if !defined (RELEASE)
		log_info(uL("cg;semantic TEXCOORD1 found (%d)"), index);
#endif
	}

	param = d3d9_cg_find_param(cgGetFirstParameter(shd->prg.v, CG_PROGRAM), "COLOR");
	if (!param) {
		param = d3d9_cg_find_param(cgGetFirstParameter(shd->prg.v, CG_PROGRAM), "COLOR0");
	}
	if (param && !indices[cgGetParameterResourceIndex(param)]) {
		stream[3] = TRUE;
		index = cgGetParameterResourceIndex(param);
		indices[index] = TRUE;
		decl[index] = (D3DVERTEXELEMENT9) {
			3, sizeof(float) * 7,
			D3DDECLTYPE_FLOAT4,
			D3DDECLMETHOD_DEFAULT,
			D3DDECLUSAGE_COLOR, 0
		};
#if !defined (RELEASE)
		log_info(uL("cg;semantic COLOR found (%d)"), index);
#endif
	}

	// Stream {0, 1, 2, 3} might be already taken. Find first vacant stream
	for (index = 0; index < 4; index++) {
		if (!stream[index]) {
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
#if !defined (RELEASE)
			log_info(uL("cg;attrib found (%d %d %d %d)"), i, shd->attribs.count, index, tex_index);
#endif
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
static void d3d9_vertex_buffer_set(_shader *shd, _viewport *vp, _texture_rect *prev, BYTE last_pass) {
	FLOAT u = (FLOAT)prev->base.w / prev->w;
	FLOAT v = (FLOAT)prev->base.h / prev->h;
	FLOAT rotation = 0, u0 = 0.0f, u1 = u;
	void *buffer = NULL;
	UINT i = 0;

	if (last_pass) {
		if (cfg->hflip_screen) {
			u0 = u;
			u1 = 0.0f;
		}
		switch (cfg->screen_rotation) {
			case ROTATE_90:
				rotation = 270.0f;
				break;
			case ROTATE_180:
				rotation = 180.0f;
				break;
			case ROTATE_270:
				rotation = 90.0f;
				break;
		}
	}

	shd->vb[0].x = 0.0f;
	shd->vb[0].y = vp->h;
	shd->vb[0].z = 0.5f;
	shd->vb[0].u = u0;
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
	shd->vb[1].u = u1;
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
	shd->vb[2].u = u0;
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
	shd->vb[3].u = u1;
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

	IDirect3DVertexBuffer9_Lock(shd->quad, 0, 0, (void**)&buffer, 0);
	memcpy(buffer, shd->vb, sizeof(shd->vb));
	IDirect3DVertexBuffer9_Unlock(shd->quad);

	{
		D3DXMATRIX proj, ortho, rotz;

		D3DXMatrixOrthoOffCenterLH(&ortho, 0, vp->w, 0, vp->h, 0, 1);

		D3DXMatrixIdentity(&rotz);
		D3DXMatrixRotationZ(&rotz, rotation * ((FLOAT)M_PI / 180.0f));

		D3DXMatrixMultiply(&proj, &ortho, &rotz);
		D3DXMatrixTranspose(&shd->mvp, &proj);
	}
}
static CGparameter d3d9_cg_find_param(CGparameter prm, const char *name) {
	UINT i = 0;
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
			CGparameter ret = NULL;

			ret = d3d9_cg_find_param(cgGetFirstStructParameter(prm), name);
			if (ret) {
				return (ret);
			}
		}
		if ((cgGetParameterDirection(prm) != CG_IN) || (cgGetParameterVariability(prm) != CG_VARYING)) {
			continue;
		}
		semantic = cgGetParameterSemantic(prm);
		if (!semantic) {
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
INLINE static void d3d9_shader_params_overlay_set(_shader *shd) {
	UINT i = 0;

	if (shd->uni.mvp) {
		// posso tranquillamente utilizzare l'mvp dell'ultimo pass
		cgD3D9SetUniformMatrix(shd->uni.mvp, &shd->mvp);
	}

	for (i = 0; i < 4; i++) {
		IDirect3DDevice9_SetStreamSource(d3d9.adapter->dev, i, shd->quad, 0, sizeof(_vertex_buffer));
	}
}
INLINE static void d3d9_shader_param_set(const _texture *texture, UINT sindex, UINT fcountmod, UINT fcount) {
	const _shader *shd = &texture->shader;
	UINT i = 0, index = 0;

	if (shd->uni.mvp) {
		cgD3D9SetUniformMatrix(shd->uni.mvp, &shd->mvp);
	}

	for (i = 0; i < 4; i++) {
		IDirect3DDevice9_SetStreamSource(d3d9.adapter->dev, i, shd->quad, 0, sizeof(_vertex_buffer));
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
			FLOAT fc = (FLOAT)fcount;

			if (fcountmod) {
				fc = (FLOAT)(fcount % fcountmod);
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
		UINT bound_index = (UINT)-1;

		if (shd->uni.v.lut[i]) {
			index = cgGetParameterResourceIndex(shd->uni.v.lut[i]);
			bound_index = index;
			IDirect3DDevice9_SetTexture(d3d9.adapter->dev, index, (IDirect3DBaseTexture9 * )lut->data);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MAGFILTER, lut->filter);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MINFILTER, lut->filter);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
		}
		if (shd->uni.f.lut[i]) {
			index = cgGetParameterResourceIndex(shd->uni.f.lut[i]);
			if (index == bound_index) {
				continue;
			}
			IDirect3DDevice9_SetTexture(d3d9.adapter->dev, index, (IDirect3DBaseTexture9 * )lut->data);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MAGFILTER, lut->filter);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MINFILTER, lut->filter);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
		}
	}

	// ORIG
	{
		// ORIG.texture
		if (shd->uni.orig.f.texture) {
			index = cgGetParameterResourceIndex(shd->uni.orig.f.texture);
			IDirect3DDevice9_SetTexture(d3d9.adapter->dev, index, (IDirect3DBaseTexture9 * )d3d9.screen.tex[sindex].data);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
		}
		// ORIG.video_size
		if (shd->uni.orig.v.video_size) {
			cgD3D9SetUniform(shd->uni.orig.v.video_size, &d3d9.screen.tex[sindex].shader.info.video_size);
		}
		if (shd->uni.orig.f.video_size) {
			cgD3D9SetUniform(shd->uni.orig.f.video_size, &d3d9.screen.tex[sindex].shader.info.video_size);
		}
		// ORIG.texture_size
		if (shd->uni.orig.v.texture_size) {
			cgD3D9SetUniform(shd->uni.orig.v.texture_size, &d3d9.screen.tex[sindex].shader.info.texture_size);
		}
		if (shd->uni.orig.f.texture_size) {
			cgD3D9SetUniform(shd->uni.orig.f.texture_size, &d3d9.screen.tex[sindex].shader.info.texture_size);
		}
		// ORIG.tex_coord
		if (shd->uni.orig.v.tex_coord) {
			IDirect3DDevice9_SetStreamSource(d3d9.adapter->dev,
				shd->attribs.attrib[cgGetParameterResourceIndex(shd->uni.orig.v.tex_coord)],
				d3d9.screen.tex[sindex].shader.quad, 0, sizeof(_vertex_buffer));
		}
	}

	// FEEDBACK
	if (d3d9.feedback.in_use) {
		// FEEDBACK.texture
		if (shd->uni.feedback.f.texture) {
			D3DTEXTUREFILTERTYPE filter = d3d9_shader_filter(shader_effect.sp[shader_effect.feedback_pass].linear);

			index = cgGetParameterResourceIndex(shd->uni.feedback.f.texture);
			IDirect3DDevice9_SetTexture(d3d9.adapter->dev, index, (IDirect3DBaseTexture9 * )d3d9.feedback.tex.data);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MAGFILTER, filter);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MINFILTER, filter);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
		}
		// FEEDBACK.video_size
		if (shd->uni.feedback.v.video_size) {
			cgD3D9SetUniform(shd->uni.feedback.v.video_size, &d3d9.texture[shader_effect.feedback_pass].shader.info.video_size);
		}
		if (shd->uni.feedback.f.video_size) {
			cgD3D9SetUniform(shd->uni.feedback.f.video_size, &d3d9.texture[shader_effect.feedback_pass].shader.info.video_size);
		}
		// FEEDBACK.texture_size
		if (shd->uni.feedback.v.texture_size) {
			cgD3D9SetUniform(shd->uni.feedback.v.texture_size, &d3d9.texture[shader_effect.feedback_pass].shader.info.texture_size);
		}
		if (shd->uni.feedback.f.texture_size) {
			cgD3D9SetUniform(shd->uni.feedback.f.texture_size, &d3d9.texture[shader_effect.feedback_pass].shader.info.texture_size);
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
		INT circle_index = (INT)sindex - 1;

		for (i = 0; i < (d3d9.screen.in_use - 1); i++) {
			if (circle_index < 0) {
				circle_index = (INT)d3d9.screen.in_use - 1;
			}
			// PREV[x].texture
			if (shd->uni.prev[i].f.texture) {
				index = cgGetParameterResourceIndex(shd->uni.prev[i].f.texture);
				IDirect3DDevice9_SetTexture(d3d9.adapter->dev, index, (IDirect3DBaseTexture9 *)d3d9.screen.tex[circle_index].data);
				IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
				IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MINFILTER, D3DTEXF_POINT);
				IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
				IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
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

			IDirect3DDevice9_SetTexture(d3d9.adapter->dev, resind, (IDirect3DBaseTexture9 *)d3d9.texture[i].data);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, resind, D3DSAMP_MAGFILTER, filter);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, resind, D3DSAMP_MINFILTER, filter);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, resind, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, resind, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
		}
		// PASS[x].video_size
		if (shd->uni.passprev[i].v.video_size) {
			cgD3D9SetUniform(shd->uni.passprev[i].v.video_size, &d3d9.texture[next].shader.info.video_size);
		}
		if (shd->uni.passprev[i].f.video_size) {
			cgD3D9SetUniform(shd->uni.passprev[i].f.video_size, &d3d9.texture[next].shader.info.video_size);
		}
		// PASS[x].texture_size
		if (shd->uni.passprev[i].v.texture_size) {
			cgD3D9SetUniform(shd->uni.passprev[i].v.texture_size, &d3d9.texture[next].shader.info.texture_size);
		}
		if (shd->uni.passprev[i].f.texture_size) {
			cgD3D9SetUniform(shd->uni.passprev[i].f.texture_size, &d3d9.texture[next].shader.info.texture_size);
		}
		// PASS[x].tex_coord
		if (shd->uni.passprev[i].v.tex_coord) {
			IDirect3DDevice9_SetStreamSource(d3d9.adapter->dev,
				shd->attribs.attrib[cgGetParameterResourceIndex(shd->uni.passprev[i].v.tex_coord)],
				d3d9.texture[next].shader.quad, 0, sizeof(_vertex_buffer));
		}
	}
}
