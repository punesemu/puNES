/*****************************************************************************************/
/* NO_FILTER                                                                             */
/*****************************************************************************************/
{
	// vertex shader
	"varying vec4 v_texCoord;\n"

	"void main(void) {\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"	gl_FrontColor = gl_Color;\n"
	"	v_texCoord = gl_MultiTexCoord0;\n"
	"}",
	// fragment shader
	"uniform sampler2D texture_scr;\n"

	"varying vec4 v_texCoord;\n"

	"void main(void) {\n"
	"	vec4 scr = texture2D(texture_scr, v_texCoord.xy);\n"
	"	gl_FragColor = scr * gl_Color;\n"
	"}"
},
