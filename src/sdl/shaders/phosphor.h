/*****************************************************************************************/
/* Phosphor                                                                              */
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
	"uniform float param;\n"

	"uniform sampler2D texture_scr;\n"

	"varying vec4 v_texCoord;\n"

	"vec3 to_focus(float pixel, float green) {\n"
	"	pixel = mod(pixel + 3.0, 3.0);\n"
	"	if (pixel >= 2.0) {         // Blue\n"
	"		return vec3(pixel - 2.0, 0.0, 3.0 - pixel);\n"
	"	} else if (pixel >= 1.0) {  // Green\n"
	"		return vec3(0.0, green - pixel, pixel - 1.0);\n"
	"	} else {                    // Red\n"
	"		return vec3(1.0 - pixel, pixel, 0.0);\n"
	"	}\n"
	"}\n"

	"void main() {\n"
	"	float green;"
	"	vec2 aspect1;\n"
	"	vec2 aspect2;\n"

	"	if (param == 0.0) {\n"
	"		aspect1 = aspect2 = (size_video_mode * (size_texture / size_screen_emu)) * 0.5;\n"
	"		green = 2.50;\n"
	"	} else {\n"
	"		aspect1 = size_screen_emu * pixel_aspect_ratio;\n"
	"		aspect2 = size_texture * (size_video_mode / size_screen_emu);\n"
	"		green = 2.10;"
	"	}\n"

	"	float y = mod(v_texCoord.y, 1.0);\n"
	"	float intensity = exp(-0.2 * y);\n"

	"	vec2 one_x = vec2(1.0 / (aspect1.x * 3.0), 0.0);\n"

	"	vec3 color = texture2D(texture_scr, v_texCoord.xy).rgb;\n"
	"	vec3 color_prev = texture2D(texture_scr, v_texCoord.xy - one_x).rgb;\n"
	"	vec3 color_prev_prev = texture2D(texture_scr, v_texCoord.xy - (2.0 * one_x)).rgb;\n"

	"	float pixel_x = 3.0 * (v_texCoord.x * aspect2.x);\n"

	"	vec3 focus = to_focus(pixel_x - 0.0, green);\n"
	"	vec3 focus_prev = to_focus(pixel_x - 1.0, green);\n"
	"	vec3 focus_prev_prev = to_focus(pixel_x - 2.0, green);\n"

	"	vec3 result = 0.8 * color * focus +"
	"				  0.6 * color_prev * focus_prev +"
	"				  0.3 * color_prev_prev * focus_prev_prev;\n"

	"	result = 2.3 * pow(result, vec3(1.4));\n"

	"	vec4 scr = vec4(intensity * result, 1.0);\n"

	"	gl_FragColor = scr * gl_Color;\n"
	"}"
},
