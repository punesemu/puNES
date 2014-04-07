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

	"	gl_FragColor = scr * gl_Color;\n"
	"}"
},
