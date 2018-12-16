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

#ifndef D3D9_H_
#define D3D9_H_

#include "common.h"
#include <d3d9.h>
#include <d3dx9shader.h>
#include <Cg/cg.h>
#include <Cg/cgD3D9.h>
#include "shaders.h"
#include "video/gfx.h"

#define D3D9_ADAPTER(i) (_d3d9_adapter *) ((BYTE *) d3d9.array + (i * sizeof(_d3d9_adapter)))

typedef struct _vertex_buffer {
	// position
	FLOAT x, y, z;
	// texture coord
	FLOAT u, v;
	// lut texture coord
	FLOAT lut_u, lut_v;
	// color
	FLOAT r, g, b, a;
} _vertex_buffer;
typedef struct _lut {
	LPDIRECT3DTEXTURE9 data;

	UINT w, h;
	const unsigned char *bits;
	const char *name;
	D3DTEXTUREFILTERTYPE filter;
} _lut;
typedef struct _shader_uniforms_tex {
	struct _vsut {
		CGparameter video_size;
		CGparameter texture_size;
		CGparameter tex_coord;
	} v;
	struct _fsut {
		CGparameter texture;
		CGparameter video_size;
		CGparameter texture_size;
	} f;
} _shader_uniforms_tex;
typedef struct _shader_uniforms_prog {
	CGparameter video_size;
	CGparameter output_size;
	CGparameter texture_size;

	CGparameter frame_count;
	CGparameter frame_direction;

	CGparameter lut[MAX_PASS];

	CGparameter param[MAX_PARAM];
} _shader_uniforms_prog;
typedef struct _shader_uniforms {
	CGparameter mvp;

	_shader_uniforms_prog v;
	_shader_uniforms_prog f;

	_shader_uniforms_tex orig;
	_shader_uniforms_tex passprev[MAX_PASS];
	_shader_uniforms_tex prev[MAX_PREV];
	_shader_uniforms_tex feedback;
} _shader_uniforms;
typedef struct _shader_info {
	D3DXVECTOR2 video_size;
	D3DXVECTOR2 texture_size;
	D3DXVECTOR2 output_size;
} _shader_info;
typedef struct _shader_prg_cg {
	CGprogram v, f;
} _shader_prg_cg;
typedef struct _shader {
	struct _attribs {
		UINT count;
		UINT attrib[MAX_PASS + MAX_PREV];
	} attribs;

	LPDIRECT3DVERTEXDECLARATION9 vd;
	LPDIRECT3DVERTEXBUFFER9 quad;
	D3DXMATRIX mvp;

	_shader_prg_cg prg;
	_vertex_buffer vb[4];
	_shader_uniforms uni;
	_shader_info info;
} _shader;
typedef struct _texture_rect {
	FLOAT w, h;
	_wh_uint base;
} _texture_rect;
typedef struct _texture_simple {
	LPDIRECT3DTEXTURE9 data;
	LPDIRECT3DSURFACE9 map0;
	LPDIRECT3DSURFACE9 offscreen;

	_texture_rect rect;
	_shader shader;
} _texture_simple;
typedef struct _texture {
	LPDIRECT3DTEXTURE9 data;
	LPDIRECT3DSURFACE9 map0;

	_texture_rect rect;
	_viewport vp;
	_shader shader;
} _texture;
typedef struct _d3d9_adapter {
	UINT id;

	LPDIRECT3DDEVICE9 dev;
	D3DDISPLAYMODE display_mode;

	DWORD flags;

	WORD bit_per_pixel;
	WORD number_of_monitors;

	BOOL hlsl_compliant;
	BOOL dynamic_texture;
	BOOL texture_square_only;
} _d3d9_adapter;
typedef struct _d3d9 {
	LPDIRECT3D9 d3d;

	CGcontext cgctx;

	UINT adapters_on_system;
	UINT adapters_in_use;
	_d3d9_adapter *array, *adapter;

	struct _d3d9_screen {
		UINT in_use;
		UINT index;
		_texture_simple tex[MAX_PREV + 1];
	} screen;

	struct _feedback {
		uint8_t in_use;
		_texture tex;
	} feedback;

	RECT viewp;

	_texture_simple text;
	_texture texture[MAX_PASS + 1];
	_lut lut[MAX_PASS];
} _d3d9;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC _d3d9 d3d9;

EXTERNC BYTE d3d9_init(void);
EXTERNC BYTE d3d9_context_create(void);
EXTERNC void d3d9_draw_scene(void);
EXTERNC void d3d9_quit(void);

#undef EXTERNC

#endif /* D3D9_H_ */
