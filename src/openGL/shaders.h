/*
 * shaders.h
 *
 *  Created on: 09/mag/2012
 *      Author: fhorse
 */

#ifndef SHADERS_H_
#define SHADERS_H_

#include "sdltext.h"

enum {
	SHADER_NOFILTER,
	SHADER_SCALE2X,
	SHADER_SCALE3X,
	SHADER_SCALE4X,
	SHADER_HQ2X,
	SHADER_HQ4X,
	SHADER_4xBR,
	SHADER_PIXELLATE,
	SHADER_POSPHOR,
	SHADER_SCANLINE,
	SHADER_TOTAL,
	SHADER_NONE = 255
};

#define delete_shader()\
	if (shader.vert) {\
		glDeleteShader(shader.vert);\
	}\
	shader.vert = 0;\
	if (shader.frag) {\
		glDeleteShader(shader.frag);\
	}\
	shader.frag = 0;\
	if (shader.program) {\
		glDeleteProgram(shader.program);\
	}\
	shader.program = 0

typedef struct {
	GLuint data;
	GLenum format;
	GLenum type;
	GLint format_internal;

	GLfloat x;
	GLfloat y;

	GLfloat w;
	GLfloat h;
} _texture;
typedef struct {
	const GLchar *vert;
	const GLchar *frag;
} _shader_routine;
struct {
	GLuint program;
	GLuint vert;
	GLuint frag;

	_shader_routine *routine;

	_texture text;

	GLfloat size_texture[2];
	GLfloat size_output[2];

	struct {
		GLint size_texture;
		GLint size_output;
		GLint texture_scr;
		GLint texture_txt;
	} loc;
} shader;

#endif /* SHADERS_H_ */

