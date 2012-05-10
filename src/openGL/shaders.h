/*
 * shaders.h
 *
 *  Created on: 09/mag/2012
 *      Author: fhorse
 */

#ifndef SHADERS_H_
#define SHADERS_H_

typedef struct {
	const GLchar *vert;
	const GLchar *frag;
} _shader_routine;

struct {
	GLuint program;
	GLuint vert;
	GLuint frag;

	const GLchar *vs;
	const GLchar *fs;
} shader;

GLfloat OGL2Size[4];
GLfloat OGL2Param[4];
GLuint vs, /* Vertex Shader */
	   fs, /* Fragment Shader */
	   sp; /* Shader Program */
GLint baseImageLoc;
GLint OGL2SizeLoc;
GLint OGL2ParamLoc;

#ifdef _SHADERS_CODE_
static _shader_routine shader_routine[2] = {
	/* NOFILTER */
	{
		// vertex shader
		"varying vec4 vTexCoord;\n"
		"void main(void)\n"
		"{\n"
		"	vTexCoord = gl_MultiTexCoord0;\n"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
		"}",
		// fragment shader
		"uniform sampler2D OGL2Texture;\n"
		"varying vec4 vTexCoord;\n"
		"void main(void)\n"
		"{\n"
		"	gl_FragColor = texture2DProj(OGL2Texture, vTexCoord).bgra;\n"
		"}"
	},
	/* SCALE2X */
	{
		// vertex shader
		"varying vec4 vTexCoord[5];\n"
		"uniform vec4 OGL2Size;\n"
		"uniform vec4 OGL2Param;\n"
		"void main()\n"
		"{\n"
		"	vec4 offsetx;\n"
		"	vec4 offsety;\n"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
		"	offsetx.x = OGL2Param.x;	// setup one x/y texel offset\n"
		"	offsetx.y = 0.0;			// we could also use \"1.0/OGL2Size.x (y)\"\n"
		"								// than it wouldn't\n"
		"	offsetx.w = 0.0;			// be dependand on the \"shader effect level\" setting...\n"
		"	offsetx.z = 0.0;			// but more choice is usual better, eh?\n"
		"	offsety.y = OGL2Param.y;\n"
		"	offsety.x = 0.0;\n"
		"	offsety.w = 0.0;\n"
		"	offsety.z = 0.0;\n"
		"	vTexCoord[0]    = gl_MultiTexCoord0;		// center\n"
		"	vTexCoord[1]    = vTexCoord[0] - offsetx;	// left\n"
		"	vTexCoord[2]    = vTexCoord[0] + offsetx;	// right\n"
		"	vTexCoord[3]    = vTexCoord[0] - offsety;	// top\n"
		"	vTexCoord[4]    = vTexCoord[0] + offsety;	// bottom\n"
		"}",
		// fragment shader
		"varying vec4 vTexCoord[5];\n"
		"uniform vec4 OGL2Size;\n"
		"uniform vec4 OGL2Param;\n"
		"uniform sampler2D OGL2Texture;\n"
		"void main()\n"
		"{\n"
		"	vec4 colD,colF,colB,colH,col,tmp;\n"
		"	vec2 sel;\n"
		"	col  = texture2DProj(OGL2Texture, vTexCoord[0]);	// central (can be E0-E3)\n"
		"	colD = texture2DProj(OGL2Texture, vTexCoord[1]);	// D (left)\n"
		"	colF = texture2DProj(OGL2Texture, vTexCoord[2]);	// F (right)\n"
		"	colB = texture2DProj(OGL2Texture, vTexCoord[3]);	// B (top)\n"
		"	colH = texture2DProj(OGL2Texture, vTexCoord[4]);	// H (bottom)\n"
		"	sel=fract(vTexCoord[0].xy * OGL2Size.xy);			// where are we (E0-E3)?\n"
		"														// E0 is default\n"
		"	if(sel.y >= 0.5)  {tmp=colB;colB=colH;colH=tmp;}	// E1 (or E3): swap B and H\n"
		"	if(sel.x >= 0.5)  {tmp=colF;colF=colD;colD=tmp;}	// E2 (or E3): swap D and F\n"
		"	if((colB == colD) && (colB != colF) && (colD != colH)) {	// do the Scale2x rule\n"
		"		col=colD;\n"
		"	}\n"
		"	gl_FragColor = col;\n"
		"}"
	},
};
#endif

#endif /* SHADERS_H_ */
