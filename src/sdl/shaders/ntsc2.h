/*****************************************************************************************/
/* NTSC2                                                                                 */
/*****************************************************************************************/
{
	// vertex shader
	NULL,
	// fragment shader
	"#version 120\n"

	"uniform sampler2D texture_scr;\n"
	"uniform vec2 size_texture;\n"
	"uniform float frame_counter;\n"

	"#define TEX2D(c) texture2D(texture_scr,(c))\n"

	"#define PI 3.14159265\n"

	"void main() {\n"
	"	vec2 xy = gl_TexCoord[0].st;\n"

	"	vec2 xyf = fract(xy * size_texture);\n"
	"	vec2 xyp = floor(xy * size_texture)+vec2(0.5);\n"
	"	xy = xyp / size_texture;\n"
	"	xyp.y = -xyp.y-1.0; // fix for inconsistent texture coordinates\n"
	"	float offs = mod(frame_counter,2)/2.0;\n"
	"	vec4 phases = (vec4(0.0,0.25,0.5,0.75) + vec4(xyp.x+xyp.y/2.0+offs)) *4.0*PI/3.0;\n"
	"	vec4 phasesl = (vec4(0.0,0.25,0.5,0.75) + vec4(-1.0+xyp.x+xyp.y/2.0+offs)) *4.0*PI/3.0;\n"
	"	vec4 phasesr = (vec4(0.0,0.25,0.5,0.75) + vec4( 1.0+xyp.x+xyp.y/2.0+offs)) *4.0*PI/3.0;\n"
	"	vec4 phsin = sin(phases);\n"
	"	vec4 phcos = cos(phases);\n"
	"	vec4 phsinl= sin(phasesl);\n"
	"	vec4 phcosl= cos(phasesl);\n"
	"	vec4 phsinr= sin(phasesr);\n"
	"	vec4 phcosr= cos(phasesr);\n"
	"	vec4 phone = vec4(1.0);\n"

	"	vec2 one = 1.0/size_texture;\n"

	"	vec4 c = TEX2D(xy)*2.3-0.65;\n"
	"	vec4 cl= TEX2D(xy + vec2(-one.x,0.0))*2.3-0.65;\n"
	"	vec4 cr= TEX2D(xy + vec2( one.x,0.0))*2.3-0.65;\n"

	"	vec3 yuva = vec3((dot(cl.zw,phone.zw)+dot(c.xyz,phone.xyz)+0.5*(cl.y+c.w))/6.0, (dot(cl.zw,phsinl.zw)+dot(c.xyz,phsin.xyz)+0.5*(cl.y*phsinl.y+c.w*phsin.w))/3.0, (dot(cl.zw,phcosl.zw)+dot(c.xyz,phcos.xyz)+0.5*(cl.y*phcosl.y+c.w*phcos.w))/3.0);\n"

	"	vec3 yuvb = vec3((cl.w*phone.w+dot(c.xyzw,phone.xyzw)+0.5*(cl.z+cr.x))/6.0, (cl.w*phsinl.w+dot(c.xyzw,phsin.xyzw)+0.5*(cl.z*phsinl.z+cr.x*phsinr.x))/3.0, (cl.w*phcosl.w+dot(c.xyzw,phcos.xyzw)+0.5*(cl.z*phcosl.z+cr.x*phcosr.x))/3.0);\n"

	"	vec3 yuvc = vec3((cr.x*phone.x+dot(c.xyzw,phone.xyzw)+0.5*(cl.w+cr.y))/6.0, (cr.x*phsinr.x+dot(c.xyzw,phsin.xyzw)+0.5*(cl.w*phsinl.w+cr.y*phsinr.y))/3.0, (cr.x*phcosr.x+dot(c.xyzw,phcos.xyzw)+0.5*(cl.w*phcosl.w+cr.y*phcosr.y))/3.0);\n"

	"	vec3 yuvd = vec3((dot(cr.xy,phone.xy)+dot(c.yzw,phone.yzw)+0.5*(c.x+cr.z))/6.0, (dot(cr.xy,phsinr.xy)+dot(c.yzw,phsin.yzw)+0.5*(c.x*phsin.x+cr.z*phsinr.z))/3.0, (dot(cr.xy,phcosr.xy)+dot(c.yzw,phcos.yzw)+0.5*(c.x*phcos.x+cr.z*phcosr.z))/3.0);\n"

	"	mat3x3 yuv2rgb = mat3x3(1.0, 1.0, 1.0,"
	"			0.0,-0.39465,2.03211,"
	"			1.13983,-0.58060,0.0);\n"

	"	vec4 scr;\n"

	"	if (xyf.x < 0.25)\n"
	"		scr = vec4(yuv2rgb*yuva, 0.0);\n"
	"	else if (xyf.x < 0.5)\n"
	"		scr = vec4(yuv2rgb*yuvb, 0.0);\n"
	"	else if (xyf.x < 0.75)\n"
	"		scr = vec4(yuv2rgb*yuvc, 0.0);\n"
	"	else\n"
	"		scr = vec4(yuv2rgb*yuvd, 0.0);\n"

	"	gl_FragColor = scr * gl_Color;\n"
	"}"
},
