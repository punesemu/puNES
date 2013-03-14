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
	//SHADER_SCALE2X,
	//SHADER_SCALE3X,
	//SHADER_SCALE4X,
	//SHADER_HQ2X,
	//SHADER_HQ4X,
	//SHADER_4xBR,
	SHADER_PIXELLATE,
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
	//SHADER_NTSC,
	//SHADER_NTSC2,
	//SHADER_NTSC3,
	//SHADER_TOON,
	SHADER_TOTAL,
	SHADER_NONE = 255
};

typedef struct {
	//GLuint data;
	//GLenum format;
	//GLenum type;
	//GLint format_internal;

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

	struct {
		struct {
			UINT input;
			UINT output;
			UINT texture;
		} size;
		struct {
			UINT scr;
			UINT txt;
		} texture;
		UINT frame_counter;
	} loc;
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
		"VsOutput Vs(float3 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		//"	output.Position = mul(float4(position, 1), WorldViewProjection);\n"
		"	output.Position = float4(position, 1.0);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"texture texture_scr;\n"
		"sampler s0 = sampler_state { Texture = <texture_scr>; };\n"
		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float4 scr = tex2D(s0, texCoord) * 2;\n"
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
		"VsOutput Vs(float3 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = float4(position, 1.0);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_texture;\n"
		"texture texture_scr;\n"
		"sampler s0 = sampler_state { Texture = <texture_scr>; };\n"
		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float4 scr = tex2D(s0, texCoord);\n"
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
		"VsOutput Vs(float3 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = float4(position, 1.0);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_texture;\n"
		"texture texture_scr;\n"
		"sampler s0 = sampler_state { Texture = <texture_scr>; };\n"

		"#define round(x) floor((x) + 0.5)\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float2 texelSize = 1.0 / size_texture;\n"

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

		"	float2 border = clamp(round(texCoord / texelSize) * texelSize,"
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
		"VsOutput Vs(float3 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = float4(position, 1.0);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_texture;\n"
		"texture texture_scr;\n"
		"sampler s0 = sampler_state { Texture = <texture_scr>; };\n"

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

		"	float2 one_x = float2(1.0 / (3.0 * size_texture.x), 0.0);\n"

		"	float3 color = tex2D(s0, texCoord - 0.0 * one_x).rgb;\n"
		"	float3 color_prev = tex2D(s0, texCoord - 1.0 * one_x).rgb;\n"
		"	float3 color_prev_prev = tex2D(s0, texCoord - 2.0 * one_x).rgb;\n"

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
		"VsOutput Vs(float3 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = float4(position, 1.0);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_input;\n"
		"float2 size_output;\n"
		"float2 size_texture;\n"
		"texture texture_scr;\n"
		"sampler s0 = sampler_state { Texture = <texture_scr>; };\n"

		"static const float base_brightness = 0.95;\n"
		"static const float2 sine_comp = float2(0.05, 0.15);\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float2 omega = float2(3.1415 * size_output.x * size_texture.x / size_input.x,"
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
		"VsOutput Vs(float3 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
		"	VsOutput output;\n"
		"	output.Position = float4(position, 1.0);\n"
		"	output.TexCoord = texCoord;\n"
		"	return output;\n"
		"}",
		// pixel shader
		"float2 size_texture;\n"
		"texture texture_scr;\n"
		"sampler s0 = sampler_state { Texture = <texture_scr>; };\n"

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

		"float2 frac(float2 x) {\n"
		"	return x - floor(x);\n"
		"}\n"

		"float3 lookup(float offset_x, float offset_y, float2 coord, float2 pix) {\n"
		"	float2 offset = float2(offset_x, offset_y);\n"
		"	float3 color = tex2D(s0, coord).rgb;\n"
		/*"	float delta = dist(frac(pix), offset + float2(0.5, 0.5));\n"*/
		"	float delta = dist(frac(pix), offset + float2(0.4, 0.4));\n"
		"	return color * exp(-gamma * delta * color_bloom(color));\n"
		"}\n"

		"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
		"	float dx = 1.0 / size_texture.x;\n"
		"	float dy = 1.0 / size_texture.y;\n"

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

		// vertex shader
		/*
		"uniform vec2 size_texture;\n"

		"varying vec2 c00;\n"
		"varying vec2 c10;\n"
		"varying vec2 c20;\n"
		"varying vec2 c01;\n"
		"varying vec2 c11;\n"
		"varying vec2 c21;\n"
		"varying vec2 c02;\n"
		"varying vec2 c12;\n"
		"varying vec2 c22;\n"
		"varying vec2 pixel_no;\n"

		"void main() {\n"
		"	gl_FrontColor = gl_Color;\n"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
		"}",
		// fragment shader
		"uniform sampler2D texture_scr;\n"
		"uniform sampler2D texture_txt;\n"

		"varying vec2 c00;\n"
		"varying vec2 c10;\n"
		"varying vec2 c20;\n"
		"varying vec2 c01;\n"
		"varying vec2 c11;\n"
		"varying vec2 c21;\n"
		"varying vec2 c02;\n"
		"varying vec2 c12;\n"
		"varying vec2 c22;\n"
		"varying vec2 pixel_no;\n"

		"const float gamma = 2.4;\n"
		"const float shine = 0.05;\n"
		"const float blend = 0.65;\n"

		"float dist(vec2 coord, vec2 source) {\n"
		"	vec2 delta = coord - source;\n"
		"	return sqrt(dot(delta, delta));\n"
		"}\n"

		"float color_bloom(vec3 color) {\n"
		"	const vec3 gray_coeff = vec3(0.30, 0.59, 0.11);\n"
		"	float bright = dot(color, gray_coeff);\n"
		"	return mix(1.0 + shine, 1.0 - shine, bright);\n"
		"}\n"

		"vec3 lookup(float offset_x, float offset_y, vec2 coord) {\n"
		"	vec2 offset = vec2(offset_x, offset_y);\n"
		"	vec3 color = texture2D(texture_scr, coord).rgb;\n"
		"	float delta = dist(fract(pixel_no), offset + vec2(0.4));\n"
		"	return color * exp(-gamma * delta * color_bloom(color));\n"
		"}\n"

		"void main() {\n"
		"	vec3 mid_color = lookup(0.0, 0.0, c11);\n"
		"	vec3 color = vec3(0.0);\n"
		"	color += lookup(-1.0, -1.0, c00);\n"
		"	color += lookup( 0.0, -1.0, c10);\n"
		"	color += lookup( 1.0, -1.0, c20);\n"
		"	color += lookup(-1.0, 0.0, c01);\n"
		"	color += mid_color;\n"
		"	color += lookup( 1.0, 0.0, c21);\n"
		"	color += lookup(-1.0, 1.0, c02);\n"
		"	color += lookup( 0.0, 1.0, c12);\n"
		"	color += lookup( 1.0, 1.0, c22);\n"
		"	vec3 out_color = mix(1.2 * mid_color, color, blend);\n"

		"	vec4 scr = vec4(out_color, 1.0);\n"

		"	vec4 txt = texture2D(texture_txt, c11);\n"

		"	gl_FragColor = mix(scr, txt, txt.a) * gl_Color;\n"
		"}"
		*/
	},
};
#undef _SHADERS_CODE_
#endif
