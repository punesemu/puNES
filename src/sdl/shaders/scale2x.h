/*****************************************************************************************/
/* SCALE2X                                                                               */
/*****************************************************************************************/
{
	// vertex shader
	"uniform vec2 size_screen_emu;\n"
	"uniform vec2 size_texture;\n"
	"uniform float pixel_aspect_ratio;\n"

	"varying vec4 v_texCoord[5];\n"

	"void main() {\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"	gl_FrontColor = gl_Color;\n"

	"	vec2 dx = vec2((1.0 / size_texture.x), 0);\n"
	"	vec2 dy = vec2(0, (1.0 / size_texture.y));\n"

	"	v_texCoord[0]    = gl_MultiTexCoord0;     // center\n"
	"	v_texCoord[1].xy = v_texCoord[0].xy - dx; // left\n"
	"	v_texCoord[2].xy = v_texCoord[0].xy + dx; // right\n"
	"	v_texCoord[3].xy = v_texCoord[0].xy - dy; // top\n"
	"	v_texCoord[4].xy = v_texCoord[0].xy + dy; // bottom\n"
	"}",
	// fragment shader
	"uniform vec2 size_screen_emu;\n"
	"uniform vec2 size_video_mode;\n"
	"uniform vec2 size_texture;\n"
	"uniform float pixel_aspect_ratio;\n"

	"uniform sampler2D texture_scr;\n"

	"varying vec4 v_texCoord[5];\n"

	"void main() {\n"
	"	vec3 E = texture2D(texture_scr, v_texCoord[0].xy).xyz;\n"
	"	vec3 D = texture2D(texture_scr, v_texCoord[1].xy).xyz;\n"
	"	vec3 F = texture2D(texture_scr, v_texCoord[2].xy).xyz;\n"
	"	vec3 H = texture2D(texture_scr, v_texCoord[3].xy).xyz;\n"
	"	vec3 B = texture2D(texture_scr, v_texCoord[4].xy).xyz;\n"

	"	vec4 scr = vec4(1.0);\n"

	"	if ((D - F) * (H - B) == vec3(0.0)) {\n"
	"		scr.xyz = E;\n"
	"	} else {"
	"		vec2 p = fract(v_texCoord[0].xy * size_texture.xy);\n"
	"		vec3 tmp1 = p.x < 0.5 ? D : F;\n"
	"		vec3 tmp2 = p.y < 0.5 ? H : B;\n"
	"		scr.xyz = ((tmp1 - tmp2) != vec3(0.0)) ? E : tmp1;\n"
	"	}\n"

	"	gl_FragColor = scr * gl_Color;\n"
	"}",

	/*
	// fragment shader
	"uniform vec4 size;\n"
	"uniform sampler2D texture_scr;\n"
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
	"	sel  = fract(v_texCoord[0].xy * size.xy);           // where are we (E0-E3)?\n"
	"                                                       // E0 is default\n"
	"	if(sel.y >= 0.5)  {tmp=colB;colB=colH;colH=tmp;}    // E1 (or E3): swap B and H\n"
	"	if(sel.x >= 0.5)  {tmp=colF;colF=colD;colD=tmp;}    // E2 (or E3): swap D and F\n"
	"	if((colB == colD) && (colB != colF) && (colD != colH)) { // do the Scale2x rule\n"
	"		col=colD;\n"
	"	}\n"
	"	scr = col;\n"
	"	gl_FragColor = scr;\n"
	"}"
	*/
},
