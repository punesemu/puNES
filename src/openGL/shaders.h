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
	SHADER_QUILAZ,
	SHADER_WATERPAINT,
	SHADER_CRT,
	SHADER_CRT2,
	SHADER_CRT3,
	SHADER_CRT4,
	SHADER_TOTAL,
	SHADER_NONE = 255
};

#define MAX_SHADERS 10

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
typedef struct {
	GLuint prg;
	GLuint vrt;
	GLuint frg;
	GLuint routine;
	_shader_routine *code;
} _shd;
struct {
	_shd compiled[MAX_SHADERS];

	_texture text;

	struct {
		GLfloat input[2];
		GLfloat output[2];
		GLfloat texture[2];
	} size;
	GLint frame_counter;

	struct {
		struct {
			GLint input;
			GLint output;
			GLint texture;
		} size;
		struct {
			GLint scr;
			GLint txt;
		} texture;
		GLint frame_counter;
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
		"uniform vec2 size_input;\n"
		"uniform vec2 size_output;\n"
		"uniform vec2 size_texture;\n"
		"varying vec4 v_texCoord;\n"
		"varying vec2 omega;\n"
		"void main() {\n"
		"	omega = vec2(3.1415 * size_output.x * size_texture.x /"
		"				size_input.x, 2.0 * 3.1415 * size_texture.y);\n"

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
	/*****************************************************************************************/
	/* Quilaz                                                                                */
	/*****************************************************************************************/
	{
		// vertex shader
		NULL,
		// fragment shader
		"uniform vec2 size_texture;\n"
		"uniform sampler2D texture_scr;\n"
		"uniform sampler2D texture_txt;\n"

		"vec4 getTexel(vec2 p) {\n"
		"	p = p * size_texture + vec2(0.5);\n"

		"	vec2 i = floor(p);\n"
		"	vec2 f = p - i;\n"
		"	f = f * f * f * (f * (f * 6.0 - vec2(15.0)) + vec2(10.0));\n"
		"	p = i + f;\n"

		"	p = (p - vec2(0.5)) / size_texture;\n"
		"	return texture2D(texture_scr, p);\n"
		"}\n"

		"void main() {\n"
		"	vec4 scr = getTexel(gl_TexCoord[0].xy);\n"
		"	vec4 txt = texture2D(texture_txt, gl_TexCoord[0].xy);\n"
		"	gl_FragColor = mix(scr, txt, txt.a) * gl_Color;\n"
		"}"
	},
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
		"uniform sampler2D texture_txt;\n"
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

		"	vec4 txt = texture2D(texture_txt, gl_TexCoord[0].xy);\n"

