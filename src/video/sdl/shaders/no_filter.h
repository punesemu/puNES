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
/* NO_FILTER                                                                             */
/*****************************************************************************************/
{
	// vertex shader
	"varying vec4 v_texCoord;\n"

	"void main(void) {\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"	gl_FrontColor = gl_Color;\n"
	"	v_texCoord = gl_MultiTexCoord0;\n"
	"}",
	// fragment shader
	"uniform vec2 size_screen_emu;\n"
	"uniform vec2 size_video_mode;\n"
	"uniform vec2 size_texture;\n"
	"uniform float pixel_aspect_ratio;\n"
	"uniform float full_interpolation;\n"

	"uniform sampler2D texture_scr;\n"

	"varying vec4 v_texCoord;\n"

	"void main(void) {\n"

#include "interpolation.h"

	"	vec4 scr = texture2D(texture_scr, pnt);\n"
	"	gl_FragColor = scr * gl_Color;\n"
	"}"
},
