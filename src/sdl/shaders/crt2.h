/*****************************************************************************************/
/* CRT2                                                                                  */
/*****************************************************************************************/
{
	// vertex shader
	NULL,
	// fragment shader
	"uniform vec2 size_texture;\n"
	"uniform sampler2D texture_scr;\n"

	"#define CRTgamma 2.5\n"
	"#define display_gamma 2.2\n"
	"#define TEX2D(c) pow(texture2D(texture_scr,(c)),vec4(CRTgamma))\n"

	"void main() {\n"
	"	vec2 xy = gl_TexCoord[0].st;\n"
	"	float oney = 1.0/size_texture.y;\n"

	"	float wid = 2.0;\n"

	"	float c1 = exp(-1.0/wid/wid);\n"
	"	float c2 = exp(-4.0/wid/wid);\n"
	"	float c3 = exp(-9.0/wid/wid);\n"
	"	float c4 = exp(-16.0/wid/wid);\n"
	"	float norm = 1.0 / (1.0 + 2.0*(c1+c2+c3+c4));\n"

	"	vec4 sum = vec4(0.0);\n"

	"	sum += TEX2D(xy + vec2(0.0, -4.0 * oney)) * vec4(c4);\n"
	"	sum += TEX2D(xy + vec2(0.0, -3.0 * oney)) * vec4(c3);\n"
	"	sum += TEX2D(xy + vec2(0.0, -2.0 * oney)) * vec4(c2);\n"
	"	sum += TEX2D(xy + vec2(0.0, -1.0 * oney)) * vec4(c1);\n"
	"	sum += TEX2D(xy);\n"
	"	sum += TEX2D(xy + vec2(0.0, +1.0 * oney)) * vec4(c1);\n"
	"	sum += TEX2D(xy + vec2(0.0, +2.0 * oney)) * vec4(c2);\n"
	"	sum += TEX2D(xy + vec2(0.0, +3.0 * oney)) * vec4(c3);\n"
	"	sum += TEX2D(xy + vec2(0.0, +4.0 * oney)) * vec4(c4);\n"

	"	vec4 scr = pow(sum*vec4(norm),vec4(1.0/display_gamma));\n"

	"	gl_FragColor = scr * gl_Color;\n"
	"}"
},
