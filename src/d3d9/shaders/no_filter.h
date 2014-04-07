/*****************************************************************************************/
/* NO_FILTER                                                                             */
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

	"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
	"	float4 scr = tex2D(s0, texCoord);\n"
	"	return scr;\n"
	"}"
},
