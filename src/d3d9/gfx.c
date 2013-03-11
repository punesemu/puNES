/*
 * gfx.c
 *
 *  Created on: 01/mar/2013
 *      Author: fhorse
 */

#include <d3d9.h>
#include "win.h"
#include "emu.h"
#include "gfx.h"
#include "cfg_file.h"
#include "ppu.h"
#include "overscan.h"
#define __STATICPAL__
#include "palette.h"

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

enum pow_types { NO_POWER_OF_TWO, POWER_OF_TWO };

typedef struct {
	FLOAT w, h;
	FLOAT x, y;
	WORD no_pow_w, no_pow_h;
	LPDIRECT3DTEXTURE9 data;
} _texture;

struct _d3d9 {
	LPDIRECT3D9 d3d;
	LPDIRECT3DDEVICE9 dev;
	LPDIRECT3DVERTEXBUFFER9 textured_vertex;
	D3DDISPLAYMODE display_mode;

	_texture texture;

	uint32_t *palette;
	GFX_EFFECT_ROUTINE;

	DWORD flags;
	/* bit per pixel */
	BYTE bpp;
	/* byte per pixel */
	BYTE BPP;
	BOOL auto_gen_mipmap;
	BOOL dynamic_texture;
	WORD texture_create_usage;
	BOOL scale_force;
	FLOAT scale;
	FLOAT factor;
	BOOL interpolation;
} d3d9;

typedef struct {
	FLOAT x, y, z;
	FLOAT nx, ny, nz;
	FLOAT tu, tv;
} vertex;
#define FVF (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)

BYTE d3d9_create_context(void);
BYTE d3d9_create_texture(_texture *texture, uint32_t width, uint32_t height, uint8_t interpolation,
        uint8_t pow);
int d3d9_power_of_two(int base);

static BYTE ntsc_width_pixel[5] = {0, 0, 7, 10, 14};

