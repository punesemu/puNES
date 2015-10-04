/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

/*****************************************************************************************/
/* don't BLOOM                                                                           */
/*****************************************************************************************/
{
	// vertex shader
	"varying vec4 v_texCoord;\n"

	"void main() {\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"	gl_FrontColor = gl_Color;\n"
	"	v_texCoord = gl_MultiTexCoord0;\n"
	"}",
	// fragment shader
	"uniform vec2 size_texture;\n"
	"uniform vec2 size_screen_emu;\n"
	"uniform vec2 size_video_mode;\n"
	"uniform float pixel_aspect_ratio;\n"
	"uniform float param;\n"
	"uniform float full_interpolation;\n"

	"uniform sampler2D texture_scr;\n"

	"varying vec4 v_texCoord;\n"

	"vec2 c00;\n"
	"vec2 c10;\n"
	"vec2 c20;\n"
	"vec2 c01;\n"
	"vec2 c11;\n"
	"vec2 c21;\n"
	"vec2 c02;\n"
	"vec2 c12;\n"
	"vec2 c22;\n"
	"vec2 pixel_no;\n"

	"float gamma;\n"
	"float shine;\n"
	"float blend;\n"
	"float factor_delta;\n"
	"float dx;\n"
	"float dy;\n"

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
	"	float delta = dist(fract(pixel_no), offset + vec2(factor_delta));\n"
	"	return color * exp(-gamma * delta * color_bloom(color));\n"
	"}\n"

	"void main() {\n"
		"	if (param == 0.0) {\n"
	"		dx = (1.0 / (size_video_mode.x / pixel_aspect_ratio));\n"
	"		dy = (1.0 / (size_video_mode.y));\n"
	//"		pixel_no = v_texCoord.xy * ((256.0 * vec2(pixel_aspect_ratio, 1.0)) - 0.1);\n"
	"		pixel_no = v_texCoord.xy * (256.0 - 0.1);\n"
	"		gamma = 2.4;\n"
	"		shine = 0.05;\n"
	"		blend = 0.65;\n"
	"		factor_delta = 0.5;\n"
	"	} else if (param == 1.0) {\n"
	"		dx = dy = 1.0 / 256.0;\n"
	"		pixel_no = v_texCoord.xy;\n"
	"		gamma = 1.4;\n"
	"		shine = 0.25;\n"
	"		blend = 0.10;\n"
	"		factor_delta = 0.10;\n"
	"	} else if (param == 2.0) {\n"
	"		dx = dy = 1.0 / 256.0;\n"
	"		pixel_no = v_texCoord.xy * (256.0 / vec2(0.0, 1.0));\n"
	"		gamma = 1.0;\n"
	"		shine = 0.25;\n"
	"		blend = 0.10;\n"
	"		factor_delta = 0.5;\n"
	"	} else {\n"
	"		dx = dy = 1.0 / 256.0;\n"
	"		pixel_no = v_texCoord.xy * (256.0 / vec2(1.0, 0.0));\n"
	"		gamma = 1.0;\n"
	"		shine = 0.25;\n"
	"		blend = 0.10;\n"
	"		factor_delta = 0.5;\n"
	"	}\n"

#include "interpolation.h"

	"	c00 = pnt + vec2(-dx, -dy);\n"
	"	c10 = pnt + vec2(  0, -dy);\n"
	"	c20 = pnt + vec2( dx, -dy);\n"
	"	c01 = pnt + vec2(-dx,   0);\n"
	"	c11 = pnt + vec2(  0,   0);\n"
	"	c21 = pnt + vec2( dx,   0);\n"
	"	c02 = pnt + vec2(-dx,  dy);\n"
	"	c12 = pnt + vec2(  0,  dy);\n"
	"	c22 = pnt + vec2( dx,  dy);\n"

	"	vec3 mid_color = lookup(0.0, 0.0, c11);\n"
	"	vec3 color = vec3(0.0);\n"
	"	color += lookup(-1.0, -1.0, c00);\n"
	"	color += lookup( 0.0, -1.0, c10);\n"
	"	color += lookup( 1.0, -1.0, c20);\n"
	"	color += lookup(-1.0,  0.0, c01);\n"
	"	color += mid_color;\n"
	"	color += lookup( 1.0,  0.0, c21);\n"
	"	color += lookup(-1.0,  1.0, c02);\n"
	"	color += lookup( 0.0,  1.0, c12);\n"
	"	color += lookup( 1.0,  1.0, c22);\n"
	"	vec3 out_color = mix(1.2 * mid_color, color, blend);\n"

	"	vec4 scr = vec4(out_color, 1.0);\n"

	"	gl_FragColor = scr * gl_Color;\n"
	"}"
},
