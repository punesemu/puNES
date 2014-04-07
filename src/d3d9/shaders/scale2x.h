/*****************************************************************************************/
/* SCALE2X                                                                               */
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
	"	float x = (1.0 / size_texture.x);\n"
	"	float y = (1.0 / size_texture.y);\n"
	"	float2 dx = float2(x, 0.0);\n"
	"	float2 dy = float2(0.0, y);\n"

	"	float2 v_texCoord[5];\n"

	"	v_texCoord[0] = texCoord;           // center\n"
	"	v_texCoord[1] = v_texCoord[0] - dx; // left\n"
	"	v_texCoord[2] = v_texCoord[0] + dx; // right\n"
	"	v_texCoord[3] = v_texCoord[0] - dy; // top\n"
	"	v_texCoord[4] = v_texCoord[0] + dy; // bottom\n"

	"	float3 E = tex2D(s0, v_texCoord[0]).rgb;\n"
	"	float3 D = tex2D(s0, v_texCoord[1]).rgb;\n"
	"	float3 F = tex2D(s0, v_texCoord[2]).rgb;\n"
	"	float3 H = tex2D(s0, v_texCoord[3]).rgb;\n"
	"	float3 B = tex2D(s0, v_texCoord[4]).rgb;\n"

	"	float4 scr;\n"

	"	if (any((D - F) * (H - B))) {\n"
	"		float2 p = fract(v_texCoord[0] * size_texture);\n"
	"		float3 tmp1 = p.x < 0.5 ? D : F;\n"
	"		float3 tmp2 = p.y < 0.5 ? H : B;\n"
	"		scr = any(tmp1 - tmp2) ? float4(E, 1.0) : float4(tmp1, 1.0);\n"
	"	} else {"
	"		scr = float4(E, 1.0);\n"
	"	}\n"

	"	return scr;\n"
	"}"
},