BYTE gfx_init(void) {
	if (gui_create()) {
		fprintf(stderr, "Gui initialization failed\n");
		return (EXIT_ERROR);
	}

	cfg->filter = NO_FILTER;

	memset(&d3d9, 0x00, sizeof(d3d9));

	{
		D3DCAPS9 d3dcaps;

		if ((d3d9.d3d = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) {
			fprintf(stderr, "Unable to create d3d device\n");
			return (EXIT_ERROR);
		}

		if (IDirect3D9_GetAdapterDisplayMode(d3d9.d3d, D3DADAPTER_DEFAULT, &d3d9.display_mode)
		        != D3D_OK) {
			fprintf(stderr, "Unable to get adapter display mode\n");
			return (EXIT_ERROR);
		}

		{
			BOOL supported = FALSE;

			/* 32 bit */
			if ((d3d9.display_mode.Format == D3DFMT_X8R8G8B8)
					|| (d3d9.display_mode.Format == D3DFMT_A8R8G8B8)) {
				supported = TRUE;
				d3d9.bpp = 32;
			}
			/* 24 bit */
			if (d3d9.display_mode.Format == D3DFMT_R8G8B8) {
				supported = TRUE;
				d3d9.bpp = 24;
			}
			/* 16 bit */
			if ((d3d9.display_mode.Format == D3DFMT_A1R5G5B5)
					|| (d3d9.display_mode.Format == D3DFMT_X1R5G5B5)) {
				supported = TRUE;
				d3d9.bpp = 16;
			}
			/* 16 bit */
			if (d3d9.display_mode.Format == D3DFMT_R5G6B5) {
				supported = TRUE;
				d3d9.bpp = 16;
			}

			if (supported == FALSE) {
				fprintf(stderr, "Sorry but video mode < 16 bits are not supported\n");
				return (EXIT_ERROR);
			}
		}

		/* Check for hardware T&L */
		if (IDirect3D9_GetDeviceCaps(d3d9.d3d,
				D3DADAPTER_DEFAULT,
				D3DDEVTYPE_HAL,
				&d3dcaps) != D3D_OK) {
			fprintf(stderr, "Unable to get device caps\n");
			return (EXIT_ERROR);
		}

		/* The driver is capable of automatically generating mipmaps */
		/*
		 * The resource will automatically generate mipmaps. See Automatic
		 * Generation of Mipmaps (Direct3D 9). Automatic generation of mipmaps
		 * is not supported for volume textures and depth stencil
		 * surfaces/textures. This usage is not valid for a resource in
		 * system memory (D3DPOOL_SYSTEMMEM).
		 */
		if ((d3dcaps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP) && (IDirect3D9_CheckDeviceFormat(d3d9.d3d,
				D3DADAPTER_DEFAULT,
				D3DDEVTYPE_HAL,
				d3d9.display_mode.Format,
				D3DUSAGE_AUTOGENMIPMAP,
				D3DRTYPE_TEXTURE,
				d3d9.display_mode.Format) == D3D_OK)) {
			d3d9.auto_gen_mipmap = TRUE;
		} else {
			printf("Video driver don't support automatic generation of mipmap\n");
			d3d9.auto_gen_mipmap = FALSE;
		}

		if (d3dcaps.Caps2 & D3DCAPS2_DYNAMICTEXTURES) {
			d3d9.dynamic_texture = TRUE;
		} else {
			printf("Video driver don't support dynamic texture\n");
			d3d9.dynamic_texture = FALSE;
		}

		/*
		 * Device can accelerate a memory copy from system memory to local video memory.
		 * This cap guarantees that UpdateSurface and UpdateTexture calls will be hardware
		 * accelerated. If this cap is absent, these calls will succeed but will be slower.
		 */
		if (!(d3dcaps.Caps3 & D3DCAPS3_COPY_TO_VIDMEM)) {
			printf("Video driver don't support accelerated texture update\n");
		}

		if (d3dcaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
			d3d9.flags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
			/* Check for pure device */
			if (d3dcaps.DevCaps & D3DDEVCAPS_PUREDEVICE) {
				d3d9.flags |= D3DCREATE_PUREDEVICE;
			}
		} else {
			printf("Video driver don't support hardware accelaration\n");
			d3d9.flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}
	}

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
	if (!(d3d9.palette = malloc(NUM_COLORS * sizeof(uint32_t)))) {
		fprintf(stderr, "Out of memory\n");
		return (EXIT_ERROR);
	}

	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE);

	return (EXIT_OK);
}
void gfx_set_render(BYTE render) {
	return;
}
void gfx_set_screen(BYTE scale, BYTE filter, BYTE fullscreen, BYTE palette, BYTE force_scale) {
	BYTE set_mode;
	WORD width, height;
	//WORD w_for_pr, h_for_pr;

	//gfx_set_screen_start:
	set_mode = FALSE;
	width = 0, height = 0;
	//w_for_pr = 0, h_for_pr = 0;

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
			//case POSPHOR:
			//case SCANLINE:
			//case DBL:
			//case CRT_CURVE:
			//case CRT_NO_CURVE:
			case NO_FILTER:
			case BILINEAR:
				d3d9.effect = scale_surface;
				/*
				 * se sto passando dal filtro ntsc ad un'altro, devo
				 * ricalcolare la larghezza del video mode quindi
				 * forzo il controllo del fattore di scala.
				 */
				if (cfg->filter == NTSC_FILTER) {
					/* devo reimpostare la larghezza del video mode */
					scale = cfg->scale;
				}
				/* forzo il controllo del fattore di scale */
				force_scale = TRUE;
				/* indico che devo cambiare il video mode */
				set_mode = TRUE;
				break;
			//case SCALE2X:
			//case SCALE3X:
			//case SCALE4X:
			//	effect = scaleNx;
				/*
				 * se sto passando dal filtro ntsc ad un'altro, devo
				 * ricalcolare la larghezza del video mode quindi
				 * forzo il controllo del fattore di scala.
				 */
			//	if (cfg->filter == NTSC_FILTER) {
					/* forzo il controllo del fattore di scale */
			//		force_scale = TRUE;
					/* indico che devo cambiare il video mode */
			//		set_mode = TRUE;
			//	}
			//	break;
			//case HQ2X:
			//case HQ3X:
			//case HQ4X:
			//	effect = hqNx;
				/*
				 * se sto passando dal filtro ntsc ad un'altro, devo
				 * ricalcolare la larghezza del video mode quindi
				 * forzo il controllo del fattore di scala.
				 */
			//	if (cfg->filter == NTSC_FILTER) {
					/* forzo il controllo del fattore di scale */
			//		force_scale = TRUE;
					/* indico che devo cambiare il video mode */
			//		set_mode = TRUE;
			//	}
			//	break;
			case NTSC_FILTER:
				d3d9.effect = ntsc_surface;
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

	// .......................
	// .......................
	// .......................
	// mancano un saaaaaaacco di cose
	// .......................
	// .......................
	// .......................

	/* fattore di scala */
	if (scale == NO_CHANGE) {
		scale = cfg->scale;
	}
	if ((scale != cfg->scale) || info.on_cfg || force_scale) {
/*
#define ctrl_filter_scale(scalexf, hqxf)\
	if ((filter >= SCALE2X) && (filter <= SCALE4X)) {\
		filter = scalexf;\
	} else  if ((filter >= HQ2X) && (filter <= HQ4X)) {\
		filter = hqxf;\
	}
*/
		switch (scale) {
			case X1:
				/*
				 * il fattore di scala a 1 e' possibile
				 * solo senza filtro.
				 */
				if (filter != NO_FILTER) {
					/*
					 * con un fattore di scala X1 effect deve essere
					 * sempre impostato su scale_surface.
					 */
					d3d9.effect = scale_surface;
					return;
				}
				set_mode = TRUE;
				break;
			case X2:
				//ctrl_filter_scale(SCALE2X, HQ2X)
				ntsc_width(width, ntsc_width_pixel[scale], TRUE);
				set_mode = TRUE;
				break;
			case X3:
				//ctrl_filter_scale(SCALE3X, HQ3X)
				ntsc_width(width, ntsc_width_pixel[scale], TRUE);
				set_mode = TRUE;
				break;
			case X4:
				//ctrl_filter_scale(SCALE4X, HQ4X)
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
		//hqx_init();

		/*
		 * memorizzo i colori della paletta nel
		 * formato di visualizzazione.
		 */
		{
			WORD i;

			for (i = 0; i < NUM_COLORS; i++) {
				d3d9.palette[i] =
						D3DCOLOR_ARGB(255, palette_RGB[i].r, palette_RGB[i].g, palette_RGB[i].b);
			}
		}
	}

	/* salvo il nuovo fattore di scala */
	cfg->scale = scale;
	/* salvo ill nuovo filtro */
	cfg->filter = filter;
	/* salvo il nuovo tipo di paletta */
	cfg->palette = palette;

	{
		d3d9.scale_force = FALSE;
		d3d9.scale = cfg->scale;
		d3d9.factor = 1;
		d3d9.interpolation = FALSE;

		switch (cfg->filter) {
			case NO_FILTER:
				d3d9.scale_force = TRUE;
				d3d9.scale = X1;
				d3d9.factor = cfg->scale;
				d3d9.interpolation = FALSE;
				break;
			case BILINEAR:
				d3d9.scale_force = TRUE;
				d3d9.scale = X1;
				d3d9.factor = cfg->scale;
				d3d9.interpolation = TRUE;
				break;
		}
	}

	if (set_mode) {
		gfx.w[VIDEO_MODE] = width;
		gfx.h[VIDEO_MODE] = height;

		/*
		SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, cfg->vsync);
		*/

		if (fullscreen == TRUE) {
			gfx.w[VIDEO_MODE] = gfx.w[MONITOR];
			gfx.h[VIDEO_MODE] = gfx.h[MONITOR];
		}

		/* faccio quello che serve prima del setvideo */
		gui_set_video_mode();

		if (d3d9_create_context() == EXIT_ERROR) {
			fprintf(stderr, "Unable to initialize d3d context\n");
			return;
		}
	}

	/* setto il titolo della finestra */
	gui_update();

	if (info.on_cfg == TRUE) {
		info.on_cfg = FALSE;
	}

	return;
}
void gfx_draw_screen(BYTE forced) {
	{
		D3DLOCKED_RECT locked_rect;

		/* lock della texture */
		IDirect3DTexture9_LockRect(d3d9.texture.data, 0, &locked_rect, NULL, D3DLOCK_DISCARD);

		/* applico e l'effetto copio nella texture */
		d3d9.effect(screen.data,
				screen.line,
				d3d9.palette,
				d3d9.bpp,
				locked_rect.Pitch,
				locked_rect.pBits,
				gfx.rows,
				gfx.lines,
				d3d9.texture.no_pow_w,
				d3d9.texture.no_pow_h,
				d3d9.scale);

		/* unlock della texture */
		IDirect3DTexture9_UnlockRect(d3d9.texture.data, 0);
	}

	IDirect3DDevice9_Clear(d3d9.dev, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 0, 0, 0),
			1.0f, 0);

	IDirect3DDevice9_BeginScene(d3d9.dev);

		//IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		// select which vertex format we are using
		IDirect3DDevice9_SetFVF(d3d9.dev, FVF);
		// select the vertex buffer to display
		IDirect3DDevice9_SetStreamSource(d3d9.dev, 0, d3d9.textured_vertex, 0, sizeof(vertex));
		// copy the vertex buffer to the back buffer
		IDirect3DDevice9_DrawPrimitive(d3d9.dev, D3DPT_TRIANGLEFAN, 0, 2);
		//IDirect3DDevice9_DrawPrimitive(d3d9.dev, D3DPT_TRIANGLELIST, 0, 2);

	IDirect3DDevice9_EndScene(d3d9.dev);

	IDirect3DDevice9_Present(d3d9.dev, NULL, NULL, NULL, NULL);
}
void gfx_reset_video(void) {
	return;
}
void gfx_quit(void) {
	ntsc_quit();

	if (d3d9.palette) {
		free(d3d9.palette);
		d3d9.palette = NULL;
	}

	if (d3d9.texture.data) {
		IDirect3DTexture9_Release(d3d9.texture.data);
		d3d9.texture.data = NULL;
	}

	if (d3d9.textured_vertex) {
		IDirect3DVertexBuffer9_Release(d3d9.textured_vertex);
		d3d9.textured_vertex = NULL;
	}

	if (d3d9.dev) {
		IDirect3DDevice9_Release(d3d9.dev);
		d3d9.dev = NULL;
	}

	if (d3d9.d3d) {
		IDirect3D9_Release(d3d9.d3d);
		d3d9.d3d = NULL;
	}

    return;
}





