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
