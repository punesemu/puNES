/*****************************************************************************************/
/* NTSC                                                                                  */
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
	"#version 120\n"

	"uniform sampler2D texture_scr;\n"
	"uniform vec2 size_texture;\n"

	"varying vec4 v_texCoord;\n"

	"void main() {\n"
	"	mat3x3 rgb2yuv = mat3x3("
	"		0.299,-0.14713, 0.615  ,"
	"		0.587,-0.28886,-0.51499,"
	"		0.114, 0.436  ,-0.10001 "
	");\n"
	"	mat3x3 yuv2rgb = mat3x3("
	"		1.0    , 1.0    , 1.0    ,"
	"		0.0    ,-0.39465, 2.03211,"
	"		1.13983,-0.58060, 0.0     "
	");\n"

	"	vec4 sum = vec4(0.0);\n"

	"	float wid = 3.0;\n"
	"	vec4 c1 = vec4(exp(-1.0 / wid / wid));\n"
	"	vec4 c2 = vec4(exp(-4.0 / wid / wid));\n"
	"	vec4 c3 = vec4(exp(-9.0 / wid / wid));\n"
	"	vec4 c4 = vec4(exp(-16.0 / wid / wid));\n"
	"	vec4 norm = 1.0 / (vec4(1.0) + vec4(2.0) * (c1 + c2 + c3 + c4));\n"

	"	float onex = (1.0 / size_texture.x);\n"

	"	sum += texture2D(texture_scr, v_texCoord.xy + vec2(-4.0 * onex, 0.0)) * c4;\n"
	"	sum += texture2D(texture_scr, v_texCoord.xy + vec2(-3.0 * onex, 0.0)) * c3;\n"
	"	sum += texture2D(texture_scr, v_texCoord.xy + vec2(-2.0 * onex, 0.0)) * c2;\n"
	"	sum += texture2D(texture_scr, v_texCoord.xy + vec2(-1.0 * onex, 0.0)) * c1;\n"
	"	sum += texture2D(texture_scr, v_texCoord.xy);\n"
	"	sum += texture2D(texture_scr, v_texCoord.xy + vec2(+1.0 * onex, 0.0)) * c1;\n"
	"	sum += texture2D(texture_scr, v_texCoord.xy + vec2(+2.0 * onex, 0.0)) * c2;\n"
	"	sum += texture2D(texture_scr, v_texCoord.xy + vec2(+3.0 * onex, 0.0)) * c3;\n"
	"	sum += texture2D(texture_scr, v_texCoord.xy + vec2(+4.0 * onex, 0.0)) * c4;\n"

	"	float y = (rgb2yuv * texture2D(texture_scr, v_texCoord.xy).rgb).x;\n"
	"	vec2 uv = (rgb2yuv * (sum.rgb * norm.rgb)).yz;\n"

	"	vec4 scr = vec4(yuv2rgb * vec3(y, uv), 0.0);\n"

	"	gl_FragColor = scr * gl_Color;\n"
	"}"
},
