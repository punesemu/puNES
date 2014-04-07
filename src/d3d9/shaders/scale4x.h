/*****************************************************************************************/
/* SCALE4X                                                                               */
/*****************************************************************************************/
{
	// vertex shader
	"struct VsOutput {\n"
	"	float4 Position : POSITION;\n"
	"	float2 TexCoord : TEXCOORD0;\n"
	"};\n"

	"float4x4 m_world_view_projection : WORLDVIEWPROJECTION;\n"

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

	"float2 fract(float2 x) {\n"
	"	return x - floor(x);\n"
	"}\n"

	"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
	"	float x = 0.5 * (1.0 / size_texture.x);\n"
	"	float y = 0.5 * (1.0 / size_texture.y);\n"

	"	float2 dg1 = float2( x, y);\n"
	"	float2 dg2 = float2(-x, y);\n"
	"	float2 sd1 = dg1 * 0.5;\n"
	"	float2 sd2 = dg2 * 0.5;\n"

	"	float4 v_texCoord[7];\n"

	"	v_texCoord[0] = float4(texCoord, 0.0, 0.0);\n"
	"	v_texCoord[1].xy = v_texCoord[0].xy - sd1;\n"
	"	v_texCoord[2].xy = v_texCoord[0].xy - sd2;\n"
	"	v_texCoord[3].xy = v_texCoord[0].xy + sd1;\n"
	"	v_texCoord[4].xy = v_texCoord[0].xy + sd2;\n"
	"	v_texCoord[5].xy = v_texCoord[0].xy - dg1;\n"
	"	v_texCoord[6].xy = v_texCoord[0].xy + dg1;\n"
	"	v_texCoord[5].zw = v_texCoord[0].xy - dg2;\n"
	"	v_texCoord[6].zw = v_texCoord[0].xy + dg2;\n"

	"	float3 c  = tex2D(s0, v_texCoord[0].xy).rgb;\n"
	"	float3 i1 = tex2D(s0, v_texCoord[1].xy).rgb;\n"
	"	float3 i2 = tex2D(s0, v_texCoord[2].xy).rgb;\n"
	"	float3 i3 = tex2D(s0, v_texCoord[3].xy).rgb;\n"
	"	float3 i4 = tex2D(s0, v_texCoord[4].xy).rgb;\n"
	"	float3 o1 = tex2D(s0, v_texCoord[5].xy).rgb;\n"
	"	float3 o3 = tex2D(s0, v_texCoord[6].xy).rgb;\n"
	"	float3 o2 = tex2D(s0, v_texCoord[5].zw).rgb;\n"
	"	float3 o4 = tex2D(s0, v_texCoord[6].zw).rgb;\n"

	"	float3 dt = float3(1.0, 1.0, 1.0);\n"

	"	float ko1 = dot(abs(o1 - c), dt);\n"
	"	float ko2 = dot(abs(o2 - c), dt);\n"
	"	float ko3 = dot(abs(o3 - c), dt);\n"
	"	float ko4 = dot(abs(o4 - c), dt);\n"

	"	float k1 = min(dot(abs(i1 - i3), dt), dot(abs(o1 - o3), dt));\n"
	"	float k2 = min(dot(abs(i2 - i4), dt), dot(abs(o2 - o4), dt));\n"

	"	float w1 = k2; if (ko3 < ko1) { w1 = 0.0; };\n"
	"	float w2 = k1; if (ko4 < ko2) { w2 = 0.0; };\n"
	"	float w3 = k2; if (ko1 < ko3) { w3 = 0.0; };\n"
	"	float w4 = k1; if (ko2 < ko4) { w4 = 0.0; };\n"

	"	float4 scr = float4(((w1 * o1) + (w2 * o2) + (w3 * o3) + (w4 * o4) + (0.0001 * c)) /"
	"			(w1 + w2 + w3 + w4 + 0.0001), 1.0);\n"

	"	return scr;\n"
	"}"
},