BYTE d3d9_create_context(void) {

	if (d3d9.textured_vertex) {
		IDirect3DVertexBuffer9_Release(d3d9.textured_vertex);
		d3d9.textured_vertex = NULL;
	}

	if (d3d9.dev) {
		IDirect3DDevice9_Release(d3d9.dev);
		d3d9.dev = NULL;
	}

	{
		D3DPRESENT_PARAMETERS d3dpp;

		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow = gui_window_id();
		d3dpp.BackBufferCount = 1;
		d3dpp.BackBufferFormat = d3d9.display_mode.Format;
		d3dpp.BackBufferWidth = gfx.w[VIDEO_MODE];
		d3dpp.BackBufferHeight = gfx.h[VIDEO_MODE];
		d3dpp.MultiSampleQuality = D3DMULTISAMPLE_NONE;

		if (IDirect3D9_CreateDevice(d3d9.d3d,
				D3DADAPTER_DEFAULT,
				D3DDEVTYPE_HAL,
				gui_window_id(),
				d3d9.flags,
				&d3dpp,
				&d3d9.dev) != D3D_OK) {
			fprintf(stderr, "Unable to create d3d device\n");
			return (EXIT_ERROR);
		}
	}

	if (IDirect3DDevice9_CreateVertexBuffer(d3d9.dev,
			4 * sizeof(vertex),
			D3DUSAGE_WRITEONLY,
			FVF,
			D3DPOOL_DEFAULT,
			&d3d9.textured_vertex,
			NULL) != D3D_OK) {
		fprintf(stderr, "Unable to create the vertex buffer\n");
		return (EXIT_ERROR);
	}

	{
		{
			WORD w, h;

			if (d3d9.scale_force) {
				w = SCR_ROWS * d3d9.scale;
				h = SCR_LINES * d3d9.scale;
			} else {
				w = gfx.w[CURRENT];
				h = gfx.h[CURRENT];
			}

			/* creo la texture principale */
			if (d3d9_create_texture(&d3d9.texture, w, h, 0, POWER_OF_TWO) == EXIT_ERROR) {
				fprintf(stderr, "Unable to create main texture\n");
				return (EXIT_ERROR);
			}
		}

		IDirect3DDevice9_SetTexture(d3d9.dev, 0, (IDirect3DBaseTexture9 *) d3d9.texture.data);

		if (d3d9.interpolation == TRUE) {
			 /* bilinear filtering */
			IDirect3DDevice9_SetSamplerState(d3d9.dev, 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice9_SetSamplerState(d3d9.dev, 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			//IDirect3DDevice9_SetSamplerState(d3d9.dev, 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice9_SetSamplerState(d3d9.dev, 0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
		} else {
			IDirect3DDevice9_SetSamplerState(d3d9.dev, 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			IDirect3DDevice9_SetSamplerState(d3d9.dev, 0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			IDirect3DDevice9_SetSamplerState(d3d9.dev, 0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
		}

		/* Enable Z-Buffer (Depth Buffer) */
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_ZENABLE,  FALSE );
		/* Disable Backface Culling */
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_CULLMODE, FALSE);

		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_LIGHTING, FALSE);

		/*
		 * Su moltissime schede video, quando la texture veniva disegnata sui due triangoli
		 * la meta' sul secondo era traslasta di un pixel rispetto al primo sia sull'asse
		 * verticale che su quello orizzontale. In poche parole le proorzioni della texture
		 * erano sbagliate. Per correggerle ho dovuto sottrarre 1 a gfx.w[CURRENT] e
		 * gfx.h[CURRENT].
		 */
		d3d9.texture.x = (FLOAT) (gfx.w[CURRENT] - 1) / (d3d9.texture.w * (FLOAT) d3d9.factor);
		d3d9.texture.y = (FLOAT) (gfx.h[CURRENT] - 1) / (d3d9.texture.h * (FLOAT) d3d9.factor);

		{
			VOID *tv_vertices;
			const vertex vertices[] = {
				{-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f          , d3d9.texture.y },
				{-1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f          , 0.0f           },
				{ 1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, d3d9.texture.x, 0.0f           },
				{ 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, d3d9.texture.x, d3d9.texture.y }
			};

			IDirect3DVertexBuffer9_Lock(d3d9.textured_vertex, 0, 0, (void**) &tv_vertices, 0);
			memcpy(tv_vertices, vertices, sizeof(vertices));
			IDirect3DVertexBuffer9_Unlock(d3d9.textured_vertex);
		}
	}

	return (EXIT_OK);
}


BYTE d3d9_create_texture(_texture *texture, uint32_t width, uint32_t height, uint8_t interpolation,
        uint8_t pow) {
	DWORD usage = 0;

	texture->no_pow_w = width;
	texture->no_pow_h = height;

	if (pow) {
		texture->w = d3d9_power_of_two(width);
		texture->h = d3d9_power_of_two(height);
	} else {
		texture->w = width;
		texture->h = height;
	}

	if (texture->data) {
		IDirect3DTexture9_Release(texture->data);
	}

	if (d3d9.dynamic_texture == TRUE) {
		usage |= D3DUSAGE_DYNAMIC;
	}
	if (d3d9.auto_gen_mipmap == TRUE) {
		usage |= D3DUSAGE_AUTOGENMIPMAP;
	}

	if (IDirect3DDevice9_CreateTexture(d3d9.dev,
			texture->w,
			texture->h,
			0,
			usage,
			d3d9.display_mode.Format,
			D3DPOOL_DEFAULT,
			&texture->data,
			NULL) != D3D_OK) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}

int d3d9_power_of_two(int base) {
	int pot = 1;

	while (pot < base) {
		pot <<= 1;
	}
	return (pot);
}
