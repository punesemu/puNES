/*****************************************************************************************/
/* HQ2X                                                                                  */
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
	"uniform sampler2D texture_scr;\n"
	"varying vec4 v_texCoord[5];\n"

	"const float mx = 0.325;      // start smoothing wt.\n"
	"const float k = -0.250;      // wt. decrease factor\n"
	"const float max_w = 0.25;    // max filter weigth\n"
	"const float min_w =-0.05;    // min filter weigth\n"
	"const float lum_add = 0.25;  // effects smoothing\n"

	"void main() {\n"
	"	vec3 c00 = texture2D(texture_scr, v_texCoord[1].xy).xyz;\n"
	"	vec3 c10 = texture2D(texture_scr, v_texCoord[1].zw).xyz;\n"
	"	vec3 c20 = texture2D(texture_scr, v_texCoord[2].xy).xyz;\n"
	"	vec3 c01 = texture2D(texture_scr, v_texCoord[4].zw).xyz;\n"
	"	vec3 c11 = texture2D(texture_scr, v_texCoord[0].xy).xyz;\n"
	"	vec3 c21 = texture2D(texture_scr, v_texCoord[2].zw).xyz;\n"
	"	vec3 c02 = texture2D(texture_scr, v_texCoord[4].xy).xyz;\n"
	"	vec3 c12 = texture2D(texture_scr, v_texCoord[3].zw).xyz;\n"
	"	vec3 c22 = texture2D(texture_scr, v_texCoord[3].xy).xyz;\n"

	"	vec3 dt = vec3(1.0, 1.0, 1.0);\n"

	"	float md1 = dot(abs(c00 - c22), dt);\n"
	"	float md2 = dot(abs(c02 - c20), dt);\n"

	"	float w1 = dot(abs(c22 - c11), dt) * md2;\n"
	"	float w2 = dot(abs(c02 - c11), dt) * md1;\n"
	"	float w3 = dot(abs(c00 - c11), dt) * md2;\n"
	"	float w4 = dot(abs(c20 - c11), dt) * md1;\n"

	"	float t1 = w1 + w3;\n"
	"	float t2 = w2 + w4;\n"
	"	float ww = max(t1, t2) + 0.0001;\n"

	"	c11 = (w1 * c00 + w2 * c20 + w3 * c22 + w4 * c02 + ww * c11) / (t1 + t2 + ww);\n"

	"	float lc1 = k / (0.12 * dot(c10 + c12 + c11, dt) + lum_add);\n"
	"	float lc2 = k / (0.12 * dot(c01 + c21 + c11, dt) + lum_add);\n"

	"	w1 = clamp(lc1 * dot(abs(c11 - c10), dt) + mx, min_w, max_w);\n"
	"	w2 = clamp(lc2 * dot(abs(c11 - c21), dt) + mx, min_w, max_w);\n"
	"	w3 = clamp(lc1 * dot(abs(c11 - c12), dt) + mx, min_w, max_w);\n"
	"	w4 = clamp(lc2 * dot(abs(c11 - c01), dt) + mx, min_w, max_w);\n"

	"	vec4 scr = vec4(w1 * c10 + w2 * c21 + w3 * c12 + w4 * c01 +"
	"			  (1.0 - w1 - w2 - w3 - w4) * c11, 1.0);\n"

	"	gl_FragColor = scr * gl_Color;\n"
	"}"
},
