/*****************************************************************************************/
/* Waterpaint                                                                                */
/*****************************************************************************************/
{
	// vertex shader
	"uniform vec2 size_texture;\n"
	"varying vec4 v_texCoord[5];\n"

	"void main() {\n"
	"	float x = 0.5 * (1.0 / size_texture.x);\n"
	"	float y = 0.5 * (1.0 / size_texture.y);\n"
	"	vec2 dg1 = vec2( x, y);\n"
	"	vec2 dg2 = vec2(-x, y);\n"
	"	vec2 dx = vec2(x, 0.0);\n"
	"	vec2 dy = vec2(0.0, y);\n"

	"	gl_FrontColor = gl_Color;\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"

	"	v_texCoord[0] = gl_MultiTexCoord0;\n"
	"	v_texCoord[1].xy = v_texCoord[0].xy - dg1;\n"
	"	v_texCoord[1].zw = v_texCoord[0].xy - dy;\n"
	"	v_texCoord[2].xy = v_texCoord[0].xy - dg2;\n"
	"	v_texCoord[2].zw = v_texCoord[0].xy + dx;\n"
	"	v_texCoord[3].xy = v_texCoord[0].xy + dg1;\n"
	"	v_texCoord[3].zw = v_texCoord[0].xy + dy;\n"
	"	v_texCoord[4].xy = v_texCoord[0].xy + dg2;\n"
	"	v_texCoord[4].zw = v_texCoord[0].xy - dx;\n"
	"}",
	// fragment shader
	"uniform vec2 size_texture;\n"
	"uniform sampler2D texture_scr;\n"
	"varying vec4 v_texCoord[5];\n"

	"vec4 compress(vec4 in_color, float threshold, float ratio) {\n"
	"	vec4 diff = in_color - vec4(threshold);\n"
	"	diff = clamp(diff, 0.0, 100.0);\n"
	"	return in_color - (diff * (1.0 - 1.0/ratio));\n"
	"}\n"

	"void main () {\n"
	"	vec3 c00 = texture2D(texture_scr, v_texCoord[1].xy).xyz;\n"
	"	vec3 c01 = texture2D(texture_scr, v_texCoord[4].zw).xyz;\n"
	"	vec3 c02 = texture2D(texture_scr, v_texCoord[4].xy).xyz;\n"
	"	vec3 c10 = texture2D(texture_scr, v_texCoord[1].zw).xyz;\n"
	"	vec3 c11 = texture2D(texture_scr, v_texCoord[0].xy).xyz;\n"
	"	vec3 c12 = texture2D(texture_scr, v_texCoord[3].zw).xyz;\n"
	"	vec3 c20 = texture2D(texture_scr, v_texCoord[2].xy).xyz;\n"
	"	vec3 c21 = texture2D(texture_scr, v_texCoord[2].zw).xyz;\n"
	"	vec3 c22 = texture2D(texture_scr, v_texCoord[3].xy).xyz;\n"

	"	vec2 tex = v_texCoord[0].xy;\n"
	"	vec2 texsize = size_texture;\n"

	"	vec3 first = mix(c00, c20, fract(tex.x * texsize.x + 0.5));\n"
	"	vec3 second = mix(c02, c22, fract(tex.x * texsize.x + 0.5));\n"

	"	vec3 mid_horiz = mix(c01, c21, fract(tex.x * texsize.x + 0.5));\n"
	"	vec3 mid_vert = mix(c10, c12, fract(tex.y * texsize.y + 0.5));\n"

	"	vec3 res = mix(first, second, fract(tex.y * texsize.y + 0.5));\n"
	"	vec4 final = vec4(0.26 * (res + mid_horiz + mid_vert) +"
	"					3.5 * abs(res - mix(mid_horiz, mid_vert, 0.5)), 1.0);\n"

	"	vec4 scr = compress(final, 0.8, 5.0);\n"

	"	gl_FragColor = scr * gl_Color;\n"
	"}"
},
