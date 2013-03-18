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
	//SHADER_HQ2X,
	//SHADER_HQ4X,
	//SHADER_4xBR,
	//SHADER_PIXELLATE,
	SHADER_POSPHOR,
	SHADER_SCANLINE,
	//SHADER_QUILAZ,
	//SHADER_WATERPAINT,
	//SHADER_CRT,
	//SHADER_CRT2,
	//SHADER_CRT3,
	//SHADER_CRT4,
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
	LPDIRECT3DTEXTURE9 data;
	LPDIRECT3DSURFACE9 surface;
	LPDIRECT3DSURFACE9 surface_map0;

	FLOAT x;
	FLOAT y;

	FLOAT w;
	FLOAT h;

	WORD no_pow_w;
	WORD no_pow_h;
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

	_texture text;
} _shader;

#endif /* SHADERS_H_ */

#ifdef _SHADERS_CODE_
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

		"VsOutput Vs(float3 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(float4(position, 1.0), m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"
		"float2 factor;\n"

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

		"VsOutput Vs(float3 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(float4(position, 1.0), m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"
		"float2 factor;\n"

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

		"VsOutput Vs(float3 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(float4(position, 1.0), m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"
		"float2 factor;\n"

		"texture texture_scr;\n"
		"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

		"float2 fract(float2 x) {\n"
		"	return x - floor(x);\n"
		"}\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float x = factor.x * (1.0 / size_texture.x);\n"
		"	float y = factor.y * (1.0 / size_texture.y);\n"
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
		"		float3 tmp1 = p.x < factor.x ? D : F;\n"
		"		float3 tmp2 = p.y < factor.y ? H : B;\n"
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

		"VsOutput Vs(float3 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(float4(position, 1.0), m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"
		"float2 factor;\n"

		"texture texture_scr;\n"
		"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

		"float2 fract(float2 x) {\n"
		"	return x - floor(x);\n"
		"}\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float2 dx = float2(factor.x * (1.0 / size_texture.x), 1.0);\n"
		"	float2 dy = float2(0.0, factor.y * (1.0 / size_texture.y));\n"

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
		//"	const float2 sep = float2(0.33333, 0.66667);\n"
		"	const float2 sep = float2(factor.x, factor.y / 0.5);\n"

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

		"VsOutput Vs(float3 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(float4(position, 1.0), m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"
		"float2 factor;\n"

		"texture texture_scr;\n"
		"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

		"float2 fract(float2 x) {\n"
		"	return x - floor(x);\n"
		"}\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float x = (factor.x * 2.0) * (1.0 / size_texture.x);\n"
		"	float y = (factor.y * 2.0) * (1.0 / size_texture.y);\n"

		"	float2 dg1 = float2( x, y);\n"
		"	float2 dg2 = float2(-x, y);\n"
		"	float2 sd1 = dg1 * 0.5;\n"
		"	float2 sd2 = dg2 * 0.5;\n"

		//"	texCoord = fract(texCoord * (factor * (size_video_mode / size_texture)));\n"

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




	//{
		// vertex shader
	/*
		"struct VsOutput {\n"
		"	float4 Position : POSITION;\n"
		"	float2 TexCoord : TEXCOORD0;\n"
		"};\n"

		"float4x4 m_world_view_projection : WORLDVIEWPROJECTION;\n"

		"VsOutput Vs(float3 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(float4(position, 1.0), m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
	*/
		// pixel shader
	/*
		"float2 size_input;\n"
		"float2 size_output;\n"
		"float2 size_texture;\n"
		"texture texture_scr;\n"
		"sampler s0 = sampler_state { Texture = <texture_scr>; };\n"

		"float2 fract(float2 x) {\n"
		"	return x - floor(x);\n"
		"}\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float x = 0.5 * (1.0 / size_texture.x);\n"
		"	float y = 0.5 * (1.0 / size_texture.y);\n"

		"	return scr;\n"
		"}"
	*/

		// vertex shader
		/*
		"uniform vec2 size_texture;\n"
		"varying vec4 v_texCoord[5];\n"
		"void main() {\n"
		"	float x = 0.5 * (1.0 / size_texture.x);\n"
		"	float y = 0.5 * (1.0 / size_texture.y);\n"

		"	vec2 dg1 = vec2( x, y);\n"
		"	vec2 dg2 = vec2(-x, y);\n"
		"	vec2 dx = vec2(x, 0.0);\n"
		"	vec2 dy = vec2(0.0, y);\n"

		"	gl_FrontColor = gl_Color;\n"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"

		"	v_texCoord[0] = gl_MultiTexCoord0;\n"
		"	v_texCoord[1].xy = v_texCoord[0].xy - dg1;\n"
		"	v_texCoord[1].zw = v_texCoord[0].xy - dy;\n"
		"	v_texCoord[2].xy = v_texCoord[0].xy - dg2;\n"
		"	v_texCoord[2].zw = v_texCoord[0].xy + dx;\n"
		"	v_texCoord[3].xy = v_texCoord[0].xy + dg1;\n"
		"	v_texCoord[3].zw = v_texCoord[0].xy + dy;\n"
		"	v_texCoord[4].xy = v_texCoord[0].xy + dg2;\n"
		"	v_texCoord[4].zw = v_texCoord[0].xy - dx;\n"
		"}",
	*/
		// fragment shader
		/*
		"uniform sampler2D texture_scr;\n"
		"uniform sampler2D texture_txt;\n"
		"varying vec4 v_texCoord[5];\n"

		"const float mx = 0.325;      // start smoothing wt.\n"
		"const float k = -0.250;      // wt. decrease factor\n"
		"const float max_w = 0.25;    // max filter weigth\n"
		"const float min_w =-0.05;    // min filter weigth\n"
		"const float lum_add = 0.25;  // effects smoothing\n"

		"void main() {\n"
		"	vec3 c00 = texture2D(texture_scr, v_texCoord[1].xy).xyz;\n"
		"	vec3 c10 = texture2D(texture_scr, v_texCoord[1].zw).xyz;\n"
		"	vec3 c20 = texture2D(texture_scr, v_texCoord[2].xy).xyz;\n"
		"	vec3 c01 = texture2D(texture_scr, v_texCoord[4].zw).xyz;\n"
		"	vec3 c11 = texture2D(texture_scr, v_texCoord[0].xy).xyz;\n"
		"	vec3 c21 = texture2D(texture_scr, v_texCoord[2].zw).xyz;\n"
		"	vec3 c02 = texture2D(texture_scr, v_texCoord[4].xy).xyz;\n"
		"	vec3 c12 = texture2D(texture_scr, v_texCoord[3].zw).xyz;\n"
		"	vec3 c22 = texture2D(texture_scr, v_texCoord[3].xy).xyz;\n"

		"	vec3 dt = vec3(1.0, 1.0, 1.0);\n"

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

		"	vec4 scr = vec4(w1 * c10 + w2 * c21 + w3 * c12 + w4 * c01 +"
		"			  (1.0 - w1 - w2 - w3 - w4) * c11, 1.0);\n"

		"	vec4 txt = texture2D(texture_txt, v_texCoord[0].xy);\n"

		"	gl_FragColor = mix(scr, txt, txt.a) * gl_Color;\n"
		"}"
		*/
	//},

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

		"VsOutput Vs(float3 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(float4(position, 1.0), m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"
		"float2 factor;\n"

		"texture texture_scr;\n"
		"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

		"float mod(float x, float y) {\n"
		"	return x - y * floor(x/y);\n"
		"}\n"

		"float3 to_focus(float pixel) {\n"
		"	pixel = mod(pixel + 3.0, 3.0);\n"
		"	if (pixel >= 2.0) {                      //  Blue\n"
		"		return float3(pixel - 2.0, 0.0, 3.0 - pixel);\n"
		"	} else if (pixel >= 1.0) {               // Green\n"
		"		return float3(0.0, 2.0 - pixel, pixel - 1.0);\n"
		"	} else {                                 //  Red\n"
		"		return float3(1.0 - pixel, pixel, 0.0);\n"
		"	}\n"
		"}\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float y = mod(texCoord.y * size_texture.y, 1.0);\n"
		"	float intensity = exp(-0.2 * y);\n"

		"	float2 one_x = float2(factor.x * (1.0 / (3.0 * size_texture.x)), 0.0);\n"

		"	float3 color = tex2D(s0, texCoord).rgb;\n"
		"	float3 color_prev = tex2D(s0, texCoord - one_x).rgb;\n"
		"	float3 color_prev_prev = tex2D(s0, texCoord - (2.0 * one_x)).rgb;\n"

		"	float pixel_x = texCoord.x * (3.0 * size_texture.x);\n"

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

		"float4x4 m_world_view_projection;"

		"VsOutput Vs(float3 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(float4(position, 1.0), m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"
		"float2 factor;\n"

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
	/* don't BLOOM                                                                           */
	/*****************************************************************************************/
	{
		// vertex shader
		"struct VsOutput {\n"
		"	float4 Position : POSITION;\n"
		"	float2 TexCoord : TEXCOORD0;\n"
		"};\n"

		"float4x4 m_world_view_projection;"

		"VsOutput Vs(float3 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(float4(position, 1.0), m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"
		"float2 factor;\n"

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
		"	float dx = factor.x * (1.0 / size_texture.x);\n"
		"	float dy = factor.y * (1.0 / size_texture.y);\n"

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

		"VsOutput Vs(float3 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = mul(float4(position, 1.0), m_world_view_projection);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_screen_emu;\n"
		"float2 size_video_mode;\n"
		"float2 size_texture;\n"
		"float2 factor;\n"

		"texture texture_scr;\n"
		"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float3x3 rgb2yuv = float3x3("
		"		0.299,-0.14713, 0.615  ,"
		"		0.587,-0.28886,-0.51499,"
		"		0.114, 0.436  ,-0.10001"
		"	);\n"
		"	float3x3 yuv2rgb = float3x3("
		"		1.0    , 1.0    , 1.0    ,"
		"		0.0    ,-0.39465, 2.03211,"
		"		1.13983,-0.58060, 0.0     "
		"	);\n"

		"	float4 sum = float4(0.0, 0.0, 0.0, 0.0);\n"

		"	float wid = 3.0;\n"
		"	float tmp = 0;\n"
		"	tmp = exp( -1.0 / wid / wid);\n"
		"	float4 c1 = float4(tmp, tmp, tmp, tmp);\n"
		"	tmp = exp( -4.0 / wid / wid);\n"
		"	float4 c2 = float4(tmp, tmp, tmp, tmp);\n"
		"	tmp = exp( -9.0 / wid / wid);\n"
		"	float4 c3 = float4(tmp, tmp, tmp, tmp);\n"
		"	tmp = exp( -16.0 / wid / wid);\n"
		"	float4 c4 = float4(tmp, tmp, tmp, tmp);\n"
		"	float4 norm = 1.0 / (float4(1.0, 1.0, 1.0, 1.0) + "
		"			float4(2.0, 2.0, 2.0, 2.0) * (c1 + c2 + c3 + c4));\n"

		"	float onex = factor.x * (1.0 / size_texture.x);\n"

		"	sum += tex2D(s0, texCoord + float2(-4.0 * onex, 0.0)) * c4;\n"
		"	sum += tex2D(s0, texCoord + float2(-3.0 * onex, 0.0)) * c3;\n"
		"	sum += tex2D(s0, texCoord + float2(-2.0 * onex, 0.0)) * c2;\n"
		"	sum += tex2D(s0, texCoord + float2(-1.0 * onex, 0.0)) * c1;\n"
		"	sum += tex2D(s0, texCoord);\n"
		"	sum += tex2D(s0, texCoord + float2(+1.0 * onex, 0.0)) * c1;\n"
		"	sum += tex2D(s0, texCoord + float2(+2.0 * onex, 0.0)) * c2;\n"
		"	sum += tex2D(s0, texCoord + float2(+3.0 * onex, 0.0)) * c3;\n"
		"	sum += tex2D(s0, texCoord + float2(+4.0 * onex, 0.0)) * c4;\n"

		"	float y = mul(rgb2yuv, tex2D(s0, texCoord).rgb).x;\n"
		"	float2 uv = mul(rgb2yuv, float3(sum.rgb * norm.rgb)).yz;\n"

		"	float4 scr = float4(mul(yuv2rgb, float3(y, uv)), 1.0);\n"

		"	return scr;\n"
		"}"
	},

};
#undef _SHADERS_CODE_
#endif
