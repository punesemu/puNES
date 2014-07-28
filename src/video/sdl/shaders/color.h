/*****************************************************************************************/
/* COLOR                                                                                 */
/*****************************************************************************************/
{
	// vertex shader
	"void main(void) {\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"	gl_FrontColor = gl_Color;\n"
	"}",
	// fragment shader
	"void main(void) {\n"
	"	gl_FragColor = gl_Color;\n"
	"}"
},
