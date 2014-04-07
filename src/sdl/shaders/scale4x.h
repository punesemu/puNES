/*****************************************************************************************/
/* SCALE4X                                                                               */
/*****************************************************************************************/
{
	// vertex shader
	"uniform vec2 size_texture;\n"

	"varying vec4 v_texCoord[7];\n"

	"void main() {\n"
	"	gl_FrontColor = gl_Color;\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"

	"	float x = 0.5 * (1.0 / size_texture.x);\n"
	"	float y = 0.5 * (1.0 / size_texture.y);\n"

	"	vec2 dg1 = vec2( x, y);\n"
	"	vec2 dg2 = vec2(-x, y);\n"
	"	vec2 sd1 = dg1 * 0.5;\n"
	"	vec2 sd2 = dg2 * 0.5;\n"

	"	v_texCoord[0] = gl_MultiTexCoord0;\n"
	"	v_texCoord[1].xy = v_texCoord[0].xy - sd1;\n"
	"	v_texCoord[2].xy = v_texCoord[0].xy - sd2;\n"
	"	v_texCoord[3].xy = v_texCoord[0].xy + sd1;\n"
	"	v_texCoord[4].xy = v_texCoord[0].xy + sd2;\n"
	"	v_texCoord[5].xy = v_texCoord[0].xy - dg1;\n"
	"	v_texCoord[6].xy = v_texCoord[0].xy + dg1;\n"
	"	v_texCoord[5].zw = v_texCoord[0].xy - dg2;\n"
	"	v_texCoord[6].zw = v_texCoord[0].xy + dg2;\n"
	"}",
	// fragment shader
	"uniform sampler2D texture_scr;\n"

	"varying vec4 v_texCoord[7];\n"

	"void main() {\n"
	"	vec3 c  = texture2D(texture_scr, v_texCoord[0].xy).xyz;\n"
	"	vec3 i1 = texture2D(texture_scr, v_texCoord[1].xy).xyz;\n"
	"	vec3 i2 = texture2D(texture_scr, v_texCoord[2].xy).xyz;\n"
	"	vec3 i3 = texture2D(texture_scr, v_texCoord[3].xy).xyz;\n"
	"	vec3 i4 = texture2D(texture_scr, v_texCoord[4].xy).xyz;\n"
	"	vec3 o1 = texture2D(texture_scr, v_texCoord[5].xy).xyz;\n"
	"	vec3 o3 = texture2D(texture_scr, v_texCoord[6].xy).xyz;\n"
	"	vec3 o2 = texture2D(texture_scr, v_texCoord[5].zw).xyz;\n"
	"	vec3 o4 = texture2D(texture_scr, v_texCoord[6].zw).xyz;\n"

	"	vec3 dt = vec3(1.0);\n"

	"	float ko1 = dot(abs(o1 - c), dt);\n"
	"	float ko2 = dot(abs(o2 - c), dt);\n"
	"	float ko3 = dot(abs(o3 - c), dt);\n"
	"	float ko4 = dot(abs(o4 - c), dt);\n"

	"	float k1=min(dot(abs(i1-i3),dt),dot(abs(o1-o3),dt));\n"
	"	float k2=min(dot(abs(i2-i4),dt),dot(abs(o2-o4),dt));\n"

	"	float w1 = k2; if (ko3 < ko1) { w1 = 0.0; }\n"
	"	float w2 = k1; if (ko4 < ko2) { w2 = 0.0; }\n"
	"	float w3 = k2; if (ko1 < ko3) { w3 = 0.0; }\n"
	"	float w4 = k1; if (ko2 < ko4) { w4 = 0.0; }\n"

	"	vec4 scr = vec4(((w1 * o1) + (w2 * o2) + (w3 * o3) + (w4 * o4) + (0.0001 * c)) /"
	"			(w1 + w2 + w3 + w4 + 0.0001), 1.0);\n"

	"	gl_FragColor = scr * gl_Color;\n"
	"}"
},
