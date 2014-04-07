/*****************************************************************************************/
/* Quilaz                                                                                */
/*****************************************************************************************/
{
	// vertex shader
	NULL,
	// fragment shader
	"uniform vec2 size_texture;\n"
	"uniform sampler2D texture_scr;\n"

	"vec4 getTexel(vec2 p) {\n"
	"	p = p * size_texture + vec2(0.5);\n"

	"	vec2 i = floor(p);\n"
	"	vec2 f = p - i;\n"
	"	f = f * f * f * (f * (f * 6.0 - vec2(15.0)) + vec2(10.0));\n"
	"	p = i + f;\n"

	"	p = (p - vec2(0.5)) / size_texture;\n"
	"	return texture2D(texture_scr, p);\n"
	"}\n"

	"void main() {\n"
	"	vec4 scr = getTexel(gl_TexCoord[0].xy);\n"
	"	gl_FragColor = scr * gl_Color;\n"
	"}"
},