		"	gl_FragColor = mix(scr, txt, txt.a) * gl_Color;\n"
		"}"
	},
	/*****************************************************************************************/
	/* CRT                                                                                   */
	/*****************************************************************************************/
	{
		// vertex shader
		"varying float CRTgamma;\n"
		"varying float monitorgamma;\n"
		"varying vec2 overscan;\n"
		"varying vec2 aspect;\n"
		"varying float d;\n"
		"varying float R;\n"
		"varying float cornersize;\n"
		"varying float cornersmooth;\n"

		"varying vec3 stretch;\n"
		"varying vec2 sinangle;\n"
		"varying vec2 cosangle;\n"

		"uniform vec2 size_input;\n"
		"uniform vec2 size_output;\n"
		"uniform vec2 size_texture;\n"

		"varying vec2 texCoord;\n"
		"varying vec2 one;\n"
		"varying float mod_factor;\n"

		"#define FIX(c) max(abs(c), 1e-5);\n"

		"float intersect(vec2 xy) {\n"
		"	float A = dot(xy,xy)+d*d;\n"
		"	float B = 2.0*(R*(dot(xy,sinangle)-d*cosangle.x*cosangle.y)-d*d);\n"
		"	float C = d*d + 2.0*R*d*cosangle.x*cosangle.y;\n"
		"	return (-B-sqrt(B*B-4.0*A*C))/(2.0*A);\n"
		"}\n"

		"vec2 bkwtrans(vec2 xy) {\n"
		"	float c = intersect(xy);\n"
		"	vec2 point = vec2(c)*xy;\n"
		"	point -= vec2(-R)*sinangle;\n"
		"	point /= vec2(R);\n"
		"	vec2 tang = sinangle/cosangle;\n"
		"	vec2 poc = point/cosangle;\n"
		"	float A = dot(tang,tang)+1.0;\n"
		"	float B = -2.0*dot(poc,tang);\n"
		"	float C = dot(poc,poc)-1.0;\n"
		"	float a = (-B+sqrt(B*B-4.0*A*C))/(2.0*A);\n"
		"	vec2 uv = (point-a*sinangle)/cosangle;\n"
		"	float r = R*acos(a);\n"
		"	return uv*r/sin(r/R);\n"
		"}\n"

		"vec2 fwtrans(vec2 uv) {\n"
		"	float r = FIX(sqrt(dot(uv,uv)));\n"
		"	uv *= sin(r/R)/r;\n"
		"	float x = 1.0-cos(r/R);\n"
		"	float D = d/R + x*cosangle.x*cosangle.y+dot(uv,sinangle);\n"
		"	return d*(uv*cosangle-x*sinangle)/D;\n"
		"}\n"

		"vec3 maxscale() {\n"
		"	vec2 c = bkwtrans(-R * sinangle / (1.0 + R/d*cosangle.x*cosangle.y));\n"
		"	vec2 a = vec2(0.5,0.5)*aspect;\n"
		"	vec2 lo = vec2(fwtrans(vec2(-a.x,c.y)).x,\n"
		"			fwtrans(vec2(c.x,-a.y)).y)/aspect;\n"
		"	vec2 hi = vec2(fwtrans(vec2(+a.x,c.y)).x,\n"
		"			fwtrans(vec2(c.x,+a.y)).y)/aspect;\n"
		"	return vec3((hi+lo)*aspect*0.5,max(hi.x-lo.x,hi.y-lo.y));\n"
		"}\n"

		"void main() {\n"
		"	// START of parameters\n"

		"	// gamma of simulated CRT\n"
		"	CRTgamma = 2.4;\n"
		"	// gamma of display monitor (typically 2.2 is correct)\n"
		"	monitorgamma = 2.2;\n"
		"	// overscan (e.g. 1.02 for 2% overscan)\n"
		"	overscan = vec2(1.01,1.01);\n"
		"	// aspect ratio\n"
		"	aspect = vec2(1.0, 0.75);\n"
		"	// lengths are measured in units of (approximately) the width\n"
		"	// of the monitor simulated distance from viewer to monitor\n"
		"	d = 2.0;\n"
		"	// radius of curvature\n"
		"	R = 1.5;\n"
		"	// tilt angle in radians\n"
		"	// (behavior might be a bit wrong if both components are\n"
		"	// nonzero)\n"
		"	const vec2 angle = vec2(0.0,-0.15);\n"
		"	// size of curved corners\n"
		"	cornersize = 0.03;\n"
		"	// border smoothness parameter\n"
		"	// decrease if borders are too aliased\n"
		"	cornersmooth = 1000.0;\n"

		"	// END of parameters\n"

		"	gl_FrontColor = gl_Color;\n"

		"	// Do the standard vertex processing.\n"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"

		"	// Precalculate a bunch of useful values we'll need in the fragment\n"
		"	// shader.\n"
		"	sinangle = sin(angle);\n"
		"	cosangle = cos(angle);\n"
		"	stretch = maxscale();\n"

		"	// Texture coords.\n"
		"	texCoord = gl_MultiTexCoord0.xy;\n"

		"	// The size of one texel, in texture-coordinates.\n"
		"	one = 1.0 / size_texture;\n"

		"	// Resulting X pixel-coordinate of the pixel we're drawing.\n"
		"	mod_factor = texCoord.x * size_texture.x * size_output.x / size_input.x;\n"
		"}",
		// fragment shader
		"// Comment the next line to disable interpolation in linear gamma (and\n"
		"// gain speed).\n"
		"#define LINEAR_PROCESSING\n"

		"// Enable screen curvature.\n"
		"#define CURVATURE\n"

		"// Enable 3x oversampling of the beam profile\n"
		"#define OVERSAMPLE\n"

		"// Use the older, purely gaussian beam profile\n"
		"//#define USEGAUSSIAN\n"

		"// Macros.\n"
		"#define FIX(c) max(abs(c), 1e-5);\n"
		"#define PI 3.141592653589\n"

		"#ifdef LINEAR_PROCESSING\n"
		"#	define TEX2D(c) pow(texture2D(texture_scr, (c)), vec4(CRTgamma))\n"
		"#else\n"
		"#  define TEX2D(c) texture2D(texture_scr, (c))\n"
		"#endif\n"

		"uniform vec2 size_input;\n"
		"uniform vec2 size_texture;\n"
		"uniform sampler2D texture_scr;\n"
		"uniform sampler2D texture_txt;\n"

		"varying vec2 texCoord;\n"
		"varying vec2 one;\n"
		"varying float mod_factor;\n"

		"varying float CRTgamma;\n"
		"varying float monitorgamma;\n"

		"varying vec2 overscan;\n"
		"varying vec2 aspect;\n"

		"varying float d;\n"
		"varying float R;\n"

		"varying float cornersize;\n"
		"varying float cornersmooth;\n"

		"varying vec3 stretch;\n"
		"varying vec2 sinangle;\n"
		"varying vec2 cosangle;\n"

		"float intersect(vec2 xy) {\n"
		"	float A = dot(xy,xy)+d*d;\n"
		"	float B = 2.0*(R*(dot(xy,sinangle)-d*cosangle.x*cosangle.y)-d*d);\n"
		"	float C = d*d + 2.0*R*d*cosangle.x*cosangle.y;\n"
		"	return (-B-sqrt(B*B-4.0*A*C))/(2.0*A);\n"
		"}\n"

		"vec2 bkwtrans(vec2 xy) {\n"
		"	float c = intersect(xy);\n"
		"	vec2 point = vec2(c)*xy;\n"
		"	point -= vec2(-R)*sinangle;\n"
		"	point /= vec2(R);\n"
		"	vec2 tang = sinangle/cosangle;\n"
		"	vec2 poc = point/cosangle;\n"
		"	float A = dot(tang,tang)+1.0;\n"
		"	float B = -2.0*dot(poc,tang);\n"
		"	float C = dot(poc,poc)-1.0;\n"
		"	float a = (-B+sqrt(B*B-4.0*A*C))/(2.0*A);\n"
		"	vec2 uv = (point-a*sinangle)/cosangle;\n"
		"	float r = FIX(R*acos(a));\n"
		"	return uv*r/sin(r/R);\n"
		"}\n"

		"vec2 transform(vec2 coord) {\n"
		"	coord *= size_texture / size_input;\n"
		"	coord = (coord-vec2(0.5))*aspect*stretch.z+stretch.xy;\n"
		"	return (bkwtrans(coord)/overscan/aspect+vec2(0.5)) * size_input / size_texture;\n"
		"}\n"

		"float corner(vec2 coord) {\n"
		"	coord *= size_texture / size_input;\n"
		"	coord = (coord - vec2(0.5)) * overscan + vec2(0.5);\n"
		"	coord = min(coord, vec2(1.0)-coord) * aspect;\n"
		"	vec2 cdist = vec2(cornersize);\n"
		"	coord = (cdist - min(coord,cdist));\n"
		"	float dist = sqrt(dot(coord,coord));\n"
		"	return clamp((cdist.x-dist)*cornersmooth,0.0, 1.0);\n"
		"}\n"

		"// Calculate the influence of a scanline on the current pixel.\n"
		"// 'distance' is the distance in texture coordinates from the current\n"
		"// pixel to the scanline in question.\n"
		"// 'color' is the colour of the scanline at the horizontal location of\n"
		"// the current pixel.\n"
		"vec4 scanlineWeights(float distance, vec4 color) {\n"
		"	// 'wid' controls the width of the scanline beam, for each RGB\n"
		"	// channel The 'weights' lines basically specify the formula\n"
		"	// that gives you the profile of the beam, i.e. the intensity as\n"
		"	// a function of distance from the vertical center of the\n"
		"	// scanline. In this case, it is gaussian if width=2, and\n"
		"	// becomes nongaussian for larger widths. Ideally this should\n"
		"	// be normalized so that the integral across the beam is\n"
		"	// independent of its width. That is, for a narrower beam\n"
		"	// 'weights' should have a higher peak at the center of the\n"
		"	// scanline than for a wider beam.\n"
		"#ifdef USEGAUSSIAN\n"
		"	vec4 wid = 0.3 + 0.1 * pow(color, vec4(3.0));\n"
		"	vec4 weights = vec4(distance / wid);\n"
		"	return 0.4 * exp(-weights * weights) / wid;\n"
		"#else\n"
		"	vec4 wid = 2.0 + 2.0 * pow(color, vec4(4.0));\n"
		"	vec4 weights = vec4(distance / 0.3);\n"
		"	return 1.4 * exp(-pow(weights * inversesqrt(0.5 * wid), wid)) / (0.6 + 0.2 * wid);\n"
		"#endif\n"
		"}\n"

		"void main() {\n"
		"	// Here's a helpful diagram to keep in mind while trying to\n"
		"	// understand the code:\n"
		"	//\n"
		"	//  |      |      |      |      |\n"
		"	// -------------------------------\n"
		"	//  |      |      |      |      |\n"
		"	//  |  01  |  11  |  21  |  31  | <-- current scanline\n"
		"	//  |      | @    |      |      |\n"
		"	// -------------------------------\n"
		"	//  |      |      |      |      |\n"
		"	//  |  02  |  12  |  22  |  32  | <-- next scanline\n"
		"	//  |      |      |      |      |\n"
		"	// -------------------------------\n"
		"	//  |      |      |      |      |\n"
		"	//\n"
		"	// Each character-cell represents a pixel on the output\n"
		"	// surface, '@' represents the current pixel (always somewhere\n"
		"	// in the bottom half of the current scan-line, or the top-half\n"
		"	// of the next scanline). The grid of lines represents the\n"
		"	// edges of the texels of the underlying texture.\n"

		"	// Texture coordinates of the texel containing the active pixel.\n"
		"#ifdef CURVATURE\n"
		"	vec2 xy = transform(texCoord);\n"
		"#else\n"
		"	vec2 xy = texCoord;\n"
		"#endif\n"
		"	float cval = corner(xy);\n"

		"	// Of all the pixels that are mapped onto the texel we are\n"
		"	// currently rendering, which pixel are we currently rendering?\n"
		"	vec2 ratio_scale = xy * size_texture - vec2(0.5);\n"
		"#ifdef OVERSAMPLE\n"
		"	float filter = fwidth(ratio_scale.y);\n"
		"#endif\n"
		"	vec2 uv_ratio = fract(ratio_scale);\n"

		"	// Snap to the center of the underlying texel.\n"
		"	xy = (floor(ratio_scale) + vec2(0.5)) / size_texture;\n"

		"	// Calculate Lanczos scaling coefficients describing the effect\n"
		"	// of various neighbour texels in a scanline on the current\n"
		"	// pixel.\n"
		"	vec4 coeffs = PI * vec4(1.0 + uv_ratio.x, uv_ratio.x,"
		"					 1.0 - uv_ratio.x, 2.0 - uv_ratio.x);\n"

		"	// Prevent division by zero.\n"
		"	coeffs = FIX(coeffs);\n"

		"	// Lanczos2 kernel.\n"
		"	coeffs = 2.0 * sin(coeffs) * sin(coeffs / 2.0) / (coeffs * coeffs);\n"

		"	// Normalize.\n"
		"	coeffs /= dot(coeffs, vec4(1.0));\n"

		"	// Calculate the effective colour of the current and next\n"
		"	// scanlines at the horizontal location of the current pixel,\n"
		"	// using the Lanczos coefficients above.\n"
		"	vec4 col = clamp(mat4(\n"
		"			TEX2D(xy + vec2(-one.x, 0.0)),\n"
		"			TEX2D(xy),\n"
		"			TEX2D(xy + vec2(one.x, 0.0)),\n"
		"			TEX2D(xy + vec2(2.0 * one.x, 0.0))) * coeffs,\n"
		"			0.0, 1.0);\n"
		"	vec4 col2 = clamp(mat4(\n"
		"			TEX2D(xy + vec2(-one.x, one.y)),\n"
		"			TEX2D(xy + vec2(0.0, one.y)),\n"
		"			TEX2D(xy + one),\n"
		"			TEX2D(xy + vec2(2.0 * one.x, one.y))) * coeffs,\n"
		"			0.0, 1.0);\n"

		"#ifndef LINEAR_PROCESSING\n"
		"	col = pow(col , vec4(CRTgamma));\n"
		"	col2 = pow(col2, vec4(CRTgamma));\n"
		"#endif\n"

		"	// Calculate the influence of the current and next scanlines on\n"
		"	// the current pixel.\n"
		"	vec4 weights = scanlineWeights(uv_ratio.y, col);\n"
		"	vec4 weights2 = scanlineWeights(1.0 - uv_ratio.y, col2);\n"
		"#ifdef OVERSAMPLE\n"
		"	uv_ratio.y =uv_ratio.y+1.0/3.0*filter;\n"
		"	weights = (weights+scanlineWeights(uv_ratio.y, col))/3.0;\n"
		"	weights2=(weights2+scanlineWeights(abs(1.0-uv_ratio.y), col2))/3.0;\n"
		"	uv_ratio.y =uv_ratio.y-2.0/3.0*filter;\n"
		"	weights=weights+scanlineWeights(abs(uv_ratio.y), col)/3.0;\n"
		"	weights2=weights2+scanlineWeights(abs(1.0-uv_ratio.y), col2)/3.0;\n"
		"#endif\n"
		"	vec3 mul_res = (col * weights + col2 * weights2).rgb * vec3(cval);\n"

		"	// dot-mask emulation:\n"
		"	// Output pixels are alternately tinted green and magenta.\n"
		"	vec3 dotMaskWeights = mix(\n"
		"			vec3(1.0, 0.7, 1.0),\n"
		"			vec3(0.7, 1.0, 0.7),\n"
		"			floor(mod(mod_factor, 2.0))\n"
		"	);\n"

		"	mul_res *= dotMaskWeights;\n"

		"	// Convert the image gamma for display on our output device.\n"
		"	mul_res = pow(mul_res, vec3(1.0 / monitorgamma));\n"

		"	// Color the texel.\n"
		"	vec4 scr = vec4(mul_res, 1.0);\n"

		"	vec4 txt = texture2D(texture_txt, texCoord.xy);\n"

		"	gl_FragColor = mix(scr, txt, txt.a) * gl_Color;\n"
		"}"
	},
	/*****************************************************************************************/
	/* CRT2                                                                                  */
	/*****************************************************************************************/
	{
		// vertex shader
		NULL,
		// fragment shader
		"uniform vec2 size_texture;\n"
		"uniform sampler2D texture_scr;\n"
		"uniform sampler2D texture_txt;\n"

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

		"	vec4 txt = texture2D(texture_txt, gl_TexCoord[0].xy);\n"

		"	gl_FragColor = mix(scr, txt, txt.a) * gl_Color;\n"
		"}"
	},
	/*****************************************************************************************/
	/* CRT3                                                                                  */
	/*****************************************************************************************/
	{
		// vertex shader
		NULL,
		// fragment shader
		"uniform vec2 size_texture;\n"
		"uniform sampler2D texture_scr;\n"
		"uniform sampler2D texture_txt;\n"

		"#define display_gamma 2.2\n"
		"#define TEX2D(c) pow(texture2D(texture_scr,(c)),vec4(display_gamma))\n"

		"void main() {\n"
		"	vec2 xy = gl_TexCoord[0].st;\n"
		"	float onex = 1.0/size_texture.x;\n"

		"	float wid = 2.0;\n"

		"	float c1 = exp(-1.0/wid/wid);\n"
		"	float c2 = exp(-4.0/wid/wid);\n"
		"	float c3 = exp(-9.0/wid/wid);\n"
		"	float c4 = exp(-16.0/wid/wid);\n"
		"	float norm = 1.0 / (1.0 + 2.0*(c1+c2+c3+c4));\n"

		"	vec4 sum = vec4(0.0);\n"

		"	sum += TEX2D(xy + vec2(-4.0 * onex, 0.0)) * vec4(c4);\n"
		"	sum += TEX2D(xy + vec2(-3.0 * onex, 0.0)) * vec4(c3);\n"
		"	sum += TEX2D(xy + vec2(-2.0 * onex, 0.0)) * vec4(c2);\n"
		"	sum += TEX2D(xy + vec2(-1.0 * onex, 0.0)) * vec4(c1);\n"
		"	sum += TEX2D(xy);\n"
		"	sum += TEX2D(xy + vec2(+1.0 * onex, 0.0)) * vec4(c1);\n"
		"	sum += TEX2D(xy + vec2(+2.0 * onex, 0.0)) * vec4(c2);\n"
		"	sum += TEX2D(xy + vec2(+3.0 * onex, 0.0)) * vec4(c3);\n"
		"	sum += TEX2D(xy + vec2(+4.0 * onex, 0.0)) * vec4(c4);\n"

		"	vec4 scr = pow(sum*vec4(norm),vec4(1.0/display_gamma));\n"

		"	vec4 txt = texture2D(texture_txt, gl_TexCoord[0].xy);\n"

		"	gl_FragColor = mix(scr, txt, txt.a) * gl_Color;\n"
		"}"
	},
	/*****************************************************************************************/
	/* CRT4                                                                                  */
	/*****************************************************************************************/
	{
		// vertex shader
		"varying float CRTgamma;\n"
		"varying float monitorgamma;\n"
		"varying vec2 overscan;\n"
		"varying vec2 aspect;\n"
		"varying float d;\n"
		"varying float R;\n"
		"varying float cornersize;\n"
		"varying float cornersmooth;\n"

		"varying vec3 stretch;\n"
		"varying vec2 sinangle;\n"
		"varying vec2 cosangle;\n"

		"uniform vec2 size_input;\n"
		"uniform vec2 size_output;\n"
		"uniform vec2 size_texture;\n"

		"varying vec2 texCoord;\n"
		"varying vec2 one;\n"
		"varying float mod_factor;\n"
		"varying vec2 ilfac;\n"

		"#define FIX(c) max(abs(c), 1e-5);\n"

		"float intersect(vec2 xy) {\n"
		"	float A = dot(xy,xy)+d*d;\n"
		"	float B = 2.0*(R*(dot(xy,sinangle)-d*cosangle.x*cosangle.y)-d*d);\n"
		"	float C = d*d + 2.0*R*d*cosangle.x*cosangle.y;\n"
		"	return (-B-sqrt(B*B-4.0*A*C))/(2.0*A);\n"
		"}\n"

		"vec2 bkwtrans(vec2 xy) {\n"
		"	float c = intersect(xy);\n"
		"	vec2 point = vec2(c)*xy;\n"
		"	point -= vec2(-R)*sinangle;\n"
		"	point /= vec2(R);\n"
		"	vec2 tang = sinangle/cosangle;\n"
		"	vec2 poc = point/cosangle;\n"
		"	float A = dot(tang,tang)+1.0;\n"
		"	float B = -2.0*dot(poc,tang);\n"
		"	float C = dot(poc,poc)-1.0;\n"
		"	float a = (-B+sqrt(B*B-4.0*A*C))/(2.0*A);\n"
		"	vec2 uv = (point-a*sinangle)/cosangle;\n"
		"	float r = R*acos(a);\n"
		"	return uv*r/sin(r/R);\n"
		"}\n"

		"vec2 fwtrans(vec2 uv) {\n"
		"	float r = FIX(sqrt(dot(uv,uv)));\n"
		"	uv *= sin(r/R)/r;\n"
		"	float x = 1.0-cos(r/R);\n"
		"	float D = d/R + x*cosangle.x*cosangle.y+dot(uv,sinangle);\n"
		"	return d*(uv*cosangle-x*sinangle)/D;\n"
		"}\n"

		"vec3 maxscale() {\n"
		"	vec2 c = bkwtrans(-R * sinangle / (1.0 + R/d*cosangle.x*cosangle.y));\n"
		"	vec2 a = vec2(0.5,0.5)*aspect;\n"
		"	vec2 lo = vec2(fwtrans(vec2(-a.x,c.y)).x,\n"
		"			fwtrans(vec2(c.x,-a.y)).y)/aspect;\n"
		"	vec2 hi = vec2(fwtrans(vec2(+a.x,c.y)).x,\n"
		"			fwtrans(vec2(c.x,+a.y)).y)/aspect;\n"
		"	return vec3((hi+lo)*aspect*0.5,max(hi.x-lo.x,hi.y-lo.y));\n"
		"}\n"

		"void main() {\n"
		"	// START of parameters\n"

		"	// gamma of simulated CRT\n"
		"	CRTgamma = 2.4;\n"
		"	// gamma of display monitor (typically 2.2 is correct)\n"
		"	monitorgamma = 2.2;\n"
		"	// overscan (e.g. 1.02 for 2% overscan)\n"
		"	overscan = vec2(1.01,1.01);\n"
		"	// aspect ratio\n"
		"	aspect = vec2(1.0, 0.75);\n"
		"	// lengths are measured in units of (approximately) the width\n"
		"	// of the monitor simulated distance from viewer to monitor\n"
		"	d = 2.0;\n"
		"	// radius of curvature\n"
		"	R = 1.5;\n"
		"	// tilt angle in radians\n"
		"	// (behavior might be a bit wrong if both components are\n"
		"	// nonzero)\n"
		"	const vec2 angle = vec2(0.0,-0.15);\n"
		"	// size of curved corners\n"
		"	cornersize = 0.03;\n"
		"	// border smoothness parameter\n"
		"	// decrease if borders are too aliased\n"
		"	cornersmooth = 1000.0;\n"

		"	// END of parameters\n"

		"	gl_FrontColor = gl_Color;\n"

		"	// Do the standard vertex processing.\n"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"

		"	// Precalculate a bunch of useful values we'll need in the fragment\n"
		"	// shader.\n"
		"	sinangle = sin(angle);\n"
		"	cosangle = cos(angle);\n"
		"	stretch = maxscale();\n"

		"	// Texture coords.\n"
		"	texCoord = gl_MultiTexCoord0.xy;\n"

		"	ilfac = vec2(1.0,floor(size_input.y/200.0));\n"

		"	// The size of one texel, in texture-coordinates.\n"
		"	one = ilfac / size_texture;\n"

		"	// Resulting X pixel-coordinate of the pixel we're drawing.\n"
		"	mod_factor = texCoord.x * size_texture.x * size_output.x / size_input.x;\n"
		"}",
		// fragment shader
		"// Comment the next line to disable interpolation in linear gamma (and\n"
		"// gain speed).\n"
		"#define LINEAR_PROCESSING\n"

		"// Enable screen curvature.\n"
		"#define CURVATURE\n"

		"// Enable 3x oversampling of the beam profile\n"
		"#define OVERSAMPLE\n"

		"// Use the older, purely gaussian beam profile\n"
		"//#define USEGAUSSIAN\n"

		"// Macros.\n"
		"#define FIX(c) max(abs(c), 1e-5);\n"
		"#define PI 3.141592653589\n"

		"#ifdef LINEAR_PROCESSING\n"
		"#	define TEX2D(c) pow(texture2D(texture_scr, (c)), vec4(CRTgamma))\n"
		"#else\n"
		"#  define TEX2D(c) texture2D(texture_scr, (c))\n"
		"#endif\n"

		"uniform vec2 size_input;\n"
		"uniform vec2 size_texture;\n"
		"uniform int frame_counter;\n"
		"uniform sampler2D texture_scr;\n"
		"uniform sampler2D texture_txt;\n"

		"varying vec2 texCoord;\n"
		"varying vec2 one;\n"
		"varying float mod_factor;\n"
		"varying vec2 ilfac;\n"

		"varying float CRTgamma;\n"
		"varying float monitorgamma;\n"

		"varying vec2 overscan;\n"
		"varying vec2 aspect;\n"

		"varying float d;\n"
		"varying float R;\n"

		"varying float cornersize;\n"
		"varying float cornersmooth;\n"

		"varying vec3 stretch;\n"
		"varying vec2 sinangle;\n"
		"varying vec2 cosangle;\n"

		"float intersect(vec2 xy) {\n"
		"	float A = dot(xy,xy)+d*d;\n"
		"	float B = 2.0*(R*(dot(xy,sinangle)-d*cosangle.x*cosangle.y)-d*d);\n"
		"	float C = d*d + 2.0*R*d*cosangle.x*cosangle.y;\n"
		"	return (-B-sqrt(B*B-4.0*A*C))/(2.0*A);\n"
		"}\n"

		"vec2 bkwtrans(vec2 xy) {\n"
		"	float c = intersect(xy);\n"
		"	vec2 point = vec2(c)*xy;\n"
		"	point -= vec2(-R)*sinangle;\n"
		"	point /= vec2(R);\n"
		"	vec2 tang = sinangle/cosangle;\n"
		"	vec2 poc = point/cosangle;\n"
		"	float A = dot(tang,tang)+1.0;\n"
		"	float B = -2.0*dot(poc,tang);\n"
		"	float C = dot(poc,poc)-1.0;\n"
		"	float a = (-B+sqrt(B*B-4.0*A*C))/(2.0*A);\n"
		"	vec2 uv = (point-a*sinangle)/cosangle;\n"
		"	float r = FIX(R*acos(a));\n"
		"	return uv*r/sin(r/R);\n"
		"}\n"

		"vec2 transform(vec2 coord) {\n"
		"	coord *= size_texture / size_input;\n"
		"	coord = (coord-vec2(0.5))*aspect*stretch.z+stretch.xy;\n"
		"	return (bkwtrans(coord)/overscan/aspect+vec2(0.5)) * size_input / size_texture;\n"
		"}\n"

		"float corner(vec2 coord) {\n"
		"	coord *= size_texture / size_input;\n"
		"	coord = (coord - vec2(0.5)) * overscan + vec2(0.5);\n"
		"	coord = min(coord, vec2(1.0)-coord) * aspect;\n"
		"	vec2 cdist = vec2(cornersize);\n"
		"	coord = (cdist - min(coord,cdist));\n"
		"	float dist = sqrt(dot(coord,coord));\n"
		"	return clamp((cdist.x-dist)*cornersmooth,0.0, 1.0);\n"
		"}\n"

		"// Calculate the influence of a scanline on the current pixel.\n"
		"// 'distance' is the distance in texture coordinates from the current\n"
		"// pixel to the scanline in question.\n"
		"// 'color' is the colour of the scanline at the horizontal location of\n"
		"// the current pixel.\n"
		"vec4 scanlineWeights(float distance, vec4 color) {\n"
		"	// 'wid' controls the width of the scanline beam, for each RGB\n"
		"	// channel The 'weights' lines basically specify the formula\n"
		"	// that gives you the profile of the beam, i.e. the intensity as\n"
		"	// a function of distance from the vertical center of the\n"
		"	// scanline. In this case, it is gaussian if width=2, and\n"
		"	// becomes nongaussian for larger widths. Ideally this should\n"
		"	// be normalized so that the integral across the beam is\n"
		"	// independent of its width. That is, for a narrower beam\n"
		"	// 'weights' should have a higher peak at the center of the\n"
		"	// scanline than for a wider beam.\n"
		"#ifdef USEGAUSSIAN\n"
		"	vec4 wid = 0.3 + 0.1 * pow(color, vec4(3.0));\n"
		"	vec4 weights = vec4(distance / wid);\n"
		"	return 0.4 * exp(-weights * weights) / wid;\n"
		"#else\n"
		"	vec4 wid = 2.0 + 2.0 * pow(color, vec4(4.0));\n"
		"	vec4 weights = vec4(distance / 0.3);\n"
		"	return 1.4 * exp(-pow(weights * inversesqrt(0.5 * wid), wid)) / (0.6 + 0.2 * wid);\n"
		"#endif\n"
		"}\n"

		"void main() {\n"
		"	// Here's a helpful diagram to keep in mind while trying to\n"
		"	// understand the code:\n"
		"	//\n"
		"	//  |      |      |      |      |\n"
		"	// -------------------------------\n"
		"	//  |      |      |      |      |\n"
		"	//  |  01  |  11  |  21  |  31  | <-- current scanline\n"
		"	//  |      | @    |      |      |\n"
		"	// -------------------------------\n"
		"	//  |      |      |      |      |\n"
		"	//  |  02  |  12  |  22  |  32  | <-- next scanline\n"
		"	//  |      |      |      |      |\n"
		"	// -------------------------------\n"
		"	//  |      |      |      |      |\n"
		"	//\n"
		"	// Each character-cell represents a pixel on the output\n"
		"	// surface, '@' represents the current pixel (always somewhere\n"
		"	// in the bottom half of the current scan-line, or the top-half\n"
		"	// of the next scanline). The grid of lines represents the\n"
		"	// edges of the texels of the underlying texture.\n"

		"	// Texture coordinates of the texel containing the active pixel.\n"
		"#ifdef CURVATURE\n"
		"	vec2 xy = transform(texCoord);\n"
		"#else\n"
		"	vec2 xy = texCoord;\n"
		"#endif\n"
		"	float cval = corner(xy);\n"

		"	// Of all the pixels that are mapped onto the texel we are\n"
		"	// currently rendering, which pixel are we currently rendering?\n"
		"	vec2 ilvec = vec2(0.0,ilfac.y > 1.5 ? mod(float(frame_counter),2.0) : 0.0);\n"
		"	vec2 ratio_scale = (xy * size_texture - vec2(0.5) + ilvec)/ilfac;\n"
		"#ifdef OVERSAMPLE\n"
		"	float filter = fwidth(ratio_scale.y);\n"
		"#endif\n"
		"	vec2 uv_ratio = fract(ratio_scale);\n"

		"	// Snap to the center of the underlying texel.\n"
		"	xy = (floor(ratio_scale)*ilfac + vec2(0.5) - ilvec) / size_texture;\n"

		"	// Calculate Lanczos scaling coefficients describing the effect\n"
		"	// of various neighbour texels in a scanline on the current\n"
		"	// pixel.\n"
		"	vec4 coeffs = PI * vec4(1.0 + uv_ratio.x, uv_ratio.x,"
		"					 1.0 - uv_ratio.x, 2.0 - uv_ratio.x);\n"

		"	// Prevent division by zero.\n"
		"	coeffs = FIX(coeffs);\n"

		"	// Lanczos2 kernel.\n"
		"	coeffs = 2.0 * sin(coeffs) * sin(coeffs / 2.0) / (coeffs * coeffs);\n"

		"	// Normalize.\n"
		"	coeffs /= dot(coeffs, vec4(1.0));\n"

		"	// Calculate the effective colour of the current and next\n"
		"	// scanlines at the horizontal location of the current pixel,\n"
		"	// using the Lanczos coefficients above.\n"
		"	vec4 col = clamp(mat4(\n"
		"			TEX2D(xy + vec2(-one.x, 0.0)),\n"
		"			TEX2D(xy),\n"
		"			TEX2D(xy + vec2(one.x, 0.0)),\n"
		"			TEX2D(xy + vec2(2.0 * one.x, 0.0))) * coeffs,\n"
		"			0.0, 1.0);\n"
		"	vec4 col2 = clamp(mat4(\n"
		"			TEX2D(xy + vec2(-one.x, one.y)),\n"
		"			TEX2D(xy + vec2(0.0, one.y)),\n"
		"			TEX2D(xy + one),\n"
		"			TEX2D(xy + vec2(2.0 * one.x, one.y))) * coeffs,\n"
		"			0.0, 1.0);\n"

		"#ifndef LINEAR_PROCESSING\n"
		"	col = pow(col , vec4(CRTgamma));\n"
		"	col2 = pow(col2, vec4(CRTgamma));\n"
		"#endif\n"

		"	// Calculate the influence of the current and next scanlines on\n"
		"	// the current pixel.\n"
		"	vec4 weights = scanlineWeights(uv_ratio.y, col);\n"
		"	vec4 weights2 = scanlineWeights(1.0 - uv_ratio.y, col2);\n"
		"#ifdef OVERSAMPLE\n"
		"	uv_ratio.y =uv_ratio.y+1.0/3.0*filter;\n"
		"	weights = (weights+scanlineWeights(uv_ratio.y, col))/3.0;\n"
		"	weights2=(weights2+scanlineWeights(abs(1.0-uv_ratio.y), col2))/3.0;\n"
		"	uv_ratio.y =uv_ratio.y-2.0/3.0*filter;\n"
		"	weights=weights+scanlineWeights(abs(uv_ratio.y), col)/3.0;\n"
		"	weights2=weights2+scanlineWeights(abs(1.0-uv_ratio.y), col2)/3.0;\n"
		"#endif\n"
		"	vec3 mul_res = (col * weights + col2 * weights2).rgb * vec3(cval);\n"

		"	// dot-mask emulation:\n"
		"	// Output pixels are alternately tinted green and magenta.\n"
		"	vec3 dotMaskWeights = mix(\n"
		"			vec3(1.0, 0.7, 1.0),\n"
		"			vec3(0.7, 1.0, 0.7),\n"
		"			floor(mod(mod_factor, 2.0))\n"
		"	);\n"

		"	mul_res *= dotMaskWeights;\n"

		"	// Convert the image gamma for display on our output device.\n"
		"	mul_res = pow(mul_res, vec3(1.0 / monitorgamma));\n"

		"	// Color the texel.\n"
		"	vec4 scr = vec4(mul_res, 1.0);\n"

		"	vec4 txt = texture2D(texture_txt, texCoord.xy);\n"

		"	gl_FragColor = mix(scr, txt, txt.a) * gl_Color;\n"
		"}"
	},
};
#undef _SHADERS_CODE_
#endif
