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
	SHADER_POSPHOR,
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
	/*****************************************************************************************/
	/* COLOR                                                                                 */
	/*****************************************************************************************/
	{
		"struct VsOutput {\n"
		"	float4 Position : POSITION;\n"
		"	float2 TexCoord : TEXCOORD0;\n"
		"};\n"

		"float4x4 m_world_view_projection : WORLDVIEWPROJECTION;\n"

		"VsOutput Vs(float4 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(position, m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"

		"texture texture_scr;\n"
		"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float4 scr = tex2D(s0, texCoord);\n"
		"	scr *= float4(0.9f, 0.8f, 0.4, 1.0);\n"
		"	return scr;\n"
		"}"
	},
	/*****************************************************************************************/
	/* NO_FILTER                                                                             */
	/*****************************************************************************************/
	{
		// vertex shader
		"struct VsOutput {\n"
		"	float4 Position : POSITION;\n"
		"	float2 TexCoord : TEXCOORD0;\n"
		"};\n"

		"float4x4 m_world_view_projection : WORLDVIEWPROJECTION;\n"

		"VsOutput Vs(float4 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(position, m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"

		"texture texture_scr;\n"
		"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float4 scr = tex2D(s0, texCoord);\n"
		"	return scr;\n"
		"}"
	},
	/*****************************************************************************************/
	/* SCALE2X                                                                               */
	/*****************************************************************************************/
	{
		// vertex shader
		"struct VsOutput {\n"
		"	float4 Position : POSITION;\n"
		"	float2 TexCoord : TEXCOORD0;\n"
		"};\n"

		"float4x4 m_world_view_projection : WORLDVIEWPROJECTION;\n"

		"VsOutput Vs(float4 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(position, m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"

		"texture texture_scr;\n"
		"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

		"float2 fract(float2 x) {\n"
		"	return x - floor(x);\n"
		"}\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float x = (1.0 / size_texture.x);\n"
		"	float y = (1.0 / size_texture.y);\n"
		"	float2 dx = float2(x, 0.0);\n"
		"	float2 dy = float2(0.0, y);\n"

		"	float2 v_texCoord[5];\n"

		"	v_texCoord[0] = texCoord;           // center\n"
		"	v_texCoord[1] = v_texCoord[0] - dx; // left\n"
		"	v_texCoord[2] = v_texCoord[0] + dx; // right\n"
		"	v_texCoord[3] = v_texCoord[0] - dy; // top\n"
		"	v_texCoord[4] = v_texCoord[0] + dy; // bottom\n"

		"	float3 E = tex2D(s0, v_texCoord[0]).rgb;\n"
		"	float3 D = tex2D(s0, v_texCoord[1]).rgb;\n"
		"	float3 F = tex2D(s0, v_texCoord[2]).rgb;\n"
		"	float3 H = tex2D(s0, v_texCoord[3]).rgb;\n"
		"	float3 B = tex2D(s0, v_texCoord[4]).rgb;\n"

		"	float4 scr;\n"

		"	if (any((D - F) * (H - B))) {\n"
		"		float2 p = fract(v_texCoord[0] * size_texture);\n"
		"		float3 tmp1 = p.x < 0.5 ? D : F;\n"
		"		float3 tmp2 = p.y < 0.5 ? H : B;\n"
		"		scr = any(tmp1 - tmp2) ? float4(E, 1.0) : float4(tmp1, 1.0);\n"
		"	} else {"
		"		scr = float4(E, 1.0);\n"
		"	}\n"

		"	return scr;\n"
		"}"
	},
	/*****************************************************************************************/
	/* SCALE3X                                                                               */
	/*****************************************************************************************/
	{
		// vertex shader
		"struct VsOutput {\n"
		"	float4 Position : POSITION;\n"
		"	float2 TexCoord : TEXCOORD0;\n"
		"};\n"

		"float4x4 m_world_view_projection : WORLDVIEWPROJECTION;\n"

		"VsOutput Vs(float4 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(position, m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"

		"texture texture_scr;\n"
		"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

		"float2 fract(float2 x) {\n"
		"	return x - floor(x);\n"
		"}\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float2 dx = float2((1.0 / size_texture.x), 1.0);\n"
		"	float2 dy = float2(0.0, (1.0 / size_texture.y));\n"

		"	float4 v_texCoord[5];\n"

		"	v_texCoord[0]    = float4(texCoord, 0.0, 0.0); // E\n"
		"	v_texCoord[0].zw = v_texCoord[0].xy - dx - dy; // A\n"
		"	v_texCoord[1].xy = v_texCoord[0].xy - dy;      // B\n"
		"	v_texCoord[1].zw = v_texCoord[0].xy + dx - dy; // C\n"
		"	v_texCoord[2].xy = v_texCoord[0].xy - dx;      // D\n"
		"	v_texCoord[2].zw = v_texCoord[0].xy + dx;      // F\n"
		"	v_texCoord[3].xy = v_texCoord[0].xy - dx + dy; // G\n"
		"	v_texCoord[3].zw = v_texCoord[0].xy + dy;      // H\n"
		"	v_texCoord[4].xy = v_texCoord[0].xy + dx + dy; // I\n"

		"	// sufficient precision for HDTV (1920x1080)\n"
		"	const float2 sep = float2(0.33333, 0.66667);\n"

		"	float4 E = tex2D(s0, v_texCoord[0].xy); // E\n"
		"	float4 A = tex2D(s0, v_texCoord[0].zw); // A\n"
		"	float4 B = tex2D(s0, v_texCoord[1].xy); // B\n"
		"	float4 C = tex2D(s0, v_texCoord[1].zw); // C\n"
		"	float4 D = tex2D(s0, v_texCoord[2].xy); // D\n"
		"	float4 F = tex2D(s0, v_texCoord[2].zw); // F\n"
		"	float4 G = tex2D(s0, v_texCoord[3].xy); // G\n"
		"	float4 H = tex2D(s0, v_texCoord[3].zw); // H\n"
		"	float4 I = tex2D(s0, v_texCoord[4].xy); // I\n"
		"	// to be sure that ((E != A) == true) in function call\n"
		"	float4 X = float4(1.0, 1.0, 1.0, 1.0) - E;\n"
		"	float4 T;\n"

		"	float2 sel = fract(v_texCoord[0].xy * size_texture);// where are we (E0-E8)?\n"
		"	// branching is very undesirable, so we make a lot of reassignments\n"
		"	// of original pixels to make sure that rule for E1 pixel will work\n"
		"	// with any other (rotate second matrix and swap some Ex)\n"

		"	// native function call --> x y --> equivalent transpose (to minimize reassignments)\n"
		"	//(E, B, X, D, E, F, H) --> 0 0 --> (E, B, X, D, E, F, H) for E0\n"
		"	//(A, B, C, D, E, F, H) --> 1 0 --> (A, B, C, D, E, F, H) for E1\n"
		"	//(E, F, X, B, E, H, D) --> 2 0 --> (X, F, E, D, E, B, H) for E2\n"
		"	//(G, D, A, H, E, B, F) --> 0 1 --> (A, D, G, B, E, H, F) for E3\n"
		"	//( , E,  ,  , E,  ,  ) --> 1 1 --> ( , E,  ,  , E,  ,  ) for E4\n"
		"	//(C, F, I, B, E, H, D) --> 2 1 --> (I, F, C, H, E, B, D) for E5\n"
		"	//(E, D, X, H, E, B, F) --> 0 2 --> (E, D, X, H, E, F, B) for E6\n"
		"	//(I, H, G, F, E, D, B) --> 1 2 --> (G, H, I, D, E, F, B) for E7\n"
		"	//(E, H, X, F, E, D, B) --> 2 2 --> (X, H, E, D, E, F, B) for E8\n"
		"	if (sel.y < sep.x) {\n"
		"		if (sel.x < sep.x) {\n"
		"			A = E;\n"
		"			C = X;\n"
		"		} else if (sel.x >= sep.y) {\n"
		"			A = X;\n"
		"			C = E;\n"
		"			T = B;\n"
		"			B = F;\n"
		"			F = T;\n"
		"		}\n"
		"	} else if (sel.y < sep.y) {\n"
		"		T = B;\n"
		"		if (sel.x < sep.x) {\n"
		"			B = D;\n"
		"			D = T;\n"
		"			C = G;\n"
		"			T = F;\n"
		"			F = H;\n"
		"		} else if (sel.x < sep.y) {\n"
		"			B = E;\n"
		"		} else {\n"
		"			A = I;\n"
		"			B = F;\n"
		"			F = T;\n"
		"			T = D;\n"
		"			D = H;\n"
		"		}\n"
		"		H = T;\n"
		"	} else {\n"
		"		T = B;\n"
		"		if (sel.x < sep.x) {\n"
		"			A = E;\n"
		"			C = X;\n"
		"			B = D;\n"
		"			D = H;\n"
		"		} else {\n"
		"			if (sel.x < sep.y) {\n"
		"				A = G;\n"
		"				C = I;\n"
		"			} else {\n"
		"				A = X;\n"
		"				C = E;\n"
		"			}\n"
		"			B = H;\n"
		"		}\n"
		"		H = T;\n"
		"	}\n"

		"	float4 scr = ((D == B && B != F && D != H && E != C) ||"
		"			(B == F && B != D && F != H && E != A)) ?"
		"			B : E; // Scale3x rule\n"

		"	return scr;\n"
		"}",
	},
	/*****************************************************************************************/
	/* SCALE4X                                                                               */
	/*****************************************************************************************/
	{
		// vertex shader
		"struct VsOutput {\n"
		"	float4 Position : POSITION;\n"
		"	float2 TexCoord : TEXCOORD0;\n"
		"};\n"

		"float4x4 m_world_view_projection : WORLDVIEWPROJECTION;\n"

		"VsOutput Vs(float4 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(position, m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"

		"texture texture_scr;\n"
		"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

		"float2 fract(float2 x) {\n"
		"	return x - floor(x);\n"
		"}\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float x = 0.5 * (1.0 / size_texture.x);\n"
		"	float y = 0.5 * (1.0 / size_texture.y);\n"

		"	float2 dg1 = float2( x, y);\n"
		"	float2 dg2 = float2(-x, y);\n"
		"	float2 sd1 = dg1 * 0.5;\n"
		"	float2 sd2 = dg2 * 0.5;\n"

		"	float4 v_texCoord[7];\n"

		"	v_texCoord[0] = float4(texCoord, 0.0, 0.0);\n"
		"	v_texCoord[1].xy = v_texCoord[0].xy - sd1;\n"
		"	v_texCoord[2].xy = v_texCoord[0].xy - sd2;\n"
		"	v_texCoord[3].xy = v_texCoord[0].xy + sd1;\n"
		"	v_texCoord[4].xy = v_texCoord[0].xy + sd2;\n"
		"	v_texCoord[5].xy = v_texCoord[0].xy - dg1;\n"
		"	v_texCoord[6].xy = v_texCoord[0].xy + dg1;\n"
		"	v_texCoord[5].zw = v_texCoord[0].xy - dg2;\n"
		"	v_texCoord[6].zw = v_texCoord[0].xy + dg2;\n"

		"	float3 c  = tex2D(s0, v_texCoord[0].xy).rgb;\n"
		"	float3 i1 = tex2D(s0, v_texCoord[1].xy).rgb;\n"
		"	float3 i2 = tex2D(s0, v_texCoord[2].xy).rgb;\n"
		"	float3 i3 = tex2D(s0, v_texCoord[3].xy).rgb;\n"
		"	float3 i4 = tex2D(s0, v_texCoord[4].xy).rgb;\n"
		"	float3 o1 = tex2D(s0, v_texCoord[5].xy).rgb;\n"
		"	float3 o3 = tex2D(s0, v_texCoord[6].xy).rgb;\n"
		"	float3 o2 = tex2D(s0, v_texCoord[5].zw).rgb;\n"
		"	float3 o4 = tex2D(s0, v_texCoord[6].zw).rgb;\n"

		"	float3 dt = float3(1.0, 1.0, 1.0);\n"

		"	float ko1 = dot(abs(o1 - c), dt);\n"
		"	float ko2 = dot(abs(o2 - c), dt);\n"
		"	float ko3 = dot(abs(o3 - c), dt);\n"
		"	float ko4 = dot(abs(o4 - c), dt);\n"

		"	float k1 = min(dot(abs(i1 - i3), dt), dot(abs(o1 - o3), dt));\n"
		"	float k2 = min(dot(abs(i2 - i4), dt), dot(abs(o2 - o4), dt));\n"

		"	float w1 = k2; if (ko3 < ko1) { w1 = 0.0; };\n"
		"	float w2 = k1; if (ko4 < ko2) { w2 = 0.0; };\n"
		"	float w3 = k2; if (ko1 < ko3) { w3 = 0.0; };\n"
		"	float w4 = k1; if (ko2 < ko4) { w4 = 0.0; };\n"

		"	float4 scr = float4(((w1 * o1) + (w2 * o2) + (w3 * o3) + (w4 * o4) + (0.0001 * c)) /"
		"			(w1 + w2 + w3 + w4 + 0.0001), 1.0);\n"

		"	return scr;\n"
		"}"
	},
	/*****************************************************************************************/
	/* HQ2X                                                                                  */
	/*****************************************************************************************/
	{
		"struct VsOutput {\n"
		"	float4 Position : POSITION;\n"
		"	float2 TexCoord : TEXCOORD0;\n"
		"};\n"

		"float4x4 m_world_view_projection : WORLDVIEWPROJECTION;\n"

		"VsOutput Vs(float4 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(position, m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"

		"texture texture_scr;\n"
		"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

		"static const float mx = 0.325;     // start smoothing wt.\n"
		"static const float k = -0.250;     // wt. decrease factor\n"
		"static const float max_w = 0.25;   // max filter weigth\n"
		"static const float min_w =-0.05;   // min filter weigth\n"
		"static const float lum_add = 0.25; // effects smoothing\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float x = 0.5 * (1.0 / size_texture.x);\n"
		"	float y = 0.5 * (1.0 / size_texture.y);\n"

		"	float2 dx = float2(x, 0.0);\n"
		"	float2 dy = float2(0.0, y);\n"
		"	float2 dg1 = float2( x, y);\n"
		"	float2 dg2 = float2(-x, y);\n"

		"	float4 v_texCoord[5];\n"

		"	v_texCoord[0] = float4(texCoord.xy, 0.0, 0.0);\n"
		"	v_texCoord[1].xy = v_texCoord[0].xy - dg1;\n"
		"	v_texCoord[1].zw = v_texCoord[0].xy - dy;\n"
		"	v_texCoord[2].xy = v_texCoord[0].xy - dg2;\n"
		"	v_texCoord[2].zw = v_texCoord[0].xy + dx;\n"
		"	v_texCoord[3].xy = v_texCoord[0].xy + dg1;\n"
		"	v_texCoord[3].zw = v_texCoord[0].xy + dy;\n"
		"	v_texCoord[4].xy = v_texCoord[0].xy + dg2;\n"
		"	v_texCoord[4].zw = v_texCoord[0].xy - dx;\n"

		"	float3 c00 = tex2D(s0, v_texCoord[1].xy).xyz;\n"
		"	float3 c10 = tex2D(s0, v_texCoord[1].zw).xyz;\n"
		"	float3 c20 = tex2D(s0, v_texCoord[2].xy).xyz;\n"
		"	float3 c01 = tex2D(s0, v_texCoord[4].zw).xyz;\n"
		"	float3 c11 = tex2D(s0, v_texCoord[0].xy).xyz;\n"
		"	float3 c21 = tex2D(s0, v_texCoord[2].zw).xyz;\n"
		"	float3 c02 = tex2D(s0, v_texCoord[4].xy).xyz;\n"
		"	float3 c12 = tex2D(s0, v_texCoord[3].zw).xyz;\n"
		"	float3 c22 = tex2D(s0, v_texCoord[3].xy).xyz;\n"

		"	float3 dt = float3(1.0, 1.0, 1.0);\n"

		"	float md1 = dot(abs(c00 - c22), dt);\n"
		"	float md2 = dot(abs(c02 - c20), dt);\n"

		"	float w1 = dot(abs(c22 - c11), dt) * md2;\n"
		"	float w2 = dot(abs(c02 - c11), dt) * md1;\n"
		"	float w3 = dot(abs(c00 - c11), dt) * md2;\n"
		"	float w4 = dot(abs(c20 - c11), dt) * md1;\n"

		"	float t1 = w1 + w3;\n"
		"	float t2 = w2 + w4;\n"
		"	float ww = max(t1, t2) + 0.0001;\n"

		"	c11 = (w1 * c00 + w2 * c20 + w3 * c22 + w4 * c02 + ww * c11) / (t1 + t2 + ww);\n"

		"	float lc1 = k / (0.12 * dot(c10 + c12 + c11, dt) + lum_add);\n"
		"	float lc2 = k / (0.12 * dot(c01 + c21 + c11, dt) + lum_add);\n"

		"	w1 = clamp(lc1 * dot(abs(c11 - c10), dt) + mx, min_w, max_w);\n"
		"	w2 = clamp(lc2 * dot(abs(c11 - c21), dt) + mx, min_w, max_w);\n"
		"	w3 = clamp(lc1 * dot(abs(c11 - c12), dt) + mx, min_w, max_w);\n"
		"	w4 = clamp(lc2 * dot(abs(c11 - c01), dt) + mx, min_w, max_w);\n"

		"	float4 scr = float4(w1 * c10 + w2 * c21 + w3 * c12 + w4 * c01 +"
		"			  (1.0 - w1 - w2 - w3 - w4) * c11, 1.0);\n"

		"	return scr;\n"
		"}"
	},
	/*****************************************************************************************/
	/* Pixellate                                                                             */
	/*****************************************************************************************/
	{
		// vertex shader
		"struct VsOutput {\n"
		"	float4 Position : POSITION;\n"
		"	float2 TexCoord : TEXCOORD0;\n"
		"};\n"

		"float4x4 m_world_view_projection : WORLDVIEWPROJECTION;\n"

		"VsOutput Vs(float4 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(position, m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"

		"texture texture_scr;\n"
		"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

		"#define round_1(x) floor((x) + 0.5)\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float2 texelSize = (1.0 / size_texture);\n"

		"	float2 range = float2(abs(ddx(texCoord.x)), abs(ddy(texCoord.y)));\n"
		"	range = (range / 2.0) * 0.999;\n"

		"	float left   = texCoord.x - range.x;\n"
		"	float top    = texCoord.y + range.y;\n"
		"	float right  = texCoord.x + range.x;\n"
		"	float bottom = texCoord.y - range.y;\n"

		"	float4 topLeftColor     = tex2D(s0, float2(left, top));\n"
		"	float4 bottomRightColor = tex2D(s0, float2(right, bottom));\n"
		"	float4 bottomLeftColor  = tex2D(s0, float2(left, bottom));\n"
		"	float4 topRightColor    = tex2D(s0, float2(right, top));\n"

		"	float2 border = clamp(round_1(texCoord / texelSize) * texelSize,"
		"			float2(left, bottom), float2(right, top));\n"

		"	float totalArea = 4.0 * range.x * range.y;\n"

		"	float4 scr;\n"
		"	scr  = ((border.x - left) * (top - border.y) / totalArea) * topLeftColor;\n"
		"	scr += ((right - border.x) * (border.y - bottom) / totalArea) * bottomRightColor;\n"
		"	scr += ((border.x - left) * (border.y - bottom) / totalArea) * bottomLeftColor;\n"
		"	scr += ((right - border.x) * (top - border.y) / totalArea) * topRightColor;\n"

		"	return scr;\n"
		"}"
	},
	/*****************************************************************************************/
	/* Phosphor                                                                              */
	/*****************************************************************************************/
	{
		// vertex shader
		"struct VsOutput {\n"
		"	float4 Position : POSITION;\n"
		"	float2 TexCoord : TEXCOORD0;\n"
		"};\n"

		"float4x4 m_world_view_projection : WORLDVIEWPROJECTION;\n"

		"VsOutput Vs(float4 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(position, m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"

		"texture texture_scr;\n"
		"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

		"float mod(float x, float y) {\n"
		"	return x - y * floor(x / y);\n"
		"}\n"

		"float3 to_focus(float pixel) {\n"
		"	pixel = mod(pixel + 3.0, 3.0);\n"
		"	if (pixel >= 2.0) {        //  Blue\n"
		"		return float3(pixel - 2.0, 0.0, 3.0 - pixel);\n"
		"	} else if (pixel >= 1.0) { // Green\n"
		"		return float3(0.0, 2.0 - pixel, pixel - 1.0);\n"
		"	} else {                   //  Red\n"
		"		return float3(1.0 - pixel, pixel, 0.0);\n"
		"	}\n"
		"}\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float y = mod(texCoord.y * size_texture.y, 1.0);\n"
		"	float intensity = exp(-0.2 * y);\n"

		"	float2 one_x = float2((1.0 / (3.0 * size_texture.x)), 0.0);\n"

		"	float3 color = tex2D(s0, texCoord).rgb;\n"
		"	float3 color_prev = tex2D(s0, texCoord - one_x).rgb;\n"
		"	float3 color_prev_prev = tex2D(s0, texCoord - (2.0 * one_x)).rgb;\n"

		"	float pixel_x = 3.0 * (texCoord.x * size_texture.x);\n"

		"	float3 focus = to_focus(pixel_x - 0.0);\n"
		"	float3 focus_prev = to_focus(pixel_x - 1.0);\n"
		"	float3 focus_prev_prev = to_focus(pixel_x - 2.0);\n"

		"	float3 result = 0.8 * color * focus +"
		"					0.6 * color_prev * focus_prev +"
		"					0.3 * color_prev_prev * focus_prev_prev;\n"

		"	result = 2.3 * pow(result, float3(1.4, 1.4, 1.4));\n"

		"	float4 scr = float4(intensity * result, 1.0);\n"

		"	return scr;\n"
		"}"
	},
	/*****************************************************************************************/
	/* Scanline                                                                              */
	/*****************************************************************************************/
	{
		// vertex shader
		"struct VsOutput {\n"
		"	float4 Position : POSITION;\n"
		"	float2 TexCoord : TEXCOORD0;\n"
		"};\n"

		"float4x4 m_world_view_projection : WORLDVIEWPROJECTION;\n"

		"VsOutput Vs(float4 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(position, m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"

		"texture texture_scr;\n"
		"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

		"static const float base_brightness = 0.95;\n"
		"static const float2 sine_comp = float2(0.05, 0.15);\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float2 omega = float2(3.1415 * size_video_mode.x * size_texture.x / size_screen_emu.x,"
		"							2.0 * 3.1415 * size_texture.y);\n"

		"	float4 c11 = tex2D(s0, texCoord);\n"

		"	float4 scanline = c11 * (base_brightness + dot(sine_comp * sin(texCoord * omega),"
		"			float2(1.0, 1.0)));\n"

		"	float4 scr = saturate(scanline);\n"

		"	return scr;\n"
		"}"
	},
	/*****************************************************************************************/
	/* CRT                                                                                   */
	/*****************************************************************************************/
	{
		// vertex shader
		"struct VsOutput {\n"
		"	float4 Position : POSITION;\n"
		"	float2 TexCoord : TEXCOORD0;\n"
		"};\n"

		"float4x4 m_world_view_projection : WORLDVIEWPROJECTION;\n"

		"VsOutput Vs(float4 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(position, m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"

		"texture texture_scr;\n"
		"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

		"// Comment the next line to disable interpolation in linear gamma (and\n"
		"// gain speed).\n"
		"#define LINEAR_PROCESSING\n"

		"// Enable screen curvature.\n"
		"#define CURVATURE\n"

		"// Enable 3x oversampling of the beam profile\n"
		"#define OVERSAMPLE\n"

		"// Use the older, purely gaussian beam profile\n"
		"//#define USEGAUSSIAN\n"

		"// Macros.\n"
		"#define FIX(c) max(abs(c), 1e-5);\n"
		"#define PI 3.141592653589\n"

		"#ifdef LINEAR_PROCESSING\n"
		"#	define TEX2D(c) pow(tex2D(s0, (c)), CRTgamma)\n"
		"#else\n"
		"#  define TEX2D(c) tex2D(s0, (c))\n"
		"#endif\n"

		"// START of parameters\n"

		"// gamma of simulated CRT\n"
		"static float CRTgamma = 2.4;\n"
		"// gamma of display monitor (typically 2.2 is correct)\n"
		"static float monitorgamma = 2.2;\n"
		"// overscan (e.g. 1.02 for 2% overscan)\n"
		"static float2 overscan = float2(1.01,1.01);\n"
		"// aspect ratio\n"
		"static float2 aspect = float2(1.0, 0.75);\n"
		"// lengths are measured in units of (approximately) the width\n"
		"// of the monitor simulated distance from viewer to monitor\n"
		"static float d = 2.0;\n"
		"// radius of curvature\n"
		"static float R = 1.5;\n"
		"// tilt angle in radians\n"
		"// (behavior might be a bit wrong if both components are\n"
		"// nonzero)\n"
		"static float2 angle = float2(0.0,-0.15);\n"
		"// size of curved corners\n"
		"static float cornersize = 0.03;\n"
		"// border smoothness parameter\n"
		"// decrease if borders are too aliased\n"
		"static float cornersmooth = 1000.0;\n"

		"// END of parameters\n"

		"static float2 sinangle = sin(angle);\n"
		"static float2 cosangle = cos(angle);\n"

		"float intersect(float2 xy) {\n"
		"	float A = dot(xy,xy)+d*d;\n"
		"	float B = 2.0*(R*(dot(xy,sinangle)-d*cosangle.x*cosangle.y)-d*d);\n"
		"	float C = d*d + 2.0*R*d*cosangle.x*cosangle.y;\n"
		"	return (-B-sqrt(B*B-4.0*A*C))/(2.0*A);\n"
		"}\n"

		"float2 bkwtrans(float2 xy) {\n"
		"	float c = intersect(xy);\n"
		"	float2 pnt = c*xy;\n"
		"	pnt -= -R*sinangle;\n"
		"	pnt /= R;\n"
		"	float2 tang = sinangle/cosangle;\n"
		"	float2 poc = pnt/cosangle;\n"
		"	float A = dot(tang,tang)+1.0;\n"
		"	float B = -2.0*dot(poc,tang);\n"
		"	float C = dot(poc,poc)-1.0;\n"
		"	float a = (-B+sqrt(B*B-4.0*A*C))/(2.0*A);\n"
		"	float2 uv = (pnt-a*sinangle)/cosangle;\n"
		"	float r = R*acos(a);\n"
		"	return uv*r/sin(r/R);\n"
		"}\n"

		"float2 fwtrans(float2 uv) {\n"
		"	float r = FIX(sqrt(dot(uv,uv)));\n"
		"	uv *= sin(r/R)/r;\n"
		"	float x = 1.0-cos(r/R);\n"
		"	float D = d/R + x*cosangle.x*cosangle.y+dot(uv,sinangle);\n"
		"	return d*(uv*cosangle-x*sinangle)/D;\n"
		"}\n"

		"float3 maxscale() {\n"
		"	float2 c = bkwtrans(-R * sinangle / (1.0 + R/d*cosangle.x*cosangle.y));\n"
		"	float2 a = float2(0.5,0.5)*aspect;\n"
		"	float2 lo = float2(fwtrans(float2(-a.x,c.y)).x,\n"
		"			fwtrans(float2(c.x,-a.y)).y)/aspect;\n"
		"	float2 hi = float2(fwtrans(float2(+a.x,c.y)).x,\n"
		"			fwtrans(float2(c.x,+a.y)).y)/aspect;\n"
		"	return float3((hi+lo)*aspect*0.5,max(hi.x-lo.x,hi.y-lo.y));\n"
		"}\n"

		"static float3 stretch = maxscale();\n"

		"float2 transform(float2 coord) {\n"
		"	coord *= size_texture / size_screen_emu;\n"
		"	coord = (coord-0.5)*aspect*stretch.z+stretch.xy;\n"
		"	return (bkwtrans(coord)/overscan/aspect+0.5) * size_screen_emu / size_texture;\n"
		"}\n"

		"float corner(float2 coord) {\n"
		"	coord *= size_texture / size_screen_emu;\n"
		"	coord = (coord - 0.5) * overscan + 0.5;\n"
		"	coord = min(coord, 1.0-coord) * aspect;\n"
		"	float2 cdist = cornersize;\n"
		"	coord = (cdist - min(coord,cdist));\n"
		"	float dist = sqrt(dot(coord,coord));\n"
		"	return clamp((cdist.x-dist)*cornersmooth,0.0, 1.0);\n"
		"}\n"

		"// Calculate the influence of a scanline on the current pixel.\n"
		"// 'distance' is the distance in texture coordinates from the current\n"
		"// pixel to the scanline in question.\n"
		"// 'color' is the colour of the scanline at the horizontal location of\n"
		"// the current pixel.\n"
		"float4 scanlineWeights(float distance, float4 color) {\n"
		"	// 'wid' controls the width of the scanline beam, for each RGB\n"
		"	// channel The 'weights' lines basically specify the formula\n"
		"	// that gives you the profile of the beam, i.e. the intensity as\n"
		"	// a function of distance from the vertical center of the\n"
		"	// scanline. In this case, it is gaussian if width=2, and\n"
		"	// becomes nongaussian for larger widths. Ideally this should\n"
		"	// be normalized so that the integral across the beam is\n"
		"	// independent of its width. That is, for a narrower beam\n"
		"	// 'weights' should have a higher peak at the center of the\n"
		"	// scanline than for a wider beam.\n"
		"#ifdef USEGAUSSIAN\n"
		"	float4 wid = 0.3 + 0.1 * pow(color, 3.0);\n"
		"	float4 weights = distance / wid;\n"
		"	return 0.4 * exp(-weights * weights) / wid;\n"
		"#else\n"
		"	float4 wid = 2.0 + 2.0 * pow(color, 4.0);\n"
		"	float4 weights = distance / 0.3;\n"
		"	return 1.4 * exp(-pow(weights * rsqrt(0.5 * wid), wid)) / (0.6 + 0.2 * wid);\n"
		"#endif\n"
		"}\n"

		"float2 fract(float2 x) {\n"
		"	return x - floor(x);\n"
		"}\n"

		"float mod(float x, float y) {\n"
		"	return x - y * floor(x / y);\n"
		"}\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	// The size of one texel, in texture-coordinates.\n"
		"	float2 one = (1.0 / size_texture);\n"

		"	// Resulting X pixel-coordinate of the pixel we're drawing.\n"
		"	float mod_factor = texCoord.x * size_texture.x * size_texture.x / size_screen_emu.x;\n"

		"	// Here's a helpful diagram to keep in mind while trying to\n"
		"	// understand the code:\n"
		"	//\n"
		"	//  |      |      |      |      |\n"
		"	// -------------------------------\n"
		"	//  |      |      |      |      |\n"
		"	//  |  01  |  11  |  21  |  31  | <-- current scanline\n"
		"	//  |      | @    |      |      |\n"
		"	// -------------------------------\n"
		"	//  |      |      |      |      |\n"
		"	//  |  02  |  12  |  22  |  32  | <-- next scanline\n"
		"	//  |      |      |      |      |\n"
		"	// -------------------------------\n"
		"	//  |      |      |      |      |\n"
		"	//\n"
		"	// Each character-cell represents a pixel on the output\n"
		"	// surface, '@' represents the current pixel (always somewhere\n"
		"	// in the bottom half of the current scan-line, or the top-half\n"
		"	// of the next scanline). The grid of lines represents the\n"
		"	// edges of the texels of the underlying texture.\n"

		"	// Texture coordinates of the texel containing the active pixel.\n"
		"#ifdef CURVATURE\n"
		"	float2 xy = transform(texCoord);\n"
		"#else\n"
		"	float2 xy = texCoord;\n"
		"#endif\n"
		"	float cval = corner(xy);\n"

		"	// Of all the pixels that are mapped onto the texel we are\n"
		"	// currently rendering, which pixel are we currently rendering?\n"
		"	float2 ratio_scale = xy * size_texture - 0.5;\n"

		"#ifdef OVERSAMPLE\n"
		"	float filter = fwidth(ratio_scale.y);\n"
		"#endif\n"

		"	float2 uv_ratio = fract(ratio_scale);\n"

		"	// Snap to the center of the underlying texel.\n"
		"	xy = (floor(ratio_scale) + 0.5) / size_texture;\n"

		"	// Calculate Lanczos scaling coefficients describing the effect\n"
		"	// of various neighbour texels in a scanline on the current\n"
		"	// pixel.\n"
		"	float4 coeffs = PI * float4(1.0 + uv_ratio.x, uv_ratio.x, 1.0 - uv_ratio.x, 2.0 - uv_ratio.x);\n"

		"	// Prevent division by zero.\n"
		"	coeffs = FIX(coeffs);\n"

		"	// Lanczos2 kernel.\n"
		"	coeffs = 2.0 * sin(coeffs) * sin(coeffs / 2.0) / (coeffs * coeffs);\n"

		"	// Normalize.\n"
		"	coeffs /= dot(coeffs, 1.0);\n"

		"	// Calculate the effective colour of the current and next\n"
		"	// scanlines at the horizontal location of the current pixel,\n"
		"	// using the Lanczos coefficients above.\n"
		"	float4 col = clamp(mul(coeffs, float4x4("
		"			TEX2D(xy + float2(-one.x, 0.0)),"
		"			TEX2D(xy),"
		"			TEX2D(xy + float2(one.x, 0.0)),"
		"			TEX2D(xy + float2(2.0 * one.x, 0.0)))),"
		"			0.0, 1.0);\n"
		"	float4 col2 = clamp(mul(coeffs, float4x4("
		"			TEX2D(xy + float2(-one.x, one.y)),"
		"			TEX2D(xy + float2(0.0, one.y)),"
		"			TEX2D(xy + one),"
		"			TEX2D(xy + float2(2.0 * one.x, one.y)))),"
		"			0.0, 1.0);\n"

		"#ifndef LINEAR_PROCESSING\n"
		"	col  = pow(col , CRTgamma);\n"
		"	col2 = pow(col2, CRTgamma);\n"
		"#endif\n"

		"	// Calculate the influence of the current and next scanlines on\n"
		"	// the current pixel.\n"
		"	float4 weights = scanlineWeights(uv_ratio.y, col);\n"
		"	float4 weights2 = scanlineWeights(1.0 - uv_ratio.y, col2);\n"

		"#ifdef OVERSAMPLE\n"
		"	uv_ratio.y =uv_ratio.y+1.0/3.0*filter;\n"
		"	weights = (weights+scanlineWeights(uv_ratio.y, col))/3.0;\n"
		"	weights2=(weights2+scanlineWeights(abs(1.0-uv_ratio.y), col2))/3.0;\n"
		"	uv_ratio.y =uv_ratio.y-2.0/3.0*filter;\n"
		"	weights=weights+scanlineWeights(abs(uv_ratio.y), col)/3.0;\n"
		"	weights2=weights2+scanlineWeights(abs(1.0-uv_ratio.y), col2)/3.0;\n"
		"#endif\n"

		"	float3 mul_res = (col * weights + col2 * weights2).rgb * cval;\n"

		"	// dot-mask emulation:\n"
		"	// Output pixels are alternately tinted green and magenta.\n"
		"	float3 dotMaskWeights = lerp("
		"			float3(1.0, 0.7, 1.0),"
		"			float3(0.7, 1.0, 0.7),"
		"			floor(mod(mod_factor, 2.0))"
		"	);\n"

		"	mul_res *= dotMaskWeights;\n"

		"	// Convert the image gamma for display on our output device.\n"
		"	mul_res = pow(mul_res, 1.0 / monitorgamma);\n"

		"	// Color the texel.\n"
		"	float4 scr = float4(mul_res, 1.0);\n"

		"	return scr;"
		"}"
	},
	/*****************************************************************************************/
	/* CRT4                                                                                  */
	/*****************************************************************************************/
	{
		// vertex shader
		"struct VsOutput {\n"
		"	float4 Position : POSITION;\n"
		"	float2 TexCoord : TEXCOORD0;\n"
		"};\n"

		"float4x4 m_world_view_projection : WORLDVIEWPROJECTION;\n"

		"VsOutput Vs(float4 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(position, m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"
		"float frame_counter;\n"

		"texture texture_scr;\n"
		"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

		"// Comment the next line to disable interpolation in linear gamma (and\n"
		"// gain speed).\n"
		"//#define LINEAR_PROCESSING\n"

		"// Enable screen curvature.\n"
		"//#define CURVATURE\n"

		"// Enable 3x oversampling of the beam profile\n"
		"//#define OVERSAMPLE\n"

		"// Use the older, purely gaussian beam profile\n"
		"#define USEGAUSSIAN\n"

		"// Macros.\n"
		"#define FIX(c) max(abs(c), 1e-5);\n"
		"#define PI 3.141592653589\n"

		"#ifdef LINEAR_PROCESSING\n"
		"#	define TEX2D(c) pow(tex2D(s0, (c)), CRTgamma)\n"
		"#else\n"
		"#  define TEX2D(c) tex2D(s0, (c))\n"
		"#endif\n"

		"// START of parameters\n"

		"// gamma of simulated CRT\n"
		"static float CRTgamma = 2.4;\n"
		"// gamma of display monitor (typically 2.2 is correct)\n"
		"static float monitorgamma = 2.2;\n"
		"// overscan (e.g. 1.02 for 2% overscan)\n"
		"static float2 overscan = float2(1.01,1.01);\n"
		"// aspect ratio\n"
		"static float2 aspect = float2(1.0, 0.75);\n"
		"// lengths are measured in units of (approximately) the width\n"
		"// of the monitor simulated distance from viewer to monitor\n"
		"static float d = 2.0;\n"
		"// radius of curvature\n"
		"static float R = 1.5;\n"
		"// tilt angle in radians\n"
		"// (behavior might be a bit wrong if both components are\n"
		"// nonzero)\n"
		"static float2 angle = float2(0.0,-0.15);\n"
		"// size of curved corners\n"
		"static float cornersize = 0.03;\n"
		"// border smoothness parameter\n"
		"// decrease if borders are too aliased\n"
		"static float cornersmooth = 1000.0;\n"

		"// END of parameters\n"

		"// Precalculate a bunch of useful values we'll need in the fragment\n"
		"// shader.\n"
		"static float2 sinangle = sin(angle);\n"
		"static float2 cosangle = cos(angle);\n"

		"float intersect(float2 xy) {\n"
		"	float A = dot(xy,xy)+d*d;\n"
		"	float B = 2.0*(R*(dot(xy,sinangle)-d*cosangle.x*cosangle.y)-d*d);\n"
		"	float C = d*d + 2.0*R*d*cosangle.x*cosangle.y;\n"
		"	return (-B-sqrt(B*B-4.0*A*C))/(2.0*A);\n"
		"}\n"

		"float2 bkwtrans(float2 xy) {\n"
		"	float c = intersect(xy);\n"
		"	float2 pnt = c*xy;\n"
		"	pnt -= -R*sinangle;\n"
		"	pnt /= R;\n"
		"	float2 tang = sinangle/cosangle;\n"
		"	float2 poc = pnt/cosangle;\n"
		"	float A = dot(tang,tang)+1.0;\n"
		"	float B = -2.0*dot(poc,tang);\n"
		"	float C = dot(poc,poc)-1.0;\n"
		"	float a = (-B+sqrt(B*B-4.0*A*C))/(2.0*A);\n"
		"	float2 uv = (pnt-a*sinangle)/cosangle;\n"
		"	float r = R*acos(a);\n"
		"	return uv*r/sin(r/R);\n"
		"}\n"

		"float2 fwtrans(float2 uv) {\n"
		"	float r = FIX(sqrt(dot(uv,uv)));\n"
		"	uv *= sin(r/R)/r;\n"
		"	float x = 1.0-cos(r/R);\n"
		"	float D = d/R + x*cosangle.x*cosangle.y+dot(uv,sinangle);\n"
		"	return d*(uv*cosangle-x*sinangle)/D;\n"
		"}\n"

		"float3 maxscale() {\n"
		"	float2 c = bkwtrans(-R * sinangle / (1.0 + R/d*cosangle.x*cosangle.y));\n"
		"	float2 a = float2(0.5,0.5)*aspect;\n"
		"	float2 lo = float2(fwtrans(float2(-a.x,c.y)).x,\n"
		"			fwtrans(float2(c.x,-a.y)).y)/aspect;\n"
		"	float2 hi = float2(fwtrans(float2(+a.x,c.y)).x,\n"
		"			fwtrans(float2(c.x,+a.y)).y)/aspect;\n"
		"	return float3((hi+lo)*aspect*0.5,max(hi.x-lo.x,hi.y-lo.y));\n"
		"}\n"

		"static float3 stretch = maxscale();\n"

		"float2 transform(float2 coord) {\n"
		"	coord *= size_texture / size_screen_emu;\n"
		"	coord = (coord-0.5)*aspect*stretch.z+stretch.xy;\n"
		"	return (bkwtrans(coord)/overscan/aspect+0.5) * size_screen_emu / size_texture;\n"
		"}\n"

		"float corner(float2 coord) {\n"
		"	coord *= size_texture / size_screen_emu;\n"
		"	coord = (coord - 0.5) * overscan + 0.5;\n"
		"	coord = min(coord, 1.0-coord) * aspect;\n"
		"	float2 cdist = cornersize;\n"
		"	coord = (cdist - min(coord,cdist));\n"
		"	float dist = sqrt(dot(coord,coord));\n"
		"	return clamp((cdist.x-dist)*cornersmooth,0.0, 1.0);\n"
		"}\n"

		"// Calculate the influence of a scanline on the current pixel.\n"
		"// 'distance' is the distance in texture coordinates from the current\n"
		"// pixel to the scanline in question.\n"
		"// 'color' is the colour of the scanline at the horizontal location of\n"
		"// the current pixel.\n"
		"float4 scanlineWeights(float distance, float4 color) {\n"
		"	// 'wid' controls the width of the scanline beam, for each RGB\n"
		"	// channel The 'weights' lines basically specify the formula\n"
		"	// that gives you the profile of the beam, i.e. the intensity as\n"
		"	// a function of distance from the vertical center of the\n"
		"	// scanline. In this case, it is gaussian if width=2, and\n"
		"	// becomes nongaussian for larger widths. Ideally this should\n"
		"	// be normalized so that the integral across the beam is\n"
		"	// independent of its width. That is, for a narrower beam\n"
		"	// 'weights' should have a higher peak at the center of the\n"
		"	// scanline than for a wider beam.\n"
		"#ifdef USEGAUSSIAN\n"
		"	float4 wid = 0.3 + 0.1 * pow(color, 3.0);\n"
		"	float4 weights = distance / wid;\n"
		"	return 0.4 * exp(-weights * weights) / wid;\n"
		"#else\n"
		"	float4 wid = 2.0 + 2.0 * pow(color, 4.0);\n"
		"	float4 weights = distance / 0.3;\n"
		"	return 1.4 * exp(-pow(weights * rsqrt(0.5 * wid), wid)) / (0.6 + 0.2 * wid);\n"
		"#endif\n"
		"}\n"

		"float2 fract(float2 x) {\n"
		"	return x - floor(x);\n"
		"}\n"

		"float mod(float x, float y) {\n"
		"	return x - y * floor(x / y);\n"
		"}\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float2 ilfac = float2(1.0,floor(size_screen_emu.y/200.0));\n"

		"	// The size of one texel, in texture-coordinates.\n"
		"	float2 one = (ilfac / size_texture);\n"

		"	// Resulting X pixel-coordinate of the pixel we're drawing.\n"
		"	float mod_factor = texCoord.x * size_texture.x * size_texture.x / size_screen_emu.x;\n"

		"	// Here's a helpful diagram to keep in mind while trying to\n"
		"	// understand the code:\n"
		"	//\n"
		"	//  |      |      |      |      |\n"
		"	// -------------------------------\n"
		"	//  |      |      |      |      |\n"
		"	//  |  01  |  11  |  21  |  31  | <-- current scanline\n"
		"	//  |      | @    |      |      |\n"
		"	// -------------------------------\n"
		"	//  |      |      |      |      |\n"
		"	//  |  02  |  12  |  22  |  32  | <-- next scanline\n"
		"	//  |      |      |      |      |\n"
		"	// -------------------------------\n"
		"	//  |      |      |      |      |\n"
		"	//\n"
		"	// Each character-cell represents a pixel on the output\n"
		"	// surface, '@' represents the current pixel (always somewhere\n"
		"	// in the bottom half of the current scan-line, or the top-half\n"
		"	// of the next scanline). The grid of lines represents the\n"
		"	// edges of the texels of the underlying texture.\n"

		"	// Texture coordinates of the texel containing the active pixel.\n"
		"#ifdef CURVATURE\n"
		"	float2 xy = transform(texCoord);\n"
		"#else\n"
		"	float2 xy = texCoord;\n"
		"#endif\n"
		"	float cval = corner(xy);\n"

		"	// Of all the pixels that are mapped onto the texel we are\n"
		"	// currently rendering, which pixel are we currently rendering?\n"
		"	float2 ilvec = float2(0.0,ilfac.y > 1.5 ? mod(frame_counter,2.0) : 0.0);\n"
		"	float2 ratio_scale = (xy * size_texture - 0.5 + ilvec)/ilfac;\n"
		"#ifdef OVERSAMPLE\n"
		"	float filter = fwidth(ratio_scale.y);\n"
		"#endif\n"
		"	float2 uv_ratio = fract(ratio_scale);\n"

		"	// Snap to the center of the underlying texel.\n"
		"	xy = (floor(ratio_scale)*ilfac + 0.5 - ilvec) / size_texture;\n"

		"	// Calculate Lanczos scaling coefficients describing the effect\n"
		"	// of various neighbour texels in a scanline on the current\n"
		"	// pixel.\n"
		"	float4 coeffs = PI * float4(1.0 + uv_ratio.x, uv_ratio.x, 1.0 - uv_ratio.x, 2.0 - uv_ratio.x);\n"

		"	// Prevent division by zero.\n"
		"	coeffs = FIX(coeffs);\n"

		"	// Lanczos2 kernel.\n"
		"	coeffs = 2.0 * sin(coeffs) * sin(coeffs / 2.0) / (coeffs * coeffs);\n"

		"	// Normalize.\n"
		"	coeffs /= dot(coeffs, 1.0);\n"

		"	// Calculate the effective colour of the current and next\n"
		"	// scanlines at the horizontal location of the current pixel,\n"
		"	// using the Lanczos coefficients above.\n"
		"	float4 col = clamp(mul(coeffs, float4x4("
		"			TEX2D(xy + float2(-one.x, 0.0)),"
		"			TEX2D(xy),"
		"			TEX2D(xy + float2(one.x, 0.0)),"
		"			TEX2D(xy + float2(2.0 * one.x, 0.0)))),"
		"			0.0, 1.0);\n"
		"	float4 col2 = clamp(mul(coeffs, float4x4("
		"			TEX2D(xy + float2(-one.x, one.y)),"
		"			TEX2D(xy + float2(0.0, one.y)),"
		"			TEX2D(xy + one),"
		"			TEX2D(xy + float2(2.0 * one.x, one.y)))),"
		"			0.0, 1.0);\n"

		"#ifndef LINEAR_PROCESSING\n"
		"	col  = pow(col , CRTgamma);\n"
		"	col2 = pow(col2, CRTgamma);\n"
		"#endif\n"

		"	// Calculate the influence of the current and next scanlines on\n"
		"	// the current pixel.\n"
		"	float4 weights = scanlineWeights(uv_ratio.y, col);\n"
		"	float4 weights2 = scanlineWeights(1.0 - uv_ratio.y, col2);\n"

		"#ifdef OVERSAMPLE\n"
		"	uv_ratio.y =uv_ratio.y+1.0/3.0*filter;\n"
		"	weights = (weights+scanlineWeights(uv_ratio.y, col))/3.0;\n"
		"	weights2=(weights2+scanlineWeights(abs(1.0-uv_ratio.y), col2))/3.0;\n"
		"	uv_ratio.y =uv_ratio.y-2.0/3.0*filter;\n"
		"	weights=weights+scanlineWeights(abs(uv_ratio.y), col)/3.0;\n"
		"	weights2=weights2+scanlineWeights(abs(1.0-uv_ratio.y), col2)/3.0;\n"
		"#endif\n"
		"	float3 mul_res = (col * weights + col2 * weights2).rgb * cval;\n"

		"	// dot-mask emulation:\n"
		"	// Output pixels are alternately tinted green and magenta.\n"
		"	float3 dotMaskWeights = lerp("
		"			float3(1.0, 0.7, 1.0),"
		"			float3(0.7, 1.0, 0.7),"
		"			floor(mod(mod_factor, 2.0))"
		"	);\n"

		"	mul_res *= dotMaskWeights;\n"

		"	// Convert the image gamma for display on our output device.\n"
		"	mul_res = pow(mul_res, 1.0 / monitorgamma);\n"

		"	// Color the texel.\n"
		"	float4 scr = float4(mul_res, 1.0);\n"

		"	return scr;"
		"}"
	},
	/*****************************************************************************************/
	/* don't BLOOM                                                                           */
	/*****************************************************************************************/
	{
		// vertex shader
		"struct VsOutput {\n"
		"	float4 Position : POSITION;\n"
		"	float2 TexCoord : TEXCOORD0;\n"
		"};\n"

		"float4x4 m_world_view_projection;"

		"VsOutput Vs(float4 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(position, m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"

		"texture texture_scr;\n"
		"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

		"static const float gamma = 2.4;\n"
		"static const float shine = 0.05;\n"
		"static const float blend = 0.65;\n"

		"float dist(float2 coord, float2 source) {\n"
		"	float2 delta = coord - source;\n"
		"	return sqrt(dot(delta, delta));\n"
		"}\n"

		"float color_bloom(float3 color) {\n"
		"	const float3 gray_coeff = float3(0.30, 0.59, 0.11);\n"
		"	float bright = dot(color, gray_coeff);\n"
		"	return lerp(1.0 + shine, 1.0 - shine, bright);\n"
		"}\n"

		"float2 fract(float2 x) {\n"
		"	return x - floor(x);\n"
		"}\n"

		"float3 lookup(float offset_x, float offset_y, float2 coord, float2 pix) {\n"
		"	float2 offset = float2(offset_x, offset_y);\n"
		"	float3 color = tex2D(s0, coord).rgb;\n"
		/*"	float delta = dist(fract(pix), offset + float2(0.5, 0.5));\n"*/
		"	float delta = dist(fract(pix), offset + float2(0.4, 0.4));\n"
		"	return color * exp(-gamma * delta * color_bloom(color));\n"
		"}\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float dx = (1.0 / size_texture.x);\n"
		"	float dy = (1.0 / size_texture.y);\n"

		"	float2 c00 = texCoord + float2(-dx, -dy);\n"
		"	float2 c10 = texCoord + float2(  0, -dy);\n"
		"	float2 c20 = texCoord + float2( dx, -dy);\n"
		"	float2 c01 = texCoord + float2(-dx,   0);\n"
		"	float2 c11 = texCoord + float2(  0,   0);\n"
		"	float2 c21 = texCoord + float2( dx,   0);\n"
		"	float2 c02 = texCoord + float2(-dx,  dy);\n"
		"	float2 c12 = texCoord + float2(  0,  dy);\n"
		"	float2 c22 = texCoord + float2( dx,  dy);\n"
		"	float2 pixel_no = texCoord * size_texture;\n"

		"	float3 mid_color = lookup(0.0, 0.0, c11, pixel_no);\n"
		"	float3 color = float3(0.0, 0.0, 0.0);\n"
		"	color += lookup(-1.0, -1.0, c00, pixel_no);\n"
		"	color += lookup( 0.0, -1.0, c10, pixel_no);\n"
		"	color += lookup( 1.0, -1.0, c20, pixel_no);\n"
		"	color += lookup(-1.0,  0.0, c01, pixel_no);\n"
		"	color += mid_color;\n"
		"	color += lookup( 1.0, 0.0, c21, pixel_no);\n"
		"	color += lookup(-1.0, 1.0, c02, pixel_no);\n"
		"	color += lookup( 0.0, 1.0, c12, pixel_no);\n"
		"	color += lookup( 1.0, 1.0, c22, pixel_no);\n"
		"	float3 out_color = lerp(1.2 * mid_color, color, blend);\n"

		"	float4 scr = float4(out_color, 1.0);\n"

		"	return scr;\n"
		"}"
	},
	/*****************************************************************************************/
	/* NTSC                                                                                  */
	/*****************************************************************************************/
	{
		// vertex shader
		"struct VsOutput {\n"
		"	float4 Position : POSITION;\n"
		"	float2 TexCoord : TEXCOORD0;\n"
		"};\n"

		"float4x4 m_world_view_projection;"

		"VsOutput Vs(float4 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(position, m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"

		"texture texture_scr;\n"
		"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

		"static float3x3 rgb2yuv = float3x3("
		"	 0.299  , 0.587  , 0.114  ,"
		"	-0.14713,-0.28886, 0.436  ,"
		"	 0.615  ,-0.51499,-0.10001 "
		");\n"
		"static	float3x3 yuv2rgb = float3x3("
		"	1.0    , 0.0    , 1.13983,"
		"	1.0    ,-0.39465,-0.58060,"
		"	1.0    , 2.03211, 0.0     "
		");\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"

		"	float4 wid = float4(3.0, 3.0, 3.0, 3.0);\n"
		"	float4 c1 = float4(exp(float4(-1.0, -1.0, -1.0, -1.0) / wid / wid));\n"
		"	float4 c2 = float4(exp(float4(-4.0, -4.0, -4.0, -4.0) / wid / wid));\n"
		"	float4 c3 = float4(exp(float4(-9.0, -9.0, -9.0, -9.0) / wid / wid));\n"
		"	float4 c4 = float4(exp(float4(-16.0, -16.0, -16.0, -16.0) / wid / wid));\n"
		"	float4 norm = 1.0 / (float4(1.0, 1.0, 1.0, 1.0) + float4(2.0, 2.0, 2.0, 2.0) *"
		"			(c1 + c2 + c3 + c4));\n"

		"	float one_x = (1.0 / size_texture.x);\n"

		"	float4 sum = float4(0.0, 0.0, 0.0, 0.0);\n"
		"	sum += tex2D(s0, texCoord + float2(-4.0 * one_x, 0.0)) * c4;\n"
		"	sum += tex2D(s0, texCoord + float2(-3.0 * one_x, 0.0)) * c3;\n"
		"	sum += tex2D(s0, texCoord + float2(-2.0 * one_x, 0.0)) * c2;\n"
		"	sum += tex2D(s0, texCoord + float2(-1.0 * one_x, 0.0)) * c1;\n"
		"	sum += tex2D(s0, texCoord);\n"
		"	sum += tex2D(s0, texCoord + float2(+1.0 * one_x, 0.0)) * c1;\n"
		"	sum += tex2D(s0, texCoord + float2(+2.0 * one_x, 0.0)) * c2;\n"
		"	sum += tex2D(s0, texCoord + float2(+3.0 * one_x, 0.0)) * c3;\n"
		"	sum += tex2D(s0, texCoord + float2(+4.0 * one_x, 0.0)) * c4;\n"

		"	float y = mul(rgb2yuv, tex2D(s0, texCoord).rgb).x;\n"
		"	float2 uv = mul(rgb2yuv, float3(sum.rgb * norm.rgb)).yz;\n"

		"	float4 scr = float4(mul(yuv2rgb, float3(y, uv)), 1.0);\n"

		"	return scr;\n"
		"}"
	},
};
#undef _SHADERS_CODE_
#endif
