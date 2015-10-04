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

	"	float sharpness = 0.8;\n"

	"	if (full_interpolation == 1.0) {\n"
	"		sharpness = 0.0;\n"
	"	}\n"

	"	float2 scale = (size_video_mode / float2(pixel_aspect_ratio, 1.0)) / size_screen_emu;\n"
	"	float2 interp = (scale - lerp(scale, 1.0, sharpness)) / (scale * 2.0);\n"
	"	float2 pnt = texCoord.xy;\n"

	"	interp = saturate(interp);\n"
	"	pnt = (pnt * size_texture) + 0.5;\n"

	"	float2 i = floor(pnt);\n"
	"	float2 f = pnt - i;\n"

	"	f = (f - interp) / (1.0 - interp * 2.0);\n"
	"	f = saturate(f);\n"

	"	pnt = i + f;\n"
	"	pnt = (pnt - 0.5) / size_texture;\n"
