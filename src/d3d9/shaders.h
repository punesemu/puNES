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
	SHADER_SCALE2X,
	SHADER_SCALE3X,
	SHADER_SCALE4X,
	SHADER_HQ2X,
	//SHADER_HQ4X,
	//SHADER_4xBR,
	SHADER_PIXELLATE,
	SHADER_PHOSPHOR,
	SHADER_SCANLINE,
	//SHADER_QUILAZ,
	//SHADER_WATERPAINT,
	SHADER_CRT,
	//SHADER_CRT2,
	//SHADER_CRT3,
	SHADER_CRT4,
	//SHADER_BLOOM,
	SHADER_DONTBLOOM,
	SHADER_NTSC,
	//SHADER_NTSC2,
	//SHADER_NTSC3,
	//SHADER_TOON,
	SHADER_DARKSCREEN,
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
#include "shaders/scale2x.h"
#include "shaders/scale3x.h"
#include "shaders/scale4x.h"
#include "shaders/hq2x.h"
#include "shaders/pixellate.h"
#include "shaders/phosphor.h"
#include "shaders/scanline.h"
#include "shaders/crt.h"
#include "shaders/crt4.h"
#include "shaders/dbl.h"
#include "shaders/ntsc.h"
};
#undef _SHADERS_CODE_
#endif
