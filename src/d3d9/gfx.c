/*
 * gfx.c
 *
 *  Created on: 01/mar/2013
 *      Author: fhorse
 */

#include <d3d9.h>
#include "win.h"
#include "emu.h"
#include "palette.h"
#include "gfx.h"
#include "cfg_file.h"
#include "ppu.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

enum { NO_POWER_OF_TWO, POWER_OF_TWO };

struct _d3d9 {
	LPDIRECT3D9 d3d;
	LPDIRECT3DDEVICE9 d3ddev;
	LPDIRECT3DVERTEXBUFFER9 v_buffer;
	LPDIRECT3DTEXTURE9 texture;
	D3DDISPLAYMODE display_mode;

	uint32_t *palette;
	GFX_EFFECT_ROUTINE;

	BYTE bpp;
	BOOL auto_gen_mipmap;
	BOOL dynamic_texture;
	WORD texture_create_usage;
	int scale;
} d3d9;

typedef struct {
	FLOAT X, Y, Z, RHW;
	DWORD COLOR;
} CUSTOMVERTEX;
#define CUSTOMFVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

BYTE d3d9_create_texture(LPDIRECT3DTEXTURE9 *texture, uint32_t width, uint32_t height,
        uint8_t interpolation, uint8_t pow);
int d3d9_power_of_two(int base);

BYTE gfx_init(void) {
	if (gui_create()) {
		fprintf(stderr, "gui initialization failed\n");
		return (EXIT_ERROR);
	}

	memset(&d3d9, 0x00, sizeof(d3d9));

	{
		D3DPRESENT_PARAMETERS d3dpp;
		D3DCAPS9 d3dcaps;
		DWORD vertex_processing = 0;
		HRESULT rc;

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

		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow = gui_window_id();
		d3dpp.BackBufferFormat = d3d9.display_mode.Format;
		d3dpp.BackBufferWidth = SCREEN_WIDTH;
		d3dpp.BackBufferHeight = SCREEN_HEIGHT;

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
			d3d9.auto_gen_mipmap = FALSE;
			printf("Video driver don't support automatic generation of mipmap\n");
		}

		if (d3dcaps.Caps2 & D3DCAPS2_DYNAMICTEXTURES) {
			d3d9.dynamic_texture = TRUE;
		} else {
			d3d9.dynamic_texture = FALSE;
			printf("Video driver don't support dynamic texture\n");
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
			vertex_processing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
			/* Check for pure device */
			if (d3dcaps.DevCaps & D3DDEVCAPS_PUREDEVICE) {
				vertex_processing |= D3DCREATE_PUREDEVICE;
			}
		} else {
			vertex_processing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
			printf("Video driver don't support hardware accelaration\n");
		}

		// create a device class using this information and the info from the d3dpp stuct
		rc = IDirect3D9_CreateDevice(d3d9.d3d,
				D3DADAPTER_DEFAULT,
				D3DDEVTYPE_HAL,
				gui_window_id(),
				vertex_processing,
				&d3dpp,
				&d3d9.d3ddev);

		if (rc != D3D_OK) {
			fprintf(stderr, "Unable to create d3d device\n");
			return (EXIT_ERROR);
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

	/* creo la texture principale */
	cfg->scale = d3d9.scale = 1;
	if (d3d9_create_texture(&d3d9.texture, SCR_ROWS * d3d9.scale, SCR_LINES * d3d9.scale, 0,
	        POWER_OF_TWO) == EXIT_ERROR) {
		fprintf(stderr, "Unable to create main texture\n");
		return (EXIT_ERROR);
	}

	{
		// create the vertices using the CUSTOMVERTEX struct
		CUSTOMVERTEX vertices[] = {
			//{ 400.0f, 62.5f, 0.5f, 1.0f, D3DCOLOR_XRGB(0, 0, 255), },
			//{ 650.0f, 500.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0, 255, 0), },
			//{ 150.0f, 500.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(255, 0, 0), },
			{ 150.0f,  62.5f, 0.5f, 1.0f, D3DCOLOR_XRGB(40, 40, 40), },
			{ 650.0f,  62.5f, 0.5f, 1.0f, D3DCOLOR_XRGB(0, 0, 255), },
			{ 650.0f, 500.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0, 255, 0), },
			{ 150.0f, 500.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(255, 0, 0), },

		};

		VOID* pVoid;    // a void pointer

		// create a vertex buffer interface called v_buffer
		IDirect3DDevice9_CreateVertexBuffer(d3d9.d3ddev,
				4 * sizeof(CUSTOMVERTEX),
				0,
				CUSTOMFVF,
				D3DPOOL_DEFAULT,
				&d3d9.v_buffer,
				NULL);

		// lock v_buffer and load the vertices into it
		IDirect3DVertexBuffer9_Lock(d3d9.v_buffer, 0, 0, (void**) &pVoid, 0);
		memcpy(pVoid, vertices, sizeof(vertices));
		IDirect3DVertexBuffer9_Unlock(d3d9.v_buffer);
	}

	return (EXIT_OK);
}
void gfx_set_render(BYTE render) {
	return;
}
void gfx_set_screen(BYTE scale, BYTE filter, BYTE fullscreen, BYTE palette, BYTE force_scale) {

	gfx.rows = SCR_ROWS;
	gfx.lines = SCR_LINES;

	d3d9.effect = scale_surface;

	ntsc_set(cfg->ntsc_format, FALSE, 0, 0, (BYTE *) palette_RGB);

	/* memorizzo i colori della paletta nel formato di visualizzazione */
	{
		WORD i;

		for (i = 0; i < NUM_COLORS; i++) {
			d3d9.palette[i] =
			        D3DCOLOR_ARGB(255, palette_RGB[i].r, palette_RGB[i].g, palette_RGB[i].b);
		}
	}

	return;
}
void gfx_draw_screen(BYTE forced) {
	{
		D3DLOCKED_RECT locked_rect;

		/* lock della texture */
		if (IDirect3DTexture9_LockRect(d3d9.texture, 0, &locked_rect, NULL, D3DLOCK_DISCARD)
		        == D3D_OK) {

			/* applico l'effetto */
			d3d9.effect(screen.data, screen.line, d3d9.palette, d3d9.bpp, locked_rect.Pitch,
			        locked_rect.pBits, gfx.rows, gfx.lines, d3d9.scale);

			/* unlock della texture */
			IDirect3DTexture9_UnlockRect(d3d9.texture, 0);
		} else {
			printf("Unable to lock texture\n");
		}
	}

    // clear the window to a deep blue
	//IDirect3DDevice9_Clear(d3d9.d3ddev, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);
	IDirect3DDevice9_Clear(d3d9.d3ddev, 0, NULL, D3DCLEAR_TARGET, d3d9.palette[40], 1.0f, 0);

	IDirect3DDevice9_BeginScene(d3d9.d3ddev);    // begins the 3D scene

		// select which vertex format we are using
		IDirect3DDevice9_SetFVF(d3d9.d3ddev, CUSTOMFVF);

		// select the vertex buffer to display
		IDirect3DDevice9_SetStreamSource(d3d9.d3ddev, 0, d3d9.v_buffer, 0, sizeof(CUSTOMVERTEX));

		// copy the vertex buffer to the back buffer
		IDirect3DDevice9_DrawPrimitive(d3d9.d3ddev, D3DPT_TRIANGLEFAN, 0, 2);

	IDirect3DDevice9_EndScene(d3d9.d3ddev);    // ends the 3D scene

	IDirect3DDevice9_Present(d3d9.d3ddev, NULL, NULL, NULL, NULL);    // displays the created frame
}
void gfx_reset_video(void) {
	return;
}
void gfx_quit(void) {
	if (d3d9.v_buffer) {
		IDirect3DVertexBuffer9_Release(d3d9.v_buffer);
	}
	if (d3d9.texture) {
		IDirect3DTexture9_Release(d3d9.texture);
	}
	if (d3d9.d3ddev) {
		IDirect3DDevice9_Release(d3d9.d3ddev);
	}
	if (d3d9.d3d) {
		IDirect3D9_Release(d3d9.d3d);
	}

	if (d3d9.palette) {
		free(d3d9.palette);
	}

	ntsc_quit();

    return;
}