#ifdef _SHADERS_CODE_
static _shader_routine shader_routine[SHADER_TOTAL] = {
	/*****************************************************************************************/
	/* NOFILTER                                                                              */
	/*****************************************************************************************/
	{
		// vertex shader
		"varying vec4 v_texCoord;\n"
		"void main(void) {\n"
		"	v_texCoord = gl_MultiTexCoord0;\n"
		"	gl_FrontColor = gl_Color;\n"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
		"}",
		// fragment shader
		"uniform sampler2D texture_scr;\n"
		"uniform sampler2D texture_txt;\n"
		"varying vec4 v_texCoord;\n"
		"void main(void) {\n"
		"	vec4 scr = texture2DProj(texture_scr, v_texCoord);\n"
		"	vec4 txt = texture2DProj(texture_txt, v_texCoord);\n"
		"	gl_FragColor = mix(scr, txt, txt.a) * gl_Color;\n"
		"}"
	},
	/*****************************************************************************************/
	/* SCALE2X                                                                                */
	/*****************************************************************************************/
	{
		// vertex shader
		"uniform vec2 size_texture;\n"
		"varying vec4 v_texCoord[5];\n"
		"void main() {\n"
		"	vec2 dx = vec2((1.0 / size_texture.x), 0);\n"
		"	vec2 dy = vec2(0, (1.0 / size_texture.y));\n"

		"	gl_FrontColor = gl_Color;\n"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"

		"	v_texCoord[0]    = gl_MultiTexCoord0;     // center\n"
		"	v_texCoord[1].xy = v_texCoord[0].xy - dx; // left\n"
		"	v_texCoord[2].xy = v_texCoord[0].xy + dx; // right\n"
		"	v_texCoord[3].xy = v_texCoord[0].xy - dy; // top\n"
		"	v_texCoord[4].xy = v_texCoord[0].xy + dy; // bottom\n"
		"}",
		// fragment shader
		"uniform vec2 size_texture;\n"
		"uniform sampler2D texture_scr;\n"
		"uniform sampler2D texture_txt;\n"
		"varying vec4 v_texCoord[5];\n"
		"void main() {\n"
		"	vec3 E = texture2DProj(texture_scr, v_texCoord[0]).xyz;\n"
		"	vec3 D = texture2DProj(texture_scr, v_texCoord[1]).xyz;\n"
		"	vec3 F = texture2DProj(texture_scr, v_texCoord[2]).xyz;\n"
		"	vec3 H = texture2DProj(texture_scr, v_texCoord[3]).xyz;\n"
		"	vec3 B = texture2DProj(texture_scr, v_texCoord[4]).xyz;\n"

		"	vec4 scr;\n"

		"	if ((D - F) * (H - B) == vec3(0.0)) {\n"
		"		scr.xyz = E;\n"
		"	} else {"
		"		vec2 p = fract(v_texCoord[0].xy * size_texture.xy);\n"
		"		vec3 tmp1 = p.x < 0.5 ? D : F;\n"
		"		vec3 tmp2 = p.y < 0.5 ? H : B;\n"
		"		scr.xyz = ((tmp1 - tmp2) != vec3(0.0)) ? E : tmp1;\n"
		"	}\n"

		"	vec4 txt = texture2DProj(texture_txt, v_texCoord[0]);\n"

		"	gl_FragColor = mix(scr, txt, txt.a) * gl_Color;\n"
		"}",

		/*// fragment shader
		"uniform vec4 size;\n"
		"uniform sampler2D texture_scr;\n"
		"uniform sampler2D texture_txt;\n"
		"varying vec4 v_texCoord[5];\n"
		"void main()\n"
		"{\n"
		"	vec4 colD,colF,colB,colH,col,tmp;\n"
		"	vec4 scr, txt;\n"
		"	vec2 sel;\n"
		"	col  = texture2DProj(texture_scr, v_texCoord[0]);	// central (can be E0-E3)\n"
		"	colD = texture2DProj(texture_scr, v_texCoord[1]);	// D (left)\n"
		"	colF = texture2DProj(texture_scr, v_texCoord[2]);	// F (right)\n"
		"	colB = texture2DProj(texture_scr, v_texCoord[3]);	// B (top)\n"
		"	colH = texture2DProj(texture_scr, v_texCoord[4]);	// H (bottom)\n"
		"	sel  = fract(v_texCoord[0].xy * size.xy);            // where are we (E0-E3)?\n"
		"                                                       // E0 is default\n"
		"	if(sel.y >= 0.5)  {tmp=colB;colB=colH;colH=tmp;}    // E1 (or E3): swap B and H\n"
		"	if(sel.x >= 0.5)  {tmp=colF;colF=colD;colD=tmp;}    // E2 (or E3): swap D and F\n"
		"	if((colB == colD) && (colB != colF) && (colD != colH)) { // do the Scale2x rule\n"
		"		col=colD;\n"
		"	}\n"
		"	scr = col;\n"
		"	txt = texture2DProj(texture_txt, v_texCoord[0]);\n"
		"	gl_FragColor = mix(scr, txt, txt.a);\n"
		"}"*/
	},
	/*****************************************************************************************/
	/* SCALE3X                                                                               */
	/*****************************************************************************************/
	{
		// vertex shader
		"uniform vec2 size_texture;\n"
		"varying vec4 v_texCoord[5];\n"
		"void main() {\n"
		"	vec2 dx = vec2(1.0 / size_texture.x, 0);\n"
		"	vec2 dy = vec2(0, 1.0 / size_texture.y);\n"

		"	gl_FrontColor = gl_Color;\n"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"

		"	v_texCoord[0]    = gl_MultiTexCoord0;         // E\n"
		"	v_texCoord[0].zw = v_texCoord[0].xy - dx - dy; // A\n"
		"	v_texCoord[1].xy = v_texCoord[0].xy - dy;      // B\n"
		"	v_texCoord[1].zw = v_texCoord[0].xy + dx - dy; // C\n"
		"	v_texCoord[2].xy = v_texCoord[0].xy - dx;      // D\n"
		"	v_texCoord[2].zw = v_texCoord[0].xy + dx;      // F\n"
		"	v_texCoord[3].xy = v_texCoord[0].xy - dx + dy; // G\n"
		"	v_texCoord[3].zw = v_texCoord[0].xy + dy;      // H\n"
		"	v_texCoord[4].xy = v_texCoord[0].xy + dx + dy; // I\n"
		"}",
		// fragment shader
		"uniform vec2 size_texture;\n"
		"uniform sampler2D texture_scr;\n"
		"uniform sampler2D texture_txt;\n"
		"varying vec4 v_texCoord[5];\n"
		"void main() {\n"
		"	const vec2 sep = vec2(0.33333, 0.66667); // sufficient precision for HDTV (1920x1080)\n"

		"	vec4 E = texture2D(texture_scr, v_texCoord[0].xy); // E\n"
		"	vec4 A = texture2D(texture_scr, v_texCoord[0].zw); // A\n"
		"	vec4 B = texture2D(texture_scr, v_texCoord[1].xy); // B\n"
		"	vec4 C = texture2D(texture_scr, v_texCoord[1].zw); // C\n"
		"	vec4 D = texture2D(texture_scr, v_texCoord[2].xy); // D\n"
		"	vec4 F = texture2D(texture_scr, v_texCoord[2].zw); // F\n"
		"	vec4 G = texture2D(texture_scr, v_texCoord[3].xy); // G\n"
		"	vec4 H = texture2D(texture_scr, v_texCoord[3].zw); // H\n"
		"	vec4 I = texture2D(texture_scr, v_texCoord[4].xy); // I\n"
		"	vec4 X = vec4(1) - E; // to be sure that ((E != A) == true) in function call\n"
		"	vec4 T;\n"

		"	vec2 sel = fract(v_texCoord[0].xy * size_texture.xy);// where are we (E0-E8)?\n"
		"	// branching is very undesirable, so we make a lot of reassignments\n"
		"	// of original pixels to make sure that rule for E1 pixel will work\n"
		"	// with any other (rotate second matrix and swap some Ex)\n"

		"	// native function call --> x y --> equivalent transpose (to minimize reassignments)\n"
		"	//(E, B, X, D, E, F, H) --> 0 0 --> (E, B, X, D, E, F, H) for E0\n"
		"	//(A, B, C, D, E, F, H) --> 1 0 --> (A, B, C, D, E, F, H) for E1\n"
		"	//(E, F, X, B, E, H, D) --> 2 0 --> (X, F, E, D, E, B, H) for E2\n"
		"	//(G, D, A, H, E, B, F) --> 0 1 --> (A, D, G, B, E, H, F) for E3\n"
		"	//( , E,  ,  , E,  ,  ) --> 1 1 --> ( , E,  ,  , E,  ,  ) for E4\n"
		"	//(C, F, I, B, E, H, D) --> 2 1 --> (I, F, C, H, E, B, D) for E5\n"
		"	//(E, D, X, H, E, B, F) --> 0 2 --> (E, D, X, H, E, F, B) for E6\n"
		"	//(I, H, G, F, E, D, B) --> 1 2 --> (G, H, I, D, E, F, B) for E7\n"
		"	//(E, H, X, F, E, D, B) --> 2 2 --> (X, H, E, D, E, F, B) for E8\n"
		"	if (sel.y < sep.x) {\n"
		"		if (sel.x < sep.x) {\n"
		"			A = E;\n"
		"			C = X;\n"
		"		} else if (sel.x >= sep.y) {\n"
		"			A = X;\n"
		"			C = E;\n"
		"			T = B;\n"
		"			B = F;\n"
		"			F = T;\n"
		"		}\n"
		"	} else if (sel.y < sep.y) {\n"
		"		T = B;\n"
		"		if (sel.x < sep.x) {\n"
		"			B = D;\n"
		"			D = T;\n"
		"			C = G;\n"
		"			T = F;\n"
		"			F = H;\n"
		"		} else if (sel.x < sep.y) {\n"
		"			B = E;\n"
		"		} else {\n"
		"			A = I;\n"
		"			B = F;\n"
		"			F = T;\n"
		"			T = D;\n"
		"			D = H;\n"
		"		}\n"
		"		H = T;\n"
		"	} else {\n"
		"		T = B;\n"
		"		if (sel.x < sep.x) {\n"
		"			A = E;\n"
		"			C = X;\n"
		"			B = D;\n"
		"			D = H;\n"
		"		} else {\n"
		"			if (sel.x < sep.y) {\n"
		"				A = G;\n"
		"				C = I;\n"
		"			} else {\n"
		"				A = X;\n"
		"				C = E;\n"
		"			}\n"
		"			B = H;\n"
		"		}\n"
		"		H = T;\n"
		"	}\n"

		"	vec4 scr = ((D == B && B != F && D != H && E != C) ||"
		"		   (B == F && B != D && F != H && E != A)) ?"
		"			B : E; // Scale3x rule\n"

		"	vec4 txt = texture2D(texture_txt, v_texCoord[0].xy);\n"

		"	gl_FragColor = mix(scr, txt, txt.a) * gl_Color;\n"
		"}"
	},
	/*****************************************************************************************/
	/* SCALE4X                                                                               */
	/*****************************************************************************************/
	{
		// vertex shader
		"uniform vec2 size_texture;\n"
		"varying vec4 v_texCoord[7];\n"
		"void main() {\n"
		"	float x = 0.5 * (1.0 / size_texture.x);\n"
		"	float y = 0.5 * (1.0 / size_texture.y);\n"

		"	vec2 dg1 = vec2( x, y);\n"
		"	vec2 dg2 = vec2(-x, y);\n"
		"	vec2 sd1 = dg1 * 0.5;\n"
		"	vec2 sd2 = dg2 * 0.5;\n"

		"	gl_FrontColor = gl_Color;\n"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"

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
		"uniform sampler2D texture_txt;\n"
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

		"	vec3 dt = vec3(1.0, 1.0, 1.0);\n"

		"	float ko1=dot(abs(o1-c),dt);\n"
		"	float ko2=dot(abs(o2-c),dt);\n"
		"	float ko3=dot(abs(o3-c),dt);\n"
		"	float ko4=dot(abs(o4-c),dt);\n"

		"	float k1=min(dot(abs(i1-i3),dt),dot(abs(o1-o3),dt));\n"
		"	float k2=min(dot(abs(i2-i4),dt),dot(abs(o2-o4),dt));\n"

		"	float w1 = k2; if (ko3<ko1) w1 = 0.0;\n"
		"	float w2 = k1; if (ko4<ko2) w2 = 0.0;\n"
		"	float w3 = k2; if (ko1<ko3) w3 = 0.0;\n"
		"	float w4 = k1; if (ko2<ko4) w4 = 0.0;\n"

		"	vec4 scr = vec4((w1*o1+w2*o2+w3*o3+w4*o4+0.0001*c)/(w1+w2+w3+w4+0.0001), 1.0);\n"

		"	vec4 txt = texture2D(texture_txt, v_texCoord[0].xy);\n"

		"	gl_FragColor = mix(scr, txt, txt.a) * gl_Color;\n"
		"}"
	},
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
		"uniform sampler2D texture_txt;\n"
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

		"	vec4 txt = texture2D(texture_txt, v_texCoord[0].xy);\n"

		"	gl_FragColor = mix(scr, txt, txt.a) * gl_Color;\n"
		"}"
	},
	/*****************************************************************************************/
	/* HQ4X (non mi piace il risultato, non lo uso)                                          */
	/*****************************************************************************************/
	{
		// vertex shader
		"varying vec4 v_texCoord[7];\n"
		"void main() {\n"
		"	float x = 0.001;\n"
		"	float y = 0.001;\n"

		"	vec2 dg1 = vec2(x,y);   vec2 dg2 = vec2(-x,y);\n"
		"	vec2 sd1 = dg1 * 0.5;   vec2 sd2 = dg2 * 0.5;\n"
		"	vec2 ddx = vec2(x,0.0); vec2 ddy = vec2(0.0,y);\n"

		"	gl_FrontColor = gl_Color;\n"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"

		"	v_texCoord[0] = gl_MultiTexCoord0;\n"
		"	v_texCoord[1].xy = v_texCoord[0].xy - sd1;\n"
		"	v_texCoord[2].xy = v_texCoord[0].xy - sd2;\n"
		"	v_texCoord[3].xy = v_texCoord[0].xy + sd1;\n"
		"	v_texCoord[4].xy = v_texCoord[0].xy + sd2;\n"
		"	v_texCoord[5].xy = v_texCoord[0].xy - dg1;\n"
		"	v_texCoord[6].xy = v_texCoord[0].xy + dg1;\n"
		"	v_texCoord[5].zw = v_texCoord[0].xy - dg2;\n"
		"	v_texCoord[6].zw = v_texCoord[0].xy + dg2;\n"
		"	v_texCoord[1].zw = v_texCoord[0].xy - ddy;\n"
		"	v_texCoord[2].zw = v_texCoord[0].xy + ddx;\n"
		"	v_texCoord[3].zw = v_texCoord[0].xy + ddy;\n"
		"	v_texCoord[4].zw = v_texCoord[0].xy - ddx;\n"
		"}",
		// fragment shader
		"uniform sampler2D texture_scr;\n"
		"uniform sampler2D texture_txt;\n"
		"varying vec4 v_texCoord[7];\n"

		"const float mx = 1.00;      // start smoothing wt.\n"
		"const float k = -1.10;      // wt. decrease factor\n"
		"const float max_w = 0.75;    // max filter weigth\n"
		"const float min_w = 0.03;    // min filter weigth\n"
		"const float lum_add = 0.33;  // effects smoothing\n"

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
		"	vec3 s1 = texture2D(texture_scr, v_texCoord[1].zw).xyz;\n"
		"	vec3 s2 = texture2D(texture_scr, v_texCoord[2].zw).xyz;\n"
		"	vec3 s3 = texture2D(texture_scr, v_texCoord[3].zw).xyz;\n"
		"	vec3 s4 = texture2D(texture_scr, v_texCoord[4].zw).xyz;\n"

		"	vec3 dt = vec3(1.0, 1.0, 1.0);\n"

		"	float ko1=dot(abs(o1-c),dt);\n"
		"	float ko2=dot(abs(o2-c),dt);\n"
		"	float ko3=dot(abs(o3-c),dt);\n"
		"	float ko4=dot(abs(o4-c),dt);\n"

		"	float k1=min(dot(abs(i1-i3),dt),max(ko1,ko3));\n"
		"	float k2=min(dot(abs(i2-i4),dt),max(ko2,ko4));\n"

		"	float w1 = k2; if(ko3<ko1) w1*=ko3/ko1;\n"
		"	float w2 = k1; if(ko4<ko2) w2*=ko4/ko2;\n"
		"	float w3 = k2; if(ko1<ko3) w3*=ko1/ko3;\n"
		"	float w4 = k1; if(ko2<ko4) w4*=ko2/ko4;\n"

		"	c=(w1*o1+w2*o2+w3*o3+w4*o4+0.001*c)/(w1+w2+w3+w4+0.001);\n"

		"	w1 = k*dot(abs(i1-c)+abs(i3-c),dt)/(0.125*dot(i1+i3,dt)+lum_add);\n"
		"	w2 = k*dot(abs(i2-c)+abs(i4-c),dt)/(0.125*dot(i2+i4,dt)+lum_add);\n"
		"	w3 = k*dot(abs(s1-c)+abs(s3-c),dt)/(0.125*dot(s1+s3,dt)+lum_add);\n"
		"	w4 = k*dot(abs(s2-c)+abs(s4-c),dt)/(0.125*dot(s2+s4,dt)+lum_add);\n"

		"	w1 = clamp(w1+mx,min_w,max_w);\n"
		"	w2 = clamp(w2+mx,min_w,max_w);\n"
		"	w3 = clamp(w3+mx,min_w,max_w);\n"
		"	w4 = clamp(w4+mx,min_w,max_w);\n"

		"	vec4 scr;"

		"	scr.xyz = (w1*(i1+i3)+w2*(i2+i4)+w3*(s1+s3)+w4*(s2+s4)+c)/"
		"			   (2.0*(w1+w2+w3+w4)+1.0);\n"
		"	scr.a = 1.0;\n"

		"	vec4 txt = texture2D(texture_txt, v_texCoord[0].xy);\n"

		"	gl_FragColor = mix(scr, txt, txt.a) * gl_Color;\n"
		"}"
	},
	/*****************************************************************************************/
	/* 4xRB                                                                                  */
	/*****************************************************************************************/
	{
		// vertex shader
		NULL,
		// fragment shader
		"uniform vec2 size_texture;\n"
		"uniform sampler2D texture_scr;\n"
		"uniform sampler2D texture_txt;\n"

		"const vec3 dtt = vec3(65536.0, 255.0, 1.0);\n"
		"float reduce(vec3 color) {\n"
		"	return dot(color, dtt);\n"
		"}"

		"void main() {\n"
		"	vec2 ps = 1.0 / size_texture;\n"
		"	vec2 dx = vec2(ps.x, 0.0);\n"
		"	vec2 dy = vec2(0.0, ps.y);\n"

		"	vec2 pixcoord = gl_TexCoord[0].xy / ps;\n;"
		"	vec2 fp = fract(pixcoord);\n"
		"	vec2 d11 = gl_TexCoord[0].xy - fp * ps;\n"

		"	// Reading the texels\n"
		"	vec3 A = texture2D(texture_scr, d11-dx-dy).xyz;\n"
		"	vec3 B = texture2D(texture_scr, d11   -dy).xyz;\n"
		"	vec3 C = texture2D(texture_scr, d11+dx-dy).xyz;\n"
		"	vec3 D = texture2D(texture_scr, d11-dx   ).xyz;\n"
		"	vec3 E = texture2D(texture_scr, d11      ).xyz;\n"
		"	vec3 F = texture2D(texture_scr, d11+dx   ).xyz;\n"
		"	vec3 G = texture2D(texture_scr, d11-dx+dy).xyz;\n"
		"	vec3 H = texture2D(texture_scr, d11+dy   ).xyz;\n"
		"	vec3 I = texture2D(texture_scr, d11+dx+dy).xyz;\n"

		"	vec3 E0 = E;\n"
		"	vec3 E1 = E;\n"
		"	vec3 E2 = E;\n"
		"	vec3 E3 = E;\n"
		"	vec3 E4 = E;\n"
		"	vec3 E5 = E;\n"
		"	vec3 E6 = E;\n"
		"	vec3 E7 = E;\n"
		"	vec3 E8 = E;\n"
		"	vec3 E9 = E;\n"
		"	vec3 E10 = E;\n"
		"	vec3 E11 = E;\n"
		"	vec3 E12 = E;\n"
		"	vec3 E13 = E;\n"
		"	vec3 E14 = E;\n"
		"	vec3 E15 = E;\n"

		"	float a = reduce(A);\n"
		"	float b = reduce(B);\n"
		"	float c = reduce(C);\n"
		"	float d = reduce(D);\n"
		"	float e = reduce(E);\n"
		"	float f = reduce(F);\n"
		"	float g = reduce(G);\n"
		"	float h = reduce(H);\n"
		"	float i = reduce(I);\n"

		"	if ((h == f)&&(h != e)) {\n"
		"		if (((e == g) && ((i == h) || (e == d))) ||"
		"			((e == c) && ((i == h) || (e == b)))) {\n"
		"			E11 = mix(E11, F, 0.5);\n"
		"			E14 = E11;\n"
		"			E15 = F;\n"
		"		}\n"
		"	}\n"

		"	if ((f == b)&&(f != e)) {\n"
		"		if (((e == i) && ((f == c) || (e == h))) ||"
		"			((e == a) && ((f == c) || (e == d)))) {\n"
		"			E2 = mix(E2, B, 0.5);\n"
		"			E7 = E2;\n"
		"			E3 = B;\n"
		"		}\n"
		"	}\n"

		"	if ((b == d)&&(b != e)) {\n"
		"		if (((e == c) && ((b == a) || (e == f))) ||"
		"			((e == g) && ((b == a) || (e == h)))) {\n"
		"			E1 = mix(E1, D, 0.5);\n"
		"			E4 = E1;\n"
		"			E0 = D;\n"
		"		}\n"
		"	}\n"

		"	if ((d == h)&&(d != e)) {\n"
		"		if (((e == a) && ((d == g) || (e == b))) ||\n"
		"			((e == i) && ((d == g) || (e == f)))) {\n"
		"			E8 = mix(E8, H, 0.5);\n"
		"			E13 = E8;\n"
		"			E12 = H;\n"
		"		}\n"
		"	}\n"

		"vec3 res;\n"

		"	if (fp.x < 0.25) {\n"
		"		if (fp.y < 0.25) res = E0;\n"
		"		else if ((fp.y > 0.25) && (fp.y < 0.50)) res = E4;\n"
		"		else if ((fp.y > 0.50) && (fp.y < 0.75)) res = E8;\n"
		"		else res = E12;\n"
		"	} else if ((fp.x > 0.25) && (fp.x < 0.50)) {\n"
		"		if (fp.y < 0.25) res = E1;\n"
		"		else if ((fp.y > 0.25) && (fp.y < 0.50)) res = E5;\n"
		"		else if ((fp.y > 0.50) && (fp.y < 0.75)) res = E9;\n"
		"		else res = E13;\n"
		"	} else if ((fp.x > 0.50) && (fp.x < 0.75)) {\n"
		"		if (fp.y < 0.25) res = E2;\n"
		"		else if ((fp.y > 0.25) && (fp.y < 0.50)) res = E6;\n"
		"		else if ((fp.y > 0.50) && (fp.y < 0.75)) res = E10;\n"
		"		else res = E14;\n"
		"	} else {\n"
		"		if (fp.y < 0.25) res = E3;\n"
		"		else if ((fp.y > 0.25) && (fp.y < 0.50)) res = E7;\n"
		"		else if ((fp.y > 0.50) && (fp.y < 0.75)) res = E11;\n"
		"		else res = E15;\n"
		"	}\n"

		"	gl_FragColor = vec4(res, 1.0);\n"
		"}"
	},
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
		"uniform sampler2D texture_txt;\n"
		"varying vec4 v_texCoord;\n"

		"#define round(x) floor( (x) + 0.5 )\n"

		"void main() {\n"
		"	vec2 texelSize = 1.0 / size_texture;\n"
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

		"	vec4 txt = texture2D(texture_txt, v_texCoord.xy);\n"

		"	gl_FragColor = mix(scr, txt, txt.a) * gl_Color;\n"
		"}"
	},
	/*****************************************************************************************/
	/* Phosphor                                                                              */
	/*****************************************************************************************/
	{
		// vertex shader
		NULL,
		// fragment shader
		"uniform vec2 size_texture;\n"
		"uniform sampler2D texture_scr;\n"
		"uniform sampler2D texture_txt;\n"

		"vec3 to_focus(float pixel) {\n"
		"	pixel = mod(pixel + 3.0, 3.0);\n"
		"	if (pixel >= 2.0) // Blue\n"
		"		return vec3(pixel - 2.0, 0.0, 3.0 - pixel);\n"
		"	else if (pixel >= 1.0)// Green\n"
		"		return vec3(0.0, 2.0 - pixel, pixel - 1.0);\n"
		"	else// Red\n"
		"		return vec3(1.0 - pixel, pixel, 0.0);\n"
		"}\n"

		"void main() {\n"
		"	float y = mod(gl_TexCoord[0].y * size_texture.y, 1.0);\n"
		"	float intensity = exp(-0.2 * y);\n"

		"	vec2 one_x = vec2(1.0 / (3.0 * size_texture.x), 0.0);\n"

		"	vec3 color = texture2D(texture_scr, gl_TexCoord[0].xy - 0.0 * one_x).rgb;\n"
		"	vec3 color_prev = texture2D(texture_scr, gl_TexCoord[0].xy - 1.0 * one_x).rgb;\n"
		"	vec3 color_prev_prev = texture2D(texture_scr, gl_TexCoord[0].xy - 2.0 * one_x).rgb;\n"

		"	float pixel_x = 3.0 * gl_TexCoord[0].x * size_texture.x;\n"

		"	vec3 focus = to_focus(pixel_x - 0.0);\n"
		"	vec3 focus_prev = to_focus(pixel_x - 1.0);\n"
		"	vec3 focus_prev_prev = to_focus(pixel_x - 2.0);\n"

		"	vec3 result = 0.8 * color * focus +"
		"				  0.6 * color_prev * focus_prev +"
		"				  0.3 * color_prev_prev * focus_prev_prev;\n"

		"	result = 2.3 * pow(result, vec3(1.4));\n"

		"	vec4 scr = vec4(intensity * result, 1.0);\n"

		"	vec4 txt = texture2D(texture_txt, gl_TexCoord[0].xy);\n"

		"	gl_FragColor = mix(scr, txt, txt.a) * gl_Color;\n"
		"}"
	},
	/*****************************************************************************************/
	/* Scanline                                                                              */
	/*****************************************************************************************/
	{
		// vertex shader
		"uniform vec2 size_texture;\n"
		"uniform vec2 size_output;\n"
		"varying vec4 v_texCoord;\n"
		"varying vec2 omega;\n"
		"void main() {\n"
		"	omega = vec2(3.1415 * size_output.x * size_texture.x /"
		"				size_texture.x, 2.0 * 3.1415 * size_texture.y);\n"

		"	gl_FrontColor = gl_Color;\n"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"

		"	v_texCoord = gl_MultiTexCoord0;\n"
		"}",
		// fragment shader
		"uniform vec2 size_texture;\n"
		"uniform sampler2D texture_scr;\n"
		"uniform sampler2D texture_txt;\n"
		"varying vec4 v_texCoord;\n"
		"varying vec2 omega;\n"

		"const float base_brightness = 0.95;\n"
		"const vec2 sine_comp = vec2(0.05, 0.15);\n"

		"void main() {\n"
		"	vec4 c11 = texture2D(texture_scr, v_texCoord.xy);\n"

		"	vec4 scanline = c11 * (base_brightness +"
		"					dot(sine_comp * sin(v_texCoord.xy * omega), vec2(1.0)));\n"
		"	vec4 scr = clamp(scanline, 0.0, 1.0);\n"

		"	vec4 txt = texture2D(texture_txt, v_texCoord.xy);\n"

		"	gl_FragColor = mix(scr, txt, txt.a) * gl_Color;\n"
		"}"
	},
};
#undef _SHADERS_CODE_
#endif
