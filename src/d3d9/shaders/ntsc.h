/*****************************************************************************************/
/* NTSC                                                                                  */
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
	"float2 size_screen_emu;\n"
	"float2 size_video_mode;\n"
	"float2 size_texture;\n"

	"texture texture_scr;\n"
	"sampler2D s0 = sampler_state { Texture = <texture_scr>; };\n"

	"static float3x3 rgb2yuv = float3x3("
	"	 0.299  , 0.587  , 0.114  ,"
	"	-0.14713,-0.28886, 0.436  ,"
	"	 0.615  ,-0.51499,-0.10001 "
	");\n"
	"static	float3x3 yuv2rgb = float3x3("
	"	1.0    , 0.0    , 1.13983,"
	"	1.0    ,-0.39465,-0.58060,"
	"	1.0    , 2.03211, 0.0     "
	");\n"

	"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"

	"	float4 wid = float4(3.0, 3.0, 3.0, 3.0);\n"
	"	float4 c1 = float4(exp(float4(-1.0, -1.0, -1.0, -1.0) / wid / wid));\n"
	"	float4 c2 = float4(exp(float4(-4.0, -4.0, -4.0, -4.0) / wid / wid));\n"
	"	float4 c3 = float4(exp(float4(-9.0, -9.0, -9.0, -9.0) / wid / wid));\n"
	"	float4 c4 = float4(exp(float4(-16.0, -16.0, -16.0, -16.0) / wid / wid));\n"
	"	float4 norm = 1.0 / (float4(1.0, 1.0, 1.0, 1.0) + float4(2.0, 2.0, 2.0, 2.0) *"
	"			(c1 + c2 + c3 + c4));\n"

	"	float one_x = (1.0 / size_texture.x);\n"

	"	float4 sum = float4(0.0, 0.0, 0.0, 0.0);\n"
	"	sum += tex2D(s0, texCoord + float2(-4.0 * one_x, 0.0)) * c4;\n"
	"	sum += tex2D(s0, texCoord + float2(-3.0 * one_x, 0.0)) * c3;\n"
	"	sum += tex2D(s0, texCoord + float2(-2.0 * one_x, 0.0)) * c2;\n"
	"	sum += tex2D(s0, texCoord + float2(-1.0 * one_x, 0.0)) * c1;\n"
	"	sum += tex2D(s0, texCoord);\n"
	"	sum += tex2D(s0, texCoord + float2(+1.0 * one_x, 0.0)) * c1;\n"
	"	sum += tex2D(s0, texCoord + float2(+2.0 * one_x, 0.0)) * c2;\n"
	"	sum += tex2D(s0, texCoord + float2(+3.0 * one_x, 0.0)) * c3;\n"
	"	sum += tex2D(s0, texCoord + float2(+4.0 * one_x, 0.0)) * c4;\n"

	"	float y = mul(rgb2yuv, tex2D(s0, texCoord).rgb).x;\n"
	"	float2 uv = mul(rgb2yuv, float3(sum.rgb * norm.rgb)).yz;\n"

	"	float4 scr = float4(mul(yuv2rgb, float3(y, uv)), 1.0);\n"

	"	return scr;\n"
	"}"
},
