/*
 * shaders.h
 *
 *  Created on: 09/mag/2012
 *      Author: fhorse
 */

#ifndef SHADERS_H_
#define SHADERS_H_

#include "text.h"

enum shader_type {
	SHADER_COLOR,
	SHADER_NO_FILTER,
	SHADER_PHOSPHOR,
	SHADER_SCANLINE,
	SHADER_CRT,
	SHADER_DONTBLOOM,
	SHADER_TOTAL,
	SHADER_NONE = 255
};

typedef struct {
	LPDIRECT3DSURFACE9 data;
	WORD w;
	WORD h;
} _surface;
typedef struct {
	LPDIRECT3DTEXTURE9 data;
	LPDIRECT3DSURFACE9 map0;

	FLOAT w;
	FLOAT h;

	_surface surface;

	LPDIRECT3DVERTEXBUFFER9 quad;
	_texcoords quadcoords;
} _texture;
typedef struct {
	const char *vertex;
	const char *pixel;
} _shader_code;
typedef struct {
	LPDIRECT3DVERTEXSHADER9 vrt;
	LPD3DXCONSTANTTABLE table_vrt;

	LPDIRECT3DPIXELSHADER9 pxl;
	LPD3DXCONSTANTTABLE table_pxl;

	UINT id;
	_shader_code *code;
} _shader;

#endif /* SHADERS_H_ */

#if defined (_SHADERS_CODE_)
static _shader_code shader_code[SHADER_TOTAL] = {
#include "shaders/color.h"
#include "shaders/no_filter.h"
#include "shaders/phosphor.h"
#include "shaders/scanline.h"
#include "shaders/crt.h"
#include "shaders/dbl.h"
};
#undef _SHADERS_CODE_
#endif
