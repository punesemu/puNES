/*****************************************************************************************/
/* TOON (per funzionare dovrei usare una luce)                                           */
/*****************************************************************************************/
{
	// vertex shader
	"varying vec3 vNormal;\n"
	"varying vec3 vVertex;\n"

	"void main(void) {\n"
	"	gl_FrontColor = gl_Color;\n"
	"	gl_Position = ftransform();\n"
	"	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
	"	vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);\n"
	"	vNormal = normalize(gl_NormalMatrix * gl_Normal);\n"
	"}",
	// fragment shader
	"varying vec3 vNormal;\n"
	"varying vec3 vVertex;\n"

	"uniform float silhouetteThreshold;\n"

	"uniform sampler2D texture_scr;\n"

	"void main (void) {\n"

	"	vec4 materialColor = gl_FrontMaterial.diffuse;\n"

	"	vec4 silhouetteColor = vec4(0.0, 0.0, 0.0, 1.0);\n"

	"	vec4 specularColor = gl_FrontMaterial.specular;\n"

	"	vec3 eyePos = normalize(-vVertex);\n"
	"	vec3 lightPos = gl_LightSource[0].position.xyz;\n"

	"	vec3 Normal = vNormal; //normalize(vNormal);\n"
	"	vec3 EyeVert = normalize(eyePos - vVertex);\n"
	"	vec3 LightVert = normalize(lightPos - vVertex);\n"
	"	vec3 EyeLight = normalize(LightVert+EyeVert);\n"
	"	vec4 texture = texture2D(texture_scr,gl_TexCoord[0].st);\n"

	"	vec4 scr;\n"

	"	float sil = max(dot(Normal,EyeVert), 0.0);\n"
	"	if( sil < silhouetteThreshold )\n"
	"		scr = silhouetteColor;\n"
	"	else {\n"
	"		scr = materialColor*texture;\n"

	"		float spec = pow(max(dot(Normal,EyeLight),0.0), 5.0);\n"
	"		if( spec < 0.05 )\n"
	"			scr *= 0.9;\n"
	"		else\n"
	"			scr = specularColor*texture;\n"

	"		float diffuse = max(dot(Normal,LightVert),0.0);\n"
	"		if( diffuse < 0.3 )\n"
	"			scr *=0.8;\n"
	"	}\n"

	"	gl_FragColor = scr * gl_Color;\n"

	"}"
},
