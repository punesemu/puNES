/*
 * gfx.c
 *
 *  Created on: 01/mar/2013
 *      Author: fhorse
 */

#include <d3d9.h>
#include "emu.h"
#include "win.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

LPDIRECT3D9 d3d = NULL;
LPDIRECT3DDEVICE9 d3ddev = NULL;
LPDIRECT3DVERTEXBUFFER9 v_buffer = NULL;
D3DDISPLAYMODE display_mode;

typedef struct {
	FLOAT X, Y, Z, RHW;
	DWORD COLOR;
} CUSTOMVERTEX;
#define CUSTOMFVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

BYTE gfx_init(void) {
	if (gui_create()) {
		fprintf(stderr, "gui initialization failed\n");
		return (EXIT_ERROR);
	}

	{
		D3DPRESENT_PARAMETERS d3dpp;
		D3DCAPS9 d3dcaps;
		DWORD vertex_processing = 0;
		HRESULT rc;

		if ((d3d = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) {
			fprintf(stderr, "impossible to create d3d device\n");
			return (EXIT_ERROR);
		}

		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow = gui_window_id();
		d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		d3dpp.BackBufferWidth = SCREEN_WIDTH;
		d3dpp.BackBufferHeight = SCREEN_HEIGHT;

		IDirect3D9_GetAdapterDisplayMode(d3d, D3DADAPTER_DEFAULT, &display_mode);

		/* Check for hardware T&L */
		IDirect3D9_GetDeviceCaps(d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dcaps);

		if (d3dcaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
			vertex_processing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
			/* Check for pure device */
			if (d3dcaps.DevCaps & D3DDEVCAPS_PUREDEVICE) {
				vertex_processing |= D3DCREATE_PUREDEVICE;
			}
		} else {
			vertex_processing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}

		// create a device class using this information and the info from the d3dpp stuct
		rc = IDirect3D9_CreateDevice(d3d,
				D3DADAPTER_DEFAULT,
				D3DDEVTYPE_HAL,
				gui_window_id(),
				vertex_processing,
				&d3dpp,
				&d3ddev);

		if (rc != D3D_OK) {
			fprintf(stderr, "impossible to create d3d device\n");
			return (EXIT_ERROR);
		}
	}

	{
		// create the vertices using the CUSTOMVERTEX struct
		CUSTOMVERTEX vertices[] = {
			{ 400.0f, 62.5f, 0.5f, 1.0f, D3DCOLOR_XRGB(0, 0, 255), },
			{ 650.0f, 500.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0, 255, 0), },
			{ 150.0f, 500.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(255, 0, 0), },
		};

		VOID* pVoid;    // a void pointer

		// create a vertex buffer interface called v_buffer
		IDirect3DDevice9_CreateVertexBuffer(d3ddev,
				3 * sizeof(CUSTOMVERTEX),
				0,
				CUSTOMFVF,
				D3DPOOL_MANAGED,
				&v_buffer,
				NULL);

		// lock v_buffer and load the vertices into it
		IDirect3DVertexBuffer9_Lock(v_buffer, 0, 0, (void**) &pVoid, 0);
		memcpy(pVoid, vertices, sizeof(vertices));
		IDirect3DVertexBuffer9_Unlock(v_buffer);
	}

	return (EXIT_OK);
}
void gfx_set_render(BYTE render) {
	return;
}
void gfx_set_screen(BYTE scale, BYTE filter, BYTE fullscreen, BYTE palette, BYTE force_scale) {
	return;
}
void gfx_draw_screen(BYTE forced) {
    // clear the window to a deep blue
	IDirect3DDevice9_Clear(d3ddev, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);

	IDirect3DDevice9_BeginScene(d3ddev);    // begins the 3D scene

		// select which vertex format we are using
		IDirect3DDevice9_SetFVF(d3ddev, CUSTOMFVF);

		// select the vertex buffer to display
		IDirect3DDevice9_SetStreamSource(d3ddev, 0, v_buffer, 0, sizeof(CUSTOMVERTEX));

		// copy the vertex buffer to the back buffer
		IDirect3DDevice9_DrawPrimitive(d3ddev, D3DPT_TRIANGLELIST, 0, 1);

	IDirect3DDevice9_EndScene(d3ddev);    // ends the 3D scene

	IDirect3DDevice9_Present(d3ddev, NULL, NULL, NULL, NULL);    // displays the created frame
}
void gfx_reset_video(void) {
	return;
}
void gfx_quit(void) {
	if (v_buffer) {
		IDirect3DVertexBuffer9_Release(v_buffer);
	}
	if (d3ddev) {
		IDirect3DDevice9_Release(d3ddev);
	}
	if (d3d) {
		IDirect3D9_Release(d3d);
	}

    return;
}
