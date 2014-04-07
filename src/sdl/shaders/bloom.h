/*****************************************************************************************/
/* BLOOM                                                                                 */
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

	"#define glarebasesize 0.42\n"
	"#define power 0.65 // 0.50 is good\n"

	"void main() {\n"
	"	vec4 sum = vec4(0.0);\n"
	"	vec4 bum = vec4(0.0);\n"
	"	vec2 texcoord = vec2(v_texCoord);\n"
	"	int j;\n"
	"	int i;\n"

	"	vec2 glaresize = vec2(glarebasesize) / size_texture;\n"

	"	for(i = -2; i < 5; i++) {\n"
	"		for (j = -1; j < 1; j++) {\n"
	"			sum += texture2D(texture_scr, texcoord + vec2(-i, j)*glaresize) * power;\n"
	"			bum += texture2D(texture_scr, texcoord + vec2(j, i)*glaresize) * power;\n"
	"		}\n"
	"	}\n"

	"	vec4 scr;\n"
	"	if (texture2D(texture_scr, texcoord).r < 2.0) {\n"
	"		scr = sum*sum*sum*0.001+bum*bum*bum*0.0080 + texture2D(texture_scr, texcoord);\n"
	"	}\n"

	"	gl_FragColor = scr * gl_Color;\n"
	"}"
},
