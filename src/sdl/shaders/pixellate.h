/*****************************************************************************************/
/* Pixellate                                                                             */
/*****************************************************************************************/
{
	// vertex shader
	"varying vec4 v_texCoord;\n"
	"void main() {\n"
	"	gl_FrontColor = gl_Color;\n"
	"	gl_Position = ftransform();\n"
	"	v_texCoord = gl_MultiTexCoord0;\n"
	"}",
	// fragment shader
	"uniform vec2 size_texture;\n"
	"uniform sampler2D texture_scr;\n"
	"varying vec4 v_texCoord;\n"

	"#define round(x) floor( (x) + 0.5 )\n"

	"void main() {\n"
	"	vec2 texelSize = (1.0 / size_texture);\n"
	"	vec2 texCoord = v_texCoord.xy;\n"

	"	vec2 range = vec2(abs(dFdx(texCoord.x)), abs(dFdy(texCoord.y)));\n"
	"	range = (range / 2.0) * 0.999;\n"

	"	float left   = texCoord.x - range.x;\n"
	"	float top    = texCoord.y + range.y;\n"
	"	float right  = texCoord.x + range.x;\n"
	"	float bottom = texCoord.y - range.y;\n"

	"	vec4 topLeftColor     = texture2D(texture_scr, vec2(left, top));\n"
	"	vec4 bottomRightColor = texture2D(texture_scr, vec2(right, bottom));\n"
	"	vec4 bottomLeftColor  = texture2D(texture_scr, vec2(left, bottom));\n"
	"	vec4 topRightColor    = texture2D(texture_scr, vec2(right, top));\n"

	"	vec2 border = clamp(round(texCoord / texelSize) * texelSize,"
	"			vec2(left, bottom), vec2(right, top));\n"

	"	float totalArea = 4.0 * range.x * range.y;\n"

	"	vec4 scr;\n"
	"	scr  = ((border.x - left) * (top - border.y) / totalArea) * topLeftColor;\n"
	"	scr += ((right - border.x) * (border.y - bottom) / totalArea) * bottomRightColor;\n"
	"	scr += ((border.x - left) * (border.y - bottom) / totalArea) * bottomLeftColor;\n"
	"	scr += ((right - border.x) * (top - border.y) / totalArea) * topRightColor;\n"

	"	gl_FragColor = scr * gl_Color;\n"
	"}"
},
