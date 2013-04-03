/*
 * gfx.c
 *
 *  Created on: 01/mar/2013
 *      Author: fhorse
 */

#include <d3d9.h>
#include <d3dx9shader.h>
#include "win.h"
#include "emu.h"
#include "gfx.h"
#include "cfg_file.h"
#include "ppu.h"
#include "overscan.h"
#define _SHADERS_CODE_
#include "shaders.h"
#undef  _SHADERS_CODE_
#define __STATICPAL__
#include "palette.h"
#undef  __STATICPAL__

#define COMPILERSHADER_NOT_FOUND 0x8007007e

#define ID3DXBuffer_GetBufferPointer(p) (p)->lpVtbl->GetBufferPointer(p)
#define ID3DXBuffer_Release(p) (p)->lpVtbl->Release(p)
#define ID3DXConstantTable_SetFloatArray(p,a,b,c,d) (p)->lpVtbl->SetFloatArray(p,a,b,c,d)
#define ID3DXConstantTable_SetMatrix(p,a,b,c) (p)->lpVtbl->SetMatrix(p,a,b,c)

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

enum power_of_two_switch { NO_POWER_OF_TWO, POWER_OF_TWO };

typedef struct {
	FLOAT l, r;
	FLOAT t, b;
} _texcoords;
struct _d3d9 {
	LPDIRECT3D9 d3d;
	LPDIRECT3DDEVICE9 dev;
	LPDIRECT3DVERTEXBUFFER9 quad;
	D3DDISPLAYMODE display_mode;

	D3DXMATRIX world;
	D3DXMATRIX view;
	D3DXMATRIX projection;

	_texture texture;
	_texcoords texcoords;
	_texcoords quadcoords;

	uint32_t *palette;

	_shader shader;

	DWORD flags;
	WORD number_of_adapters;
	BOOL auto_gen_mipmap;
	BOOL dynamic_texture;
	BOOL texture_square_only;
	WORD texture_create_usage;
	BOOL scale_force;
	FLOAT scale;
	FLOAT factor;
	BOOL interpolation;
} d3d9;
typedef struct {
	FLOAT x, y, z, rhw;
	FLOAT tu, tv;
} vertex;
#define FVF (D3DFVF_XYZRHW | D3DFVF_TEX1)

BYTE d3d9_create_context(UINT width, UINT height);
void d3d9_release_context(void);
BYTE d3d9_create_device(UINT width, UINT height);
BYTE d3d9_create_texture(_texture *texture, uint32_t width, uint32_t height, uint8_t interpolation,
        uint8_t pow);
void d3d9_release_texture(_texture *texture);
BYTE d3d9_create_shader(_shader *shd);
void d3d9_release_shader(_shader *shd);
int d3d9_power_of_two(int base);

static BYTE ntsc_width_pixel[5] = {0, 0, 7, 10, 14};

