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
	"float pixel_aspect_ratio;\n"
	"float full_interpolation;\n"

	"texture texture_scr;\n"
	"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

	"static const float base_brightness = 0.95;\n"
	"static const float2 sine_comp = float2(0.00, 0.05);\n"

	"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
#include "interpolation.h"
	"	float4 c11 = tex2D(s0, pnt);\n"
	"	float2 omega = float2(3.1415, 2.0 * 3.1415 * 256.0);\n"
	"	float4 scanline = c11 * (base_brightness + dot(sine_comp * sin(texCoord * omega), float2(1.0, 1.0)));\n"
	"	float4 scr = saturate(scanline);\n"
	"	return scr;\n"
	"}"
},