BYTE d3d9_create_texture(LPDIRECT3DTEXTURE9 *texture, uint32_t width, uint32_t height,
        uint8_t interpolation, uint8_t pow) {
	int w, h;

	if (pow) {
		w = d3d9_power_of_two(width);
		h = d3d9_power_of_two(height);
	} else {
		w = width;
		h = height;
	}

	if ((*texture)) {
		IDirect3DTexture9_Release((*texture));
	}

	{
		DWORD usage = D3DUSAGE_WRITEONLY;

		if (d3d9.dynamic_texture == TRUE) {
			usage |= D3DUSAGE_DYNAMIC;
		}
		if (d3d9.auto_gen_mipmap == TRUE) {
			usage |= D3DUSAGE_AUTOGENMIPMAP;
		}

		{
			HRESULT hresult = IDirect3DDevice9_CreateTexture(d3d9.d3ddev, w, h, 1, usage,
					d3d9.display_mode.Format, D3DPOOL_DEFAULT, texture, NULL);

			if (hresult == D3DERR_INVALIDCALL) {
				if (IDirect3DDevice9_CreateTexture(d3d9.d3ddev, w, h, 1,
						usage & ~D3DUSAGE_WRITEONLY, d3d9.display_mode.Format, D3DPOOL_DEFAULT,
						texture, NULL ) == D3D_OK) {
					printf("Video driver don't support use of D3DUSAGE_WRITEONLY\n");
				} else {
					return (EXIT_ERROR);
				}
			} else if (hresult != D3D_OK) {
				return (EXIT_ERROR);
			}
		}
	}

	return (EXIT_OK);

	/*glGenTextures(1, &texture->data);
	glBindTexture(GL_TEXTURE_2D, texture->data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (interpolation) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	if (opengl.glew && !GLEW_VERSION_3_1) {
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	}

	{
		SDL_Surface *blank = gfx_create_RGB_surface(opengl.surface_gl, texture->w * 2,
		        texture->h * 2);

		memset(blank->pixels, 0, blank->w * blank->h * blank->format->BytesPerPixel);

		glTexImage2D(GL_TEXTURE_2D, 0, texture->format_internal, texture->w, texture->h, 0,
		        texture->format, texture->type, blank->pixels);

		SDL_FreeSurface(blank);
	}

	if (opengl.glew && GLEW_VERSION_3_1) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glDisable(GL_TEXTURE_2D);
	*/
}

int d3d9_power_of_two(int base) {
	int pot = 1;

	while (pot < base) {
		pot <<= 1;
	}
	return (pot);
}