BYTE gfx_init(void) {
	/* casi particolari provenienti dal cfg_file_parse() e cmd_line_parse() */
	if ((cfg->scale == X1) && (cfg->filter != NO_FILTER)) {
		cfg->scale = X2;
	}

	overscan.left = 8;
	overscan.right = 9;
	overscan.up = 8;
	overscan.down = 8;

	if (gui_create() == EXIT_ERROR) {
		fprintf(stderr, "Gui initialization failed\n");
		return (EXIT_ERROR);
	}

	memset(&d3d9, 0x00, sizeof(d3d9));

	/* TODO : una volta aggiunta la lettura del file di configurazione devo
	 * eliminare la riga sotto.
	 */
	cfg->filter =  NO_FILTER;

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
				gfx.bit_per_pixel = 32;
			}
			/* 24 bit */
			if (d3d9.display_mode.Format == D3DFMT_R8G8B8) {
				supported = TRUE;
				gfx.bit_per_pixel = 24;
			}
			/* 16 bit */
			if ((d3d9.display_mode.Format == D3DFMT_A1R5G5B5)
					|| (d3d9.display_mode.Format == D3DFMT_X1R5G5B5)) {
				supported = TRUE;
				gfx.bit_per_pixel = 16;
			}
			/* 16 bit */
			if (d3d9.display_mode.Format == D3DFMT_R5G6B5) {
				supported = TRUE;
				gfx.bit_per_pixel = 16;
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

		if (d3dcaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) {
			printf("Video driver support only square texture\n");
			d3d9.texture_square_only = TRUE;
		} else {
			d3d9.texture_square_only = FALSE;
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
			/*
			 * se abilito il PURE DEVICE, non posso utilizzare il IDirect3DDevice9_GetTransform
			 * quando uso le shaders.
			 */
			if (d3dcaps.DevCaps & D3DDEVCAPS_PUREDEVICE) {
				d3d9.flags |= D3DCREATE_PUREDEVICE;
			}
		} else {
			printf("Video driver don't support hardware accelaration\n");
			d3d9.flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}

		d3d9.number_of_adapters = d3dcaps.NumberOfAdaptersInGroup;
		d3d9.flags |= D3DCREATE_ADAPTERGROUP_DEVICE;
		printf("adapters : %d\n", d3d9.number_of_adapters);

		/* per poter verificare se le shaders sono utilizzabili devo creare il dev d3d */
		if (d3d9_create_device(1, 1) != EXIT_OK) {
			return (EXIT_ERROR);
		}

		{
			gfx.hlsl.compliant = FALSE;

			if (d3dcaps.PixelShaderVersion < D3DPS_VERSION(2, 0) ||
					(d3dcaps.VertexShaderVersion < D3DVS_VERSION(2, 0))) {
				printf("Video driver don't support shaders >= 2.0\n");
			} else {
				_shader tmp;

				memset(&tmp, 0x00, sizeof(_shader));

				tmp.id = SHADER_COLOR;

				if (d3d9_create_shader(&tmp) == EXIT_OK) {
					d3d9_release_shader(&tmp);
					gfx.hlsl.compliant = TRUE;
				}
			}

			if (gfx.hlsl.compliant == FALSE) {
				printf("Shaders are not supported\n");
				gfx_set_render(RENDER_SOFTWARE);
				cfg->render = RENDER_SOFTWARE;
			}
		}
	}

	/*
	 * inizializzo l'ntsc che utilizzero' non solo
	 * come filtro ma anche nel gfx_set_screen() per
	 * generare la paletta dei colori.
	 */
	if (ntsc_init(0, 0, 0, 0, 0) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	/*
	 * mi alloco una zona di memoria dove conservare la
	 * paletta nel formato di visualizzazione.
	 */
	if (!(d3d9.palette = malloc(NUM_COLORS * sizeof(uint32_t)))) {
		fprintf(stderr, "Unable to allocate the palette\n");
		return (EXIT_ERROR);
	}

	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE);

	return (EXIT_OK);
}
void gfx_set_render(BYTE render) {
	switch (render) {
		case RENDER_SOFTWARE:
			gfx.hlsl.enabled = FALSE;
			gfx.hlsl.used = FALSE;
			break;
		case RENDER_HLSL:
			gfx.hlsl.enabled = TRUE;
			gfx.hlsl.used = FALSE;
			break;
	}
}
void gfx_set_screen(BYTE scale, BYTE filter, BYTE fullscreen, BYTE palette, BYTE force_scale) {
	BYTE set_mode;
	WORD width, height;
	//WORD w_for_pr, h_for_pr;

	gfx_set_screen_start:
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
			case NO_FILTER:
			case POSPHOR:
			case SCANLINE:
			case DBL:
			case CRT_CURVE:
			case CRT_NO_CURVE:
				gfx.filter = scale_surface;
				d3d9.interpolation = FALSE;
				/*
				 * se sto passando dal filtro ntsc ad un'altro, devo
				 * ricalcolare la larghezza del video mode quindi
				 * forzo il controllo del fattore di scala.
				 */
				if (cfg->filter == NTSC_FILTER) {
					/* devo reimpostare la larghezza del video mode */
					scale = cfg->scale;
				}
				break;
			case BILINEAR:
				gfx.filter = scale_surface;
				d3d9.interpolation = TRUE;
				break;
			case SCALE2X:
			case SCALE3X:
			case SCALE4X:
				gfx.filter = scaleNx;
				d3d9.interpolation = FALSE;
				break;
			case HQ2X:
			case HQ3X:
			case HQ4X:
				gfx.filter = hqNx;
				d3d9.interpolation = FALSE;
				break;
			case NTSC_FILTER:
				gfx.filter = ntsc_surface;
				d3d9.interpolation = FALSE;
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
				break;
		}
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
				/* il fattore di scala a 1 e' possibile solo senza filtro */
				if (filter != NO_FILTER) {
					/*
					 * con un fattore di scala X1 effect deve essere
					 * sempre impostato su scale_surface.
					 */
					gfx.filter = scale_surface;
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

	switch (cfg->filter) {
		case NO_FILTER:
		case BILINEAR:
			d3d9.scale_force = TRUE;
			d3d9.scale = X1;
			d3d9.factor = cfg->scale;
			break;
		default:
			d3d9.scale_force = FALSE;
			d3d9.scale = cfg->scale;
			d3d9.factor = X1;
			break;
	}
	d3d9.shader.id = SHADER_NONE;
	gfx.hlsl.used = FALSE;

	if ((gfx.hlsl.compliant == TRUE) && (gfx.hlsl.enabled == TRUE)) {

#define hlsl_up(e, s)\
	d3d9.shader.id = s;\
	d3d9.scale_force = TRUE;\
	d3d9.scale = X1;\
	d3d9.factor = cfg->scale;\
	gfx.hlsl.used = TRUE;\
	gfx.filter = e

		switch (cfg->filter) {
			case NO_FILTER:
				hlsl_up(scale_surface, SHADER_NO_FILTER);
				break;
			case BILINEAR:
				hlsl_up(scale_surface, SHADER_NO_FILTER);
				break;
			case POSPHOR:
				hlsl_up(scale_surface, SHADER_POSPHOR);
				break;
			case SCANLINE:
				hlsl_up(scale_surface, SHADER_SCANLINE);
				break;
			case DBL:
				hlsl_up(scale_surface, SHADER_DONTBLOOM);
				break;
			case CRT_CURVE:
				hlsl_up(scale_surface, SHADER_CRT);
				break;
			case CRT_NO_CURVE:
				hlsl_up(scale_surface, SHADER_CRT4);
				break;
			case SCALE2X:
				hlsl_up(scale_surface, SHADER_SCALE2X);
				break;
			case SCALE3X:
				hlsl_up(scale_surface, SHADER_SCALE3X);
				break;
			case SCALE4X:
				hlsl_up(scale_surface, SHADER_SCALE4X);
				break;
			case HQ2X:
				hlsl_up(scale_surface, SHADER_HQ2X);
				break;
			case HQ4X:
				hlsl_up(hqNx, SHADER_HQ2X);
				d3d9.scale = X2;
				d3d9.factor = X2;
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

		ShowWindow(gui_main_window_id(), SW_HIDE);

		/* faccio quello che serve prima del setvideo */
		gui_set_video_mode();

		if (d3d9_create_context(gfx.w[VIDEO_MODE], gfx.h[VIDEO_MODE]) == EXIT_ERROR) {
			fprintf(stderr, "Unable to initialize d3d context\n");
			ShowWindow(gui_main_window_id(), SW_NORMAL);
			return;
		}

		ShowWindow(gui_main_window_id(), SW_NORMAL);
	}

	/* questo controllo devo farlo necessariamente dopo il glew_init() */
	if ((gfx.hlsl.compliant == FALSE) || (gfx.hlsl.enabled == FALSE)) {
		if ((filter >= POSPHOR) && (filter <= CRT_NO_CURVE)) {
			filter = NO_FILTER;
			goto gfx_set_screen_start;
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
	/* filtro e aggiornamento texture */
	{
		D3DLOCKED_RECT locked_rect;

		/* lock della surface in memoria */
		IDirect3DSurface9_LockRect(d3d9.texture.surface.data, &locked_rect, NULL, D3DLOCK_DISCARD);

		/* applico l'effetto */
		gfx.filter(screen.data,
				screen.line,
				d3d9.palette,
				gfx.bit_per_pixel,
				locked_rect.Pitch,
				locked_rect.pBits,
				gfx.rows,
				gfx.lines,
				d3d9.texture.surface.w,
				d3d9.texture.surface.h,
				d3d9.scale);

		/* unlock della surface in memoria */
		IDirect3DSurface9_UnlockRect(d3d9.texture.surface.data);

		/* aggiorno la texture */
		IDirect3DDevice9_UpdateSurface(d3d9.dev, d3d9.texture.surface.data, NULL,
				d3d9.texture.map0, NULL);
	}

	IDirect3DDevice9_Clear(d3d9.dev, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 0, 0, 0),
			1.0f, 0);

	/* inizio */
	IDirect3DDevice9_BeginScene(d3d9.dev);

	if (gfx.hlsl.used == TRUE) {
		/* comunico con il vertex shader */
		D3DXMATRIX world_view_projection;

		//D3DXMATRIX matrix_world, matrix_view, matrix_proj;
		//IDirect3DDevice9_GetTransform(d3d9.dev, D3DTS_WORLD, &matrix_world);
		//IDirect3DDevice9_GetTransform(d3d9.dev, D3DTS_VIEW, &matrix_view);
		//IDirect3DDevice9_GetTransform(d3d9.dev, D3DTS_PROJECTION, &matrix_proj);

		D3DXMatrixMultiply(&world_view_projection, &d3d9.world, &d3d9.view);
		D3DXMatrixMultiply(&world_view_projection, &world_view_projection, &d3d9.projection);

		ID3DXConstantTable_SetMatrix(d3d9.shader.table_vrt, d3d9.dev, "m_world_view_projection",
				&world_view_projection);

		/* faccio il resto */
		IDirect3DDevice9_SetFVF(d3d9.dev, FVF);
		IDirect3DDevice9_SetStreamSource(d3d9.dev, 0, d3d9.quad, 0, sizeof(vertex));
		if (d3d9.shader.vrt) { IDirect3DDevice9_SetVertexShader(d3d9.dev, d3d9.shader.vrt); }
		if (d3d9.shader.pxl) { IDirect3DDevice9_SetPixelShader(d3d9.dev, d3d9.shader.pxl); }
		IDirect3DDevice9_DrawPrimitive(d3d9.dev, D3DPT_TRIANGLEFAN, 0, 2);
	} else {
		IDirect3DDevice9_SetFVF(d3d9.dev, FVF);
		IDirect3DDevice9_SetStreamSource(d3d9.dev, 0, d3d9.quad, 0, sizeof(vertex));
		IDirect3DDevice9_DrawPrimitive(d3d9.dev, D3DPT_TRIANGLEFAN, 0, 2);
	}

	IDirect3DDevice9_EndScene(d3d9.dev);

	//double start = gui_get_ms();

	IDirect3DDevice9_Present(d3d9.dev, NULL, NULL, NULL, NULL);

	//double stop = gui_get_ms();
	//printf("ms draw = %f\r", stop - start);
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

	d3d9_release_context();

	if (d3d9.d3d) {
		IDirect3D9_Release(d3d9.d3d);
		d3d9.d3d = NULL;
	}
}


BYTE d3d9_create_context(UINT width, UINT height) {
	d3d9_release_context();

	if (d3d9_create_device(width, height) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	if (IDirect3DDevice9_CreateVertexBuffer(d3d9.dev,
			4 * sizeof(vertex),
			D3DUSAGE_WRITEONLY,
			FVF,
			D3DPOOL_DEFAULT,
			&d3d9.quad,
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
			IDirect3DDevice9_SetSamplerState(d3d9.dev, 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice9_SetSamplerState(d3d9.dev, 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice9_SetSamplerState(d3d9.dev, 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
		} else {
			IDirect3DDevice9_SetSamplerState(d3d9.dev, 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			IDirect3DDevice9_SetSamplerState(d3d9.dev, 0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			IDirect3DDevice9_SetSamplerState(d3d9.dev, 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
		}

		// set the fixed render state
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_ZENABLE, D3DZB_FALSE);
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_FILLMODE, D3DFILL_SOLID);
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_SHADEMODE, D3DSHADE_FLAT);
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_ZWRITEENABLE, FALSE);
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_ALPHATESTENABLE, TRUE);
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_LASTPIXEL, TRUE);
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_CULLMODE, D3DCULL_NONE);
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_ZFUNC, D3DCMP_LESS);
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_ALPHAREF, 0);
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_ALPHAFUNC, D3DCMP_GREATER);
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_DITHERENABLE, FALSE);
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_FOGENABLE, FALSE);
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_SPECULARENABLE, FALSE);
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_STENCILENABLE, FALSE);
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_WRAP0, FALSE);
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_CLIPPING, TRUE);
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_LIGHTING, FALSE);
		IDirect3DDevice9_SetRenderState(d3d9.dev, D3DRS_COLORVERTEX, TRUE);

		IDirect3DDevice9_SetTextureStageState(d3d9.dev, 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		IDirect3DDevice9_SetTextureStageState(d3d9.dev, 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

		d3d9.texcoords.l = 0.0f;
		d3d9.texcoords.r = (FLOAT) width / (d3d9.texture.w * d3d9.factor);
		d3d9.texcoords.t = 0.0f;
		d3d9.texcoords.b = (FLOAT) height / (d3d9.texture.h * d3d9.factor);

		{
			/* aspect ratio */
			FLOAT w_quad = (FLOAT) gfx.w[VIDEO_MODE];
			FLOAT h_quad = (FLOAT) gfx.h[VIDEO_MODE];
			d3d9.quadcoords.l = 0.0f;
			d3d9.quadcoords.r = w_quad;
			d3d9.quadcoords.t = 0.0f;
			d3d9.quadcoords.b = h_quad;

			int flags = FALSE;

			/* con flags intendo sia il fullscreen che il futuro resize */
			if (flags && cfg->aspect_ratio) {
				FLOAT ratio_surface = w_quad / h_quad;
				FLOAT ratio_frame = (FLOAT) width / (FLOAT) height;

				//ratio_frame = (float) 4 / 3;
				//ratio_frame = (float) 16 / 9;

				//fprintf(stderr, "opengl : %f %f\n", ratio_surface, ratio_frame);

				/*
				 * se l'aspect ratio del frame e' maggiore di
				 * quello della superficie allora devo agire
				 * sull'altezza.
				 */
				if (ratio_frame > ratio_surface) {
					FLOAT centering_factor = 0.0f;

					h_quad = w_quad / ratio_frame;
					centering_factor = ((FLOAT) gfx.h[VIDEO_MODE] - h_quad) / 2.0f;

					d3d9.quadcoords.l = 0.0f;
					d3d9.quadcoords.r = w_quad;
					d3d9.quadcoords.t = centering_factor;
					d3d9.quadcoords.b = h_quad + centering_factor;
					/*
					 * se l'aspect ratio del frame e' minore di
					 * quello della superficie allora devo agire
					 * sulla larghezza.
					 */
				} else if (ratio_frame < ratio_surface) {
					FLOAT centering_factor = 0.0f;

					w_quad = ratio_frame * h_quad;
					centering_factor = ((FLOAT) gfx.w[VIDEO_MODE] - w_quad) / 2.0f;

					d3d9.quadcoords.l = centering_factor;
					d3d9.quadcoords.r = w_quad + centering_factor;
					d3d9.quadcoords.t = 0.0f;
					d3d9.quadcoords.b = h_quad;
				}
			}
		}

		{
			_texcoords *tc = &d3d9.texcoords;
			void *tv_vertices;
			vertex quad_vertices[] = {
				{ d3d9.quadcoords.l, d3d9.quadcoords.b, 0.0f, 1.0f, tc->l, tc->b },
				{ d3d9.quadcoords.l, d3d9.quadcoords.t, 0.0f, 1.0f, tc->l, tc->t },
				{ d3d9.quadcoords.r, d3d9.quadcoords.t, 0.0f, 1.0f, tc->r, tc->t },
				{ d3d9.quadcoords.r, d3d9.quadcoords.b, 0.0f, 1.0f, tc->r, tc->b }
			};

			/*
			 * problema dell'infamous half-texel offset of D3D9 (corretto dalle D3D10 in poi) :
			 * http://msdn.microsoft.com/en-us/library/bb219690%28VS.85%29.aspx
			 */
			{
				int i;

				for (i=0; i < LENGTH(quad_vertices); i++) {
					quad_vertices[i].x -= 0.5f;
					quad_vertices[i].y -= 0.5f;
				}
			}

			IDirect3DVertexBuffer9_Lock(d3d9.quad, 0, 0, (void**) &tv_vertices, 0);
			memcpy(tv_vertices, quad_vertices, sizeof(quad_vertices));
			IDirect3DVertexBuffer9_Unlock(d3d9.quad);
		}
	}

	{
		D3DXVECTOR3 position = { 0.0f, 0.0f, -1.0f };
		D3DXVECTOR3 target = { 0.0f, 0.0f, 0.0f };
		D3DXVECTOR3 up = { 0.0f, 1.0f, 0.0f };

		D3DXMatrixRotationY(&d3d9.world, 0.0f);
		IDirect3DDevice9_SetTransform(d3d9.dev, D3DTS_WORLD, &d3d9.world);

		D3DXMatrixLookAtLH(&d3d9.view, &position, &target, &up);
		IDirect3DDevice9_SetTransform(d3d9.dev, D3DTS_VIEW, &d3d9.view);

		D3DXMatrixPerspectiveFovLH(&d3d9.projection, D3DXToRadian(90.0f), 1.0f, 0.0f, 1.0f);
		IDirect3DDevice9_SetTransform(d3d9.dev, D3DTS_PROJECTION, &d3d9.projection);
	}

	if (gfx.hlsl.enabled == TRUE) {
		d3d9_create_shader(&d3d9.shader);
	}

	return (EXIT_OK);
}
void d3d9_release_context(void) {
	d3d9_release_shader(&d3d9.shader);

	d3d9_release_texture(&d3d9.texture);

	if (d3d9.quad) {
		IDirect3DVertexBuffer9_Release(d3d9.quad);
		d3d9.quad = NULL;
	}

	if (d3d9.dev) {
		IDirect3DDevice9_Release(d3d9.dev);
		d3d9.dev = NULL;
	}
}
BYTE d3d9_create_device(UINT width, UINT height) {
	D3DPRESENT_PARAMETERS d3dpp[d3d9.number_of_adapters];

	if (d3d9.dev) {
		IDirect3DDevice9_Release(d3d9.dev);
		d3d9.dev = NULL;
	}

	ZeroMemory(&d3dpp, d3d9.number_of_adapters * sizeof(D3DPRESENT_PARAMETERS));

	{
		int i;

		for (i = 0; i < d3d9.number_of_adapters; i++) {
			d3dpp[i].Windowed = TRUE;
			d3dpp[i].SwapEffect = D3DSWAPEFFECT_DISCARD;
			d3dpp[i].hDeviceWindow = gui_emu_frame_id();
			d3dpp[i].BackBufferCount = 1;
			d3dpp[i].BackBufferFormat = d3d9.display_mode.Format;
			d3dpp[i].BackBufferWidth = width;
			d3dpp[i].BackBufferHeight = height;
			//d3dpp[i].EnableAutoDepthStencil = TRUE;
			//d3dpp[i].AutoDepthStencilFormat = D3DFMT_D16;
			d3dpp[i].MultiSampleQuality = D3DMULTISAMPLE_NONE;
			d3dpp[i].PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		}
	}

	if (IDirect3D9_CreateDevice(d3d9.d3d,
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			gui_emu_frame_id(),
			d3d9.flags,
			d3dpp,
			&d3d9.dev) != D3D_OK) {
		fprintf(stderr, "Unable to create d3d device\n");
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
BYTE d3d9_create_texture(_texture *texture, uint32_t width, uint32_t height, uint8_t interpolation,
        uint8_t pow) {
	DWORD usage = 0;

	d3d9_release_texture(texture);

	if (pow) {
		texture->w = d3d9_power_of_two(width);
		texture->h = d3d9_power_of_two(height);
	} else {
		texture->w = width;
		texture->h = height;
	}

    /* se la scheda video supporta solo texture quadre allore devo crerle cosi' */
	if (d3d9.texture_square_only == TRUE) {
		if (texture->w < texture->h) {
			texture->w = texture->h;
		} else {
			texture->h = texture->w;
		}
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
		fprintf(stderr, "Unable to create the texture\n");
		return (EXIT_ERROR);
	}

	IDirect3DTexture9_GetSurfaceLevel(texture->data, 0, &texture->map0);

	texture->surface.w = width;
	texture->surface.h = height;

	/* creo la superficie temporanea le cui dimensioni non devono essere "POWerate" */
	if (IDirect3DDevice9_CreateOffscreenPlainSurface(d3d9.dev,
			texture->surface.w,
			texture->surface.h,
			d3d9.display_mode.Format,
			D3DPOOL_SYSTEMMEM,
			&texture->surface.data,
			NULL) != D3D_OK) {
		fprintf(stderr, "Unable to create the memory surface\n");
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
void d3d9_release_texture(_texture *texture) {
	if (texture->data) {
		IDirect3DTexture9_Release(texture->data);
		texture->data = NULL;
	}

	texture->map0 = NULL;

	if (texture->surface.data) {
		IDirect3DSurface9_Release(texture->surface.data);
		texture->surface.data = NULL;
	}
}
BYTE d3d9_create_shader(_shader *shd) {
	DWORD flags = 0;

	d3d9_release_shader(shd);

	if (shd->id == SHADER_NONE) {
		return (EXIT_OK);
	}

	shd->code = &shader_code[shd->id];

	/* vertex shader */
	if (shd->code->vertex != NULL) {
		LPD3DXBUFFER code = NULL, buffer_errors = NULL;
		HRESULT hr;

		hr = D3DXCompileShader(shd->code->vertex,
				strlen(shd->code->vertex),
				NULL,
				NULL,
				"Vs",
				D3DXGetVertexShaderProfile(d3d9.dev),
				flags,
				&code,
				&buffer_errors,
				&shd->table_vrt);

		switch (hr) {
			case D3D_OK:
				/* creo il vertex shader */
				IDirect3DDevice9_CreateVertexShader(d3d9.dev,
						(DWORD *) ID3DXBuffer_GetBufferPointer(code),
						&shd->vrt);
				ID3DXBuffer_Release(code);
				break;
			case COMPILERSHADER_NOT_FOUND:
				fprintf(stderr, "ATTENTION: DirectX HLSL compiler installation are incomplete or "
						"corrupted.\n           Please reinstall DirectX 9C\n");
				d3d9_release_shader(shd);
				return (EXIT_ERROR);
			default:
				fprintf(stderr, "Vertex shader error : 0x%lx\n", hr);
				if (buffer_errors) {
					LPVOID errors = ID3DXBuffer_GetBufferPointer(buffer_errors);

					fprintf(stderr, "Vertex shader compile error : %s\n", (const char *) errors);
					ID3DXBuffer_Release(buffer_errors);
				}
				d3d9_release_shader(shd);
				break;
		}
	}

	/* pixel shader */
	if (shd->code->pixel != NULL) {
		LPD3DXBUFFER code = NULL, buffer_errors = NULL;
		HRESULT hr;

		hr = D3DXCompileShader(shd->code->pixel,
				strlen(shd->code->pixel),
				NULL,
				NULL,
				"Ps",
				D3DXGetPixelShaderProfile(d3d9.dev),
				flags,
				&code,
				&buffer_errors,
				&shd->table_pxl);
		switch (hr) {
			case D3D_OK: {
				FLOAT sse[2], svm[2], st[2], fc;

				/* creo il pixel shader */
				IDirect3DDevice9_CreatePixelShader(d3d9.dev,
						(DWORD *) ID3DXBuffer_GetBufferPointer(code),
						&shd->pxl);
				ID3DXBuffer_Release(code);

				sse[0] = (FLOAT) SCR_ROWS;
				sse[1] = (FLOAT) SCR_LINES;
				svm[0] = d3d9.quadcoords.r - d3d9.quadcoords.l;
				svm[1] = d3d9.quadcoords.b - d3d9.quadcoords.t;
				st[0] = d3d9.texture.w;
				st[1] = d3d9.texture.h;
				fc = (FLOAT) ppu.frames;

				ID3DXConstantTable_SetFloatArray(shd->table_pxl, d3d9.dev, "size_screen_emu",
							(CONST FLOAT * ) &sse, 2);
				ID3DXConstantTable_SetFloatArray(shd->table_pxl, d3d9.dev, "size_video_mode",
							(CONST FLOAT * ) &svm, 2);
				ID3DXConstantTable_SetFloatArray(shd->table_pxl, d3d9.dev, "size_texture",
							(CONST FLOAT * ) &st, 2);
				ID3DXConstantTable_SetFloatArray(shd->table_pxl, d3d9.dev, "frame_counter",
							(CONST FLOAT * ) &fc, 1);

				/*
				printf("\n");
				printf("size_screen_emu : %f - %f\n", sse[0], sse[1]);
				printf("size_video_mode : %f - %f\n", svm[0], svm[1]);
				printf("size_texture    : %f - %f\n", st[0], st[1]);
				printf("\n");
				*/

				break;
			}
			case COMPILERSHADER_NOT_FOUND:
				fprintf(stderr, "ATTENTION: DirectX HLSL compiler installation are incomplete or "
						"corrupted.\n           Please reinstall DirectX 9C\n");
				d3d9_release_shader(shd);
				return (EXIT_ERROR);
			default:
				fprintf(stderr, "Pixel shader error : 0x%lx\n", hr);
				if (buffer_errors) {
					LPVOID errors = ID3DXBuffer_GetBufferPointer(buffer_errors);

					fprintf(stderr, "Pixel shader compile error : %s\n", (const char *) errors);
					ID3DXBuffer_Release(buffer_errors);
				}
				d3d9_release_shader(shd);
				break;
		}
	}

	return (EXIT_OK);
}
void d3d9_release_shader(_shader *shd) {
	if (shd->vrt) {
		IDirect3DDevice9_SetVertexShader(d3d9.dev, NULL);
		IDirect3DVertexShader9_Release(shd->vrt);
		shd->vrt = NULL;
	}
	if (shd->pxl) {
		IDirect3DDevice9_SetPixelShader(d3d9.dev, NULL);
		IDirect3DPixelShader9_Release(shd->pxl);
		shd->pxl = NULL;
	}
}
int d3d9_power_of_two(int base) {
	int pot = 1;

	while (pot < base) {
		pot <<= 1;
	}
	return (pot);
}
