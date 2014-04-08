/*****************************************************************************************/
/* Scanline                                                                              */
/*****************************************************************************************/
{
	// vertex shader
	"uniform vec2 size_screen_emu;\n"
	"uniform vec2 size_video_mode;\n"
	"uniform vec2 size_texture;\n"

	"varying vec4 v_texCoord;\n"
	"varying vec2 omega;\n"

	"void main() {\n"
	"	gl_FrontColor = gl_Color;\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"

	"	omega = vec2(3.1415, 2.0 * 3.1415 * size_texture.y);\n"

	//"	omega = vec2(3.1415 * size_video_mode.x * size_texture.x / size_screen_emu.x,"
	//"			2.0 * 3.1415 * size_texture.y);\n"

	"	v_texCoord = gl_MultiTexCoord0;\n"
	"}",
	// fragment shader
	"uniform vec2 size_texture;\n"

	"uniform sampler2D texture_scr;\n"

	"varying vec4 v_texCoord;\n"
	"varying vec2 omega;\n"

	"const float base_brightness = 0.95;\n"
	//"const vec2 sine_comp = vec2(0.05, 0.15);\n"
	"const vec2 sine_comp = vec2(0.15, 0.25);\n"

	"void main() {\n"
	"	vec4 c11 = texture2D(texture_scr, v_texCoord.xy);\n"

	"	vec4 scanline = c11 * (base_brightness + dot(sine_comp * sin(v_texCoord.xy * omega),"
	"			vec2(1.0)));\n"

	"	vec4 scr = clamp(scanline, 0.0, 1.0);\n"

	"	gl_FragColor = scr * gl_Color;\n"
	"}"
},
