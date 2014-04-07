/*****************************************************************************************/
/* don't BLOOM                                                                           */
/*****************************************************************************************/
{
	// vertex shader
	"uniform vec2 size_video_mode;\n"
	"uniform vec2 size_texture;\n"
	"uniform float param;\n"
	"uniform float pixel_aspect_ratio;\n"

	"varying vec2 c00;\n"
	"varying vec2 c10;\n"
	"varying vec2 c20;\n"
	"varying vec2 c01;\n"
	"varying vec2 c11;\n"
	"varying vec2 c21;\n"
	"varying vec2 c02;\n"
	"varying vec2 c12;\n"
	"varying vec2 c22;\n"
	"varying vec2 pixel_no;\n"

	"void main() {\n"
	"	gl_FrontColor = gl_Color;\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"

	"	float dx;\n"
	"	float dy;\n"

	"	if (param == 0.0) {\n"
	"		dx = (1.0 / size_video_mode.x);\n"
	"		dy = (1.0 / size_video_mode.y);\n"
	"		pixel_no = gl_MultiTexCoord0.xy * vec2(size_texture.x * pixel_aspect_ratio, size_texture.y);\n"
	"	} else {\n"
	"		dx = (1.0 / (size_texture.x * pixel_aspect_ratio));\n"
	"		dy = (1.0 / size_texture.y);\n"
	"       pixel_no = gl_MultiTexCoord0.xy;\n"
	"	}\n"

	"	c00 = gl_MultiTexCoord0.xy + vec2(-dx, -dy);\n"
	"	c10 = gl_MultiTexCoord0.xy + vec2(  0, -dy);\n"
	"	c20 = gl_MultiTexCoord0.xy + vec2( dx, -dy);\n"
	"	c01 = gl_MultiTexCoord0.xy + vec2(-dx,   0);\n"
	"	c11 = gl_MultiTexCoord0.xy + vec2(  0,   0);\n"
	"	c21 = gl_MultiTexCoord0.xy + vec2( dx,   0);\n"
	"	c02 = gl_MultiTexCoord0.xy + vec2(-dx,  dy);\n"
	"	c12 = gl_MultiTexCoord0.xy + vec2(  0,  dy);\n"
	"	c22 = gl_MultiTexCoord0.xy + vec2( dx,  dy);\n"
	"}",
	// fragment shader
	"uniform float param;\n"

	"uniform sampler2D texture_scr;\n"

	"varying vec2 c00;\n"
	"varying vec2 c10;\n"
	"varying vec2 c20;\n"
	"varying vec2 c01;\n"
	"varying vec2 c11;\n"
	"varying vec2 c21;\n"
	"varying vec2 c02;\n"
	"varying vec2 c12;\n"
	"varying vec2 c22;\n"
	"varying vec2 pixel_no;\n"

	"float gamma;\n"
	"float shine;\n"
	"float blend;\n"
	"float factor_delta;\n"

	"float dist(vec2 coord, vec2 source) {\n"
	"	vec2 delta = coord - source;\n"
	"	return sqrt(dot(delta, delta));\n"
	"}\n"

	"float color_bloom(vec3 color) {\n"
	"	const vec3 gray_coeff = vec3(0.30, 0.59, 0.11);\n"
	"	float bright = dot(color, gray_coeff);\n"
	"	return mix(1.0 + shine, 1.0 - shine, bright);\n"
	"}\n"

	"vec3 lookup(float offset_x, float offset_y, vec2 coord) {\n"
	"	vec2 offset = vec2(offset_x, offset_y);\n"
	"	vec3 color = texture2D(texture_scr, coord).rgb;\n"
	//"	float delta = dist(fract(pixel_no), offset + vec2(0.5));\n"
	"	float delta = dist(fract(pixel_no), offset + vec2(factor_delta));\n"
	"	return color * exp(-gamma * delta * color_bloom(color));\n"
	"}\n"

	"void main() {\n"
	"	if (param == 0.0) {\n"
	"		gamma = 2.4;\n"
	"		shine = 0.05;\n"
	"		blend = 0.65;\n"
	"		factor_delta = 0.4;\n"
	"	} else {\n"
	"		gamma = 1.4;\n"
	"		shine = 0.25;\n"
	"		blend = 0.10;\n"
	"		factor_delta = 0.10;\n"
	"	}\n"

	"	vec3 mid_color = lookup(0.0, 0.0, c11);\n"
	"	vec3 color = vec3(0.0);\n"
	"	color += lookup(-1.0, -1.0, c00);\n"
	"	color += lookup( 0.0, -1.0, c10);\n"
	"	color += lookup( 1.0, -1.0, c20);\n"
	"	color += lookup(-1.0,  0.0, c01);\n"
	"	color += mid_color;\n"
	"	color += lookup( 1.0,  0.0, c21);\n"
	"	color += lookup(-1.0,  1.0, c02);\n"
	"	color += lookup( 0.0,  1.0, c12);\n"
	"	color += lookup( 1.0,  1.0, c22);\n"
	"	vec3 out_color = mix(1.2 * mid_color, color, blend);\n"

	"	vec4 scr = vec4(out_color, 1.0);\n"

	"	gl_FragColor = scr * gl_Color;\n"
	"}"
},
