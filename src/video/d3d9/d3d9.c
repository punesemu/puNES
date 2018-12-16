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
#include "info.h"
#include "gui.h"
#include "conf.h"
#include "ppu.h"
#include "overscan.h"

#define _SCR_ROWS_BRD\
	((float)  (SCR_ROWS - (overscan.borders->left + overscan.borders->right)) * gfx.pixel_aspect_ratio)
#define _SCR_LINES_BRD\
	(float) (SCR_LINES - (overscan.borders->up + overscan.borders->down))
#define _SCR_ROWS_NOBRD\
	((float) SCR_ROWS * gfx.pixel_aspect_ratio)
#define _SCR_LINES_NOBRD\
	(float) SCR_LINES

static void d3d9_shader_cg_error_handler(void);
static BYTE d3d9_device_create(UINT width, UINT height);
static void d3d9_context_delete(void);
static BYTE d3d9_texture_create(_texture *texture, UINT index);
static BYTE d3d9_texture_simple_create(_texture_simple *texture, UINT w, UINT h, BOOL text);
static BYTE d3d9_texture_lut_create(_lut *lut, UINT index);
static void d3d9_surface_clean(LPDIRECT3DSURFACE9 *surface, UINT width, UINT height);
static BYTE d3d9_shader_init(UINT pass, _shader *shd, const uTCHAR *path, const char *code);
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

