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

#include "video/gfx.h"
#include "video/effects/pause.h"
#include "d3d9.h"
#include "gui.h"
#include "ppu.h"
#include "info.h"

BYTE gfx_api_init(void) {
	return (d3d9_init());
}
void gfx_api_quit(void) {
	d3d9_quit();
}
BYTE gfx_api_context_create(void) {
	return (d3d9_context_create());
}
uint32_t gfx_api_color(BYTE a, BYTE r, BYTE g, BYTE b) {
	return (D3DCOLOR_ARGB(a, r, g, b));
}
void gfx_api_overlay_blit(void *surface, _gfx_rect *rect, double device_pixel_ratio) {
	D3DLOCKED_RECT lock_dst;
	RECT dst;
	LONG pitch = 0, pitch_dpr = 0;
	unsigned char *psrc = NULL, *pdst = NULL;
	int h = 0;

	if (((rect->x + rect->w) > d3d9.overlay.rect.w) || ((rect->y + rect->h) > d3d9.overlay.rect.h)) {
		return;
	}

	dst.left = (LONG)rect->x;
	dst.top = (LONG)rect->y;
	dst.right = (LONG)(rect->x + rect->w);
	dst.bottom = (LONG)(rect->y + rect->h);

	if (IDirect3DSurface9_LockRect(d3d9.overlay.map0, &lock_dst, &dst, D3DLOCK_NO_DIRTY_UPDATE) != D3D_OK) {
		log_warning(uL("d3d9;LockRect overlay surface error"));
		return;
	}

	pitch = (LONG)(rect->w * ((float)gfx.bit_per_pixel / 8.0f));
	pitch_dpr = (LONG)((double)pitch * device_pixel_ratio);
	psrc = (unsigned char *)surface;
	pdst = (unsigned char *)lock_dst.pBits;

	for (h = (int)rect->h; h > 0; h--) {
		memcpy(pdst, psrc, pitch);
		psrc += pitch_dpr;
		pdst += lock_dst.Pitch;
	}

	IDirect3DSurface9_UnlockRect(d3d9.overlay.map0);
	IDirect3DTexture9_AddDirtyRect(d3d9.overlay.data, &dst);
}
void gfx_api_apply_filter(BYTE nidx) {
	const _texture_simple *scrtex = NULL;

	scrtex = &d3d9.screen.tex[d3d9.screen.index];

	if (scrtex->offscreen) {
		BYTE apply = !info.pause;

		gfx.frame.filtered = nes[nidx].p.ppu_screen.rd->frame;

		if (info.pause && pause_effect.frames) {
			pause_effect.frames--;
			apply = TRUE;
		}
		if (apply) {
			D3DLOCKED_RECT lrect;

			// lock della surface in memoria
			IDirect3DSurface9_LockRect(scrtex->offscreen, &lrect, NULL, D3DLOCK_DISCARD);

			// applico l'effetto
			gfx.filter.data.pitch = lrect.Pitch;
			gfx.filter.data.pix = lrect.pBits;
			gfx.filter.data.width = scrtex->rect.base.w;
			gfx.filter.data.height = scrtex->rect.base.h;
			gfx.filter.func(nidx);

			// unlock della surface in memoria
			IDirect3DSurface9_UnlockRect(scrtex->offscreen);
		}
	}
}
void gfx_api_control_changed_adapter(void *monitor) {
	HMONITOR *in_use = monitor;
	unsigned int i = 0;

	if ((*in_use) == IDirect3D9_GetAdapterMonitor(d3d9.d3d, d3d9.adapter->id)) {
		return;
	}

	for (i = 0; i < d3d9.adapters_in_use; i++) {
		_d3d9_adapter *adapter = D3D9_ADAPTER(i);

		if ((*in_use) == IDirect3D9_GetAdapterMonitor(d3d9.d3d, adapter->id)) {
			d3d9_context_delete(TRUE);
			d3d9.adapter = adapter;
			gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
			break;
		}
	}
}
