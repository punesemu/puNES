/*****************************************************************************************/
/* don't BLOOM                                                                           */
/*****************************************************************************************/
{
	// vertex shader
	"struct VsOutput {\n"
	"	float4 Position : POSITION;\n"
	"	float2 TexCoord : TEXCOORD0;\n"
	"};\n"

	"float4x4 m_world_view_projection;"

	"VsOutput Vs(float4 position : POSITION, float2 texCoord : TEXCOORD0) {\n"
	"	VsOutput output;\n"
	"	output.Position = mul(position, m_world_view_projection);\n"
	"	output.TexCoord = texCoord;\n"
	"	return output;\n"
	"}",
	// pixel shader
	"float2 size_video_mode;\n"
	"float2 size_texture;\n"
	"float  param;\n"
	"float  pixel_aspect_ratio;\n"

	"texture texture_scr;\n"
	"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

	"float dist(float2 coord, float2 source) {\n"
	"	float2 delta = coord - source;\n"
	"	return sqrt(dot(delta, delta));\n"
	"}\n"

	"float color_bloom(float3 color) {\n"
	"	float shine;\n"
	"	if (param == 0.0) {\n"
	"		shine = 0.05;\n"
	"	} else {\n"
	"		shine = 0.25;\n"
	"	}\n"

	"	const float3 gray_coeff = float3(0.30, 0.59, 0.11);\n"
	"	float bright = dot(color, gray_coeff);\n"
	"	return lerp(1.0 + shine, 1.0 - shine, bright);\n"
	"}\n"

	"float2 fract(float2 x) {\n"
	"	return x - floor(x);\n"
	"}\n"

	"float3 lookup(float offset_x, float offset_y, float2 coord, float2 pix) {\n"
	"	float gamma;\n"
	"	float factor_delta;\n"
	"	if (param == 0.0) {\n"
	"		gamma = 2.4;\n"
	"		factor_delta = 0.4;\n"
	"	} else {\n"
	"		gamma = 1.4;\n"
	"		factor_delta = 0.10;\n"
	"	}\n"

	"	float2 offset = float2(offset_x, offset_y);\n"
	"	float3 color = tex2D(s0, coord).rgb;\n"
	//"	float delta = dist(fract(pix), offset + float2(0.5, 0.5));\n"
	"	float delta = dist(fract(pix), offset + float2(factor_delta, factor_delta));\n"
	"	return color * exp(-gamma * delta * color_bloom(color));\n"
	"}\n"

	"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
	"	float dx;\n"
	"	float dy;\n"
	"	float2 pixel_no;\n"
	"	float blend;\n"

	"	if (param == 0.0) {\n"
	"		dx = (1.0 / size_video_mode.x);\n"
	"		dy = (1.0 / size_video_mode.y);\n"
	"		pixel_no = texCoord * float2(size_texture.x * pixel_aspect_ratio, size_texture.y);\n"
	"		blend = 0.65;\n"
	"	} else {\n"
	"		dx = (1.0 / (size_texture.x * pixel_aspect_ratio));\n"
	"		dy = (1.0 / size_texture.y);\n"
	"		pixel_no = texCoord;\n"
	"		blend = 0.10;\n"
	"	}\n"

	"	float2 c00 = texCoord + float2(-dx, -dy);\n"
	"	float2 c10 = texCoord + float2(  0, -dy);\n"
	"	float2 c20 = texCoord + float2( dx, -dy);\n"
	"	float2 c01 = texCoord + float2(-dx,   0);\n"
	"	float2 c11 = texCoord + float2(  0,   0);\n"
	"	float2 c21 = texCoord + float2( dx,   0);\n"
	"	float2 c02 = texCoord + float2(-dx,  dy);\n"
	"	float2 c12 = texCoord + float2(  0,  dy);\n"
	"	float2 c22 = texCoord + float2( dx,  dy);\n"

	"	float3 mid_color = lookup(0.0, 0.0, c11, pixel_no);\n"
	"	float3 color = float3(0.0, 0.0, 0.0);\n"
	"	color += lookup(-1.0, -1.0, c00, pixel_no);\n"
	"	color += lookup( 0.0, -1.0, c10, pixel_no);\n"
	"	color += lookup( 1.0, -1.0, c20, pixel_no);\n"
	"	color += lookup(-1.0,  0.0, c01, pixel_no);\n"
	"	color += mid_color;\n"
	"	color += lookup( 1.0,  0.0, c21, pixel_no);\n"
	"	color += lookup(-1.0,  1.0, c02, pixel_no);\n"
	"	color += lookup( 0.0,  1.0, c12, pixel_no);\n"
	"	color += lookup( 1.0,  1.0, c22, pixel_no);\n"
	"	float3 out_color = lerp(1.2 * mid_color, color, blend);\n"

	"	float4 scr = float4(out_color, 1.0);\n"

	"	return scr;\n"
	"}"
},