BYTE d3d9_init(void) {
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
		unsigned int adapt;

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

			if (IDirect3D9_GetAdapterDisplayMode(d3d9.d3d, dev->id, &dev->display_mode) != D3D_OK) {
				dev_error("unable to get adapter display mode\n");
				continue;
			}

			if ((dev->display_mode.Format == D3DFMT_X8R8G8B8) ||
				(dev->display_mode.Format == D3DFMT_A8R8G8B8)) {
				dev->bit_per_pixel = 32;
			}
			if (dev->bit_per_pixel < 32) {
				dev_error("video mode < 32 bits are not supported\n");
				continue;
			}

			// Check for hardware T&L
			if (IDirect3D9_GetDeviceCaps(d3d9.d3d, dev->id, D3DDEVTYPE_HAL, &d3dcaps) != D3D_OK) {
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

	return (EXIT_OK);
}
BYTE d3d9_context_create(void) {
	D3DXMATRIX identity;
	WORD w, h;
	UINT i;

	d3d9_context_delete();

	if (overscan.enabled && (!cfg->oscan_black_borders && !cfg->fullscreen)) {
		// visto che lavorero' con texture piu' grandi del video mode
		// setto un backbuffer piu' grande.
		w = gfx.w[VIDEO_MODE] * 2;
		h = gfx.h[VIDEO_MODE] * 2;
	} else {
		w = gfx.w[VIDEO_MODE];
		h = gfx.h[VIDEO_MODE];
	}

	if (d3d9_device_create(w, h) == EXIT_ERROR) {
		d3d9_context_delete();
		return (EXIT_ERROR);
	}

	if ((d3d9.cgctx = cgCreateContext()) == NULL) {
		d3d9_context_delete();
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

		if (overscan.enabled && (!cfg->oscan_black_borders && !cfg->fullscreen)) {
			vp->x = (-overscan.borders->left * gfx.width_pixel) * gfx.pixel_aspect_ratio;
			vp->y = (-overscan.borders->up * cfg->scale);
			vp->w = gfx.w[NO_OVERSCAN] * gfx.pixel_aspect_ratio;
			vp->h = gfx.h[NO_OVERSCAN];
		}

		// configuro l'aspect ratio del fullscreen
		if (cfg->fullscreen) {
			if (!cfg->stretch) {
				float ratio_frame = (float)gfx.w[VIDEO_MODE] / (float)gfx.h[VIDEO_MODE];
				float ratio_surface;

				if (overscan.enabled && (cfg->oscan_black_borders_fscr == FALSE)) {
					ratio_surface = _SCR_ROWS_BRD / _SCR_LINES_BRD;
				} else {
					ratio_surface = _SCR_ROWS_NOBRD / _SCR_LINES_NOBRD;
				}

				if (ratio_frame > ratio_surface) {
					vp->w = (int)((float)gfx.h[VIDEO_MODE] * ratio_surface);
					vp->x = (int)(((float)gfx.w[VIDEO_MODE] - (float)vp->w) * 0.5f);
				} else {
					vp->h = (int)((float)gfx.w[VIDEO_MODE] / ratio_surface);
					vp->y = (int)(((float)gfx.h[VIDEO_MODE] - (float)vp->h) * 0.5f);
				}
			}

			if (overscan.enabled && (cfg->oscan_black_borders_fscr == FALSE)) {
				float brd_l_x, brd_r_x, brd_u_y, brd_d_y;
				float ratio_x, ratio_y;

				ratio_x = (float)vp->w / _SCR_ROWS_NOBRD;
				ratio_y = (float)vp->h / _SCR_LINES_NOBRD;
				brd_l_x = (float)overscan.borders->left * ratio_x;
				brd_r_x = (float)overscan.borders->right * ratio_x;
				brd_u_y = (float)overscan.borders->up * ratio_y;
				brd_d_y = (float)overscan.borders->down * ratio_y;

				d3d9.viewp.left = brd_l_x;
				d3d9.viewp.top = brd_u_y;
				d3d9.viewp.right = gfx.w[VIDEO_MODE] - brd_r_x;
				d3d9.viewp.bottom = gfx.h[VIDEO_MODE] - brd_d_y;
			} else {
				d3d9.viewp.left = 0;
				d3d9.viewp.top = 0;
				d3d9.viewp.right = gfx.w[VIDEO_MODE];
				d3d9.viewp.bottom = gfx.h[VIDEO_MODE];
			}
		} else {
			d3d9.viewp.left = -vp->x;
			d3d9.viewp.top = -vp->y;
			d3d9.viewp.right = gfx.w[VIDEO_MODE] + d3d9.viewp.left;
			d3d9.viewp.bottom = gfx.h[VIDEO_MODE] + d3d9.viewp.top;
		}
	}

	// texture
	for (i = 0; i < shader_effect.pass; i++) {
		fprintf(stderr, "D3D9: Setting pass %d.\n", i);

		if (d3d9_texture_create(&d3d9.texture[i], i) == EXIT_ERROR) {
			d3d9_context_delete();
			return (EXIT_ERROR);
		}

		if (d3d9_shader_init(i, &d3d9.texture[i].shader, shader_effect.sp[i].path, shader_effect.sp[i].code) == EXIT_ERROR) {
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

		text.w = d3d9.text.rect.w;
		text.h = d3d9.text.rect.h;

		gfx_text_reset();

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

	umemcpy(gfx.last_shader_file, cfg->shader_file, usizeof(gfx.last_shader_file));

	return (EXIT_OK);
}
void d3d9_draw_scene(void) {
	const _texture_simple *scrtex = &d3d9.screen.tex[d3d9.screen.index];
	LPDIRECT3DSURFACE9 back_buffer;
	UINT i;

	if (gui.start == FALSE) {
		return;
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
		if (cfg->fullscreen) {
			d3d9_viewport_set(texture->vp.x, texture->vp.y, texture->vp.w, texture->vp.h);
		} else {
			d3d9_viewport_set(0, 0, texture->vp.w, texture->vp.h);
		}

		cgD3D9BindProgram(texture->shader.prg.f);
		cgD3D9BindProgram(texture->shader.prg.v);

		if (i == 0) {
			IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, (IDirect3DBaseTexture9 * )scrtex->data);
		} else {
			IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, (IDirect3DBaseTexture9 * )d3d9.texture[i - 1].data);
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
		IDirect3DDevice9_UpdateSurface(d3d9.adapter->dev, d3d9.text.offscreen, NULL, d3d9.text.map0, NULL);

		if (cfg->fullscreen) {
			d3d9_viewport_set(0, 0, d3d9.text.rect.w, d3d9.text.rect.h);
		} else {
			d3d9_viewport_set(-gfx.vp.x, -gfx.vp.y, d3d9.text.rect.w, d3d9.text.rect.h);
		}

		cgD3D9BindProgram(d3d9.text.shader.prg.f);
		cgD3D9BindProgram(d3d9.text.shader.prg.v);

		IDirect3DDevice9_SetTexture(d3d9.adapter->dev, 0, (IDirect3DBaseTexture9 *)d3d9.text.data);

		IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, 0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

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
	{
		if (IDirect3DDevice9_Present(d3d9.adapter->dev, &d3d9.viewp, NULL, NULL, NULL) == D3DERR_DEVICELOST) {
			if (IDirect3DDevice9_TestCooperativeLevel(d3d9.adapter->dev) == D3DERR_DEVICENOTRESET) {
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

			if (IDirect3DDevice9_GetBackBuffer(d3d9.adapter->dev, 0, 0, D3DBACKBUFFER_TYPE_MONO, &back_buffer) == D3D_OK) {
				D3DSURFACE_DESC sd;

				IDirect3DSurface9_GetDesc(back_buffer, &sd);

				if (IDirect3DDevice9_CreateOffscreenPlainSurface(d3d9.adapter->dev, sd.Width,
					sd.Height, sd.Format, D3DPOOL_SYSTEMMEM, &surface, NULL) == D3D_OK) {
					if (IDirect3DDevice9_GetRenderTargetData(d3d9.adapter->dev, back_buffer, surface) == D3D_OK) {
						D3DLOCKED_RECT lrect;

						if (overscan.enabled && ((!cfg->fullscreen && !cfg->oscan_black_borders) ||
							(cfg->fullscreen && !cfg->oscan_black_borders_fscr))) {
							int w = d3d9.viewp.right - d3d9.viewp.left;
							int h = d3d9.viewp.bottom - d3d9.viewp.top;
							IDirect3DSurface9 *zone;

							if (IDirect3DDevice9_CreateOffscreenPlainSurface(d3d9.adapter->dev,
								w, h, sd.Format, D3DPOOL_DEFAULT, &zone, NULL) == D3D_OK) {
								if (IDirect3DDevice9_UpdateSurface(d3d9.adapter->dev, surface, &d3d9.viewp, zone, NULL) == D3D_OK) {
									IDirect3DSurface9_LockRect(zone, &lrect, NULL, 0);
									gui_save_screenshot(w, h, lrect.pBits, FALSE);
									IDirect3DSurface9_UnlockRect(zone);
								}
								IDirect3DSurface9_Release(zone);
							}
						} else {
							IDirect3DSurface9_LockRect(surface, &lrect, NULL, 0);
							gui_save_screenshot(sd.Width, sd.Height, lrect.pBits, FALSE);
							IDirect3DSurface9_UnlockRect(surface);
						}
					}
					IDirect3DSurface9_Release(surface);
				}
				IDirect3DSurface9_Release(back_buffer);
			}
			gfx.save_screenshot = FALSE;
		}
	}
}
void d3d9_quit(void) {
	d3d9_context_delete();

	{
		UINT i;

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

static void d3d9_shader_cg_error_handler(void) {
	CGerror error = cgGetError();

	if (error == (CGerror) cgD3D9Failed) {
		fprintf(stderr, "D3D9: Error '%s' occurred.\n", cgD3D9TranslateHRESULT(cgD3D9GetLastError()));
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
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	d3dpp.hDeviceWindow = gui_screen_id();
	d3dpp.BackBufferCount = 1;
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
		MessageBox(NULL, "Unable to create d3d device", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

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

	rect->w = emu_power_of_two(rect->base.w);
	rect->h = emu_power_of_two(rect->base.h);
#else
	switch (sc->type.x) {
		case SHADER_SCALE_DEFAULT:
		case SHADER_SCALE_INPUT:
			rect->w = (FLOAT)prev->w * sc->scale.x;
			rect->base.w = (FLOAT)prev->base.w * sc->scale.x;
			break;
		case SHADER_SCALE_ABSOLUTE:
			rect->w = rect->base.w = sc->abs.x;
			break;
		case SHADER_SCALE_VIEWPORT:
			rect->w = rect->base.w = (FLOAT)gfx.vp.w * sc->scale.x;
			break;
	}
	switch (sc->type.y) {
		case SHADER_SCALE_DEFAULT:
		case SHADER_SCALE_INPUT:
			rect->h = (FLOAT)prev->h * sc->scale.y;
			rect->base.h = (FLOAT)prev->base.h * sc->scale.y;
			break;
		case SHADER_SCALE_ABSOLUTE:
			rect->h = rect->base.h = sc->abs.y;
			break;
		case SHADER_SCALE_VIEWPORT:
			rect->h = rect->base.h = (FLOAT)gfx.vp.h * sc->scale.y;
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
		MessageBox(NULL, "Unable to create the vertex buffer", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	if (IDirect3DDevice9_CreateTexture(d3d9.adapter->dev,
		(UINT) rect->w, (UINT) rect->h, 1,
		D3DUSAGE_RENDERTARGET,
		sp->fbo_flt ? D3DFMT_A32B32G32R32F : D3DFMT_A8R8G8B8,
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
		MessageBox(NULL, "Unable to create the vertex buffer", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	if (IDirect3DDevice9_CreateTexture(d3d9.adapter->dev,
		(UINT)rect->w, (UINT)rect->h, 1,
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
		MessageBox(NULL, "Unable to create the memory surface", "Error!", MB_ICONEXCLAMATION | MB_OK);
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
		MessageBox(NULL, "Unable to create the texture", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return (EXIT_ERROR);
	}

	if (IDirect3DDevice9_CreateOffscreenPlainSurface(d3d9.adapter->dev,
		lut->w, lut->h,
		D3DFMT_A8R8G8B8,
		D3DPOOL_SYSTEMMEM,
		&offscreen,
		NULL) != D3D_OK) {
		MessageBox(NULL, "Unable to create the memory surface", "Error!", MB_ICONEXCLAMATION | MB_OK);
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
static void d3d9_surface_clean(LPDIRECT3DSURFACE9 *surface, UINT width, UINT height) {
	D3DLOCKED_RECT lock_dst;

	if (IDirect3DSurface9_LockRect((*surface), &lock_dst, NULL, D3DLOCK_DISCARD) == D3D_OK) {
		uint32_t *pbits;
		UINT w, h;

		pbits = (uint32_t *)lock_dst.pBits;

		for (h = 0; h < height; h++) {
			for (w = 0; w < width; w++) {
				(*(pbits + w)) = 0;
			}
			pbits += lock_dst.Pitch / (gfx.bit_per_pixel / 8);
		}

		IDirect3DSurface9_UnlockRect((*surface));
	}
}
static BYTE d3d9_shader_init(UINT pass, _shader *shd, const uTCHAR *path, const char *code) {
	const char *list;
	const char *argv[128];
	const char **fopts = cgD3D9GetOptimalOptions(cgD3D9GetLatestPixelProfile());
	const char **vopts = cgD3D9GetOptimalOptions(cgD3D9GetLatestVertexProfile());
	char alias[MAX_PASS][128];
	uTCHAR base[LENGTH_FILE_NAME_MID];
	uTCHAR dname[LENGTH_FILE_NAME_MID];
	char bname[LENGTH_FILE_NAME_MID];
	UINT i, argc;

	if ((path != NULL) && path[0]) {
		uTCHAR buffer[LENGTH_FILE_NAME_MID];

		umemset(base, 0x00, usizeof(base));
		if (ugetcwd(base, usizeof(base)) == NULL) { ; };

		umemset(dname, 0x00, usizeof(dname));
		gui_utf_dirname((uTCHAR *) path, dname, usizeof(buffer) - 1);

		umemset(buffer, 0x00, usizeof(buffer));
		gui_utf_basename((uTCHAR *) path, buffer, usizeof(buffer) - 1);
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
			if (uchdir(dname) == -1) { ; }
			shd->prg.f = cgCreateProgramFromFile(d3d9.cgctx, CG_SOURCE, bname, cgD3D9GetLatestPixelProfile(), "main_fragment", argv);
			if (uchdir(base) == -1) { ; }
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
			shd->prg.v = cgCreateProgram(d3d9.cgctx, CG_SOURCE, code, cgD3D9GetLatestVertexProfile(), "main_vertex", argv);
		} else {
			if (uchdir(dname) == -1) { ; }
			shd->prg.v = cgCreateProgramFromFile(d3d9.cgctx, CG_SOURCE, bname, cgD3D9GetLatestVertexProfile(), "main_vertex", argv);
			if (uchdir(base) == -1) { ; }
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
			fprintf(stderr, "CG: attrib found (%d %d %d %d)\n", i, shd->attribs.count, index, tex_index);

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
	FLOAT u = (FLOAT)prev->base.w / prev->w;
	FLOAT v = (FLOAT)prev->base.h / prev->h;
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
		IDirect3DDevice9_SetStreamSource(d3d9.adapter->dev, i, shd->quad, 0, sizeof(_vertex_buffer));
	}
}
INLINE static void d3d9_shader_param_set(const _texture *texture, UINT fcountmod, UINT fcount) {
	const _shader *shd = &texture->shader;
	UINT i, index;

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
			IDirect3DDevice9_SetTexture(d3d9.adapter->dev, index, (IDirect3DBaseTexture9 * )d3d9.screen.tex[d3d9.screen.index].data);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
			IDirect3DDevice9_SetSamplerState(d3d9.adapter->dev, index, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
		}
		// ORIG.video_size
		if (shd->uni.orig.v.video_size) {
			cgD3D9SetUniform(shd->uni.orig.v.video_size, &d3d9.screen.tex[d3d9.screen.index].shader.info.video_size);
		}
		if (shd->uni.orig.f.video_size) {
			cgD3D9SetUniform(shd->uni.orig.f.video_size, &d3d9.screen.tex[d3d9.screen.index].shader.info.video_size);
		}
		// ORIG.texture_size
		if (shd->uni.orig.v.texture_size) {
			cgD3D9SetUniform(shd->uni.orig.v.texture_size, &d3d9.screen.tex[d3d9.screen.index].shader.info.texture_size);
		}
		if (shd->uni.orig.f.texture_size) {
			cgD3D9SetUniform(shd->uni.orig.f.texture_size, &d3d9.screen.tex[d3d9.screen.index].shader.info.texture_size);
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
		INT circle_index = d3d9.screen.index - 1;

		for (i = 0; i < (d3d9.screen.in_use - 1); i++) {
			if (circle_index < 0) {
				circle_index = d3d9.screen.in_use - 1;
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
