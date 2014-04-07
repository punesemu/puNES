/*****************************************************************************************/
/* NTSC3                                                                                 */
/*****************************************************************************************/
{
	// vertex shader
	NULL,
	// fragment shader
	"#version 120\n"

	"uniform sampler2D texture_scr;\n"
	"uniform vec2 size_texture;\n"
	"uniform vec2 size_screen_emu;\n"
	"uniform float frame_counter;\n"

	"#define TEX2D(c) texture2D(texture_scr,(c))\n"

	"#define PI 3.14159265\n"

	"void main() {\n"
	"	vec2 xy = gl_TexCoord[0].st;\n"

	"	vec2 xyp = xy * size_texture * 4.0 * PI / 3.0;\n"
	"	xyp.y = xyp.y / 2.0 + 2.0 * PI / 3.0 * mod(frame_counter,2);\n"

	"	vec4 rgb = TEX2D(xy);\n"

	"	mat3x3 rgb2yuv = mat3x3(0.299,-0.14713, 0.615,"
	"			0.587,-0.28886,-0.51499,"
	"			0.114, 0.436 ,-0.10001);\n"

	"	vec3 yuv;\n"
	"	yuv = rgb2yuv * rgb.rgb;\n"

	"	float dx = PI/3.0;\n"
	"	xyp.x = xyp.x * size_screen_emu.x/256.0;\n"
	"	float c0 = yuv.x + yuv.y * sin(xyp.x+xyp.y) + yuv.z*cos(xyp.x+xyp.y);\n"
	"	float c1 = yuv.x + yuv.y * sin(xyp.x+xyp.y+dx) + yuv.z * cos(xyp.x+xyp.y+dx);\n"
	"	rgb = TEX2D(xy + vec2(1.0/size_texture.x * size_screen_emu.x / 512.0, 0.0));\n"
	"	yuv = rgb2yuv * rgb.rgb;\n"
	"	float c2 = yuv.x + yuv.y * sin(xyp.x+xyp.y+2.0*dx) + yuv.z * cos(xyp.x+xyp.y+2.0*dx);\n"
	"	float c3 = yuv.x + yuv.y * sin(xyp.x+xyp.y+3.0*dx) + yuv.z * cos(xyp.x+xyp.y+3.0*dx);\n"

	"	vec4 scr = (vec4(c0,c1,c2,c3)+0.65)/2.3;\n"

	"	gl_FragColor = scr * gl_Color;\n"

	"}"
},
