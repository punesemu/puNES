/*****************************************************************************************/
/* Pixellate                                                                             */
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

	"#define round_1(x) floor((x) + 0.5)\n"

	"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
	"	float2 texelSize = (1.0 / size_texture);\n"

	"	float2 range = float2(abs(ddx(texCoord.x)), abs(ddy(texCoord.y)));\n"
	"	range = (range / 2.0) * 0.999;\n"

	"	float left   = texCoord.x - range.x;\n"
	"	float top    = texCoord.y + range.y;\n"
	"	float right  = texCoord.x + range.x;\n"
	"	float bottom = texCoord.y - range.y;\n"

	"	float4 topLeftColor     = tex2D(s0, float2(left, top));\n"
	"	float4 bottomRightColor = tex2D(s0, float2(right, bottom));\n"
	"	float4 bottomLeftColor  = tex2D(s0, float2(left, bottom));\n"
	"	float4 topRightColor    = tex2D(s0, float2(right, top));\n"

	"	float2 border = clamp(round_1(texCoord / texelSize) * texelSize,"
	"			float2(left, bottom), float2(right, top));\n"

	"	float totalArea = 4.0 * range.x * range.y;\n"

	"	float4 scr;\n"
	"	scr  = ((border.x - left) * (top - border.y) / totalArea) * topLeftColor;\n"
	"	scr += ((right - border.x) * (border.y - bottom) / totalArea) * bottomRightColor;\n"
	"	scr += ((border.x - left) * (border.y - bottom) / totalArea) * bottomLeftColor;\n"
	"	scr += ((right - border.x) * (top - border.y) / totalArea) * topRightColor;\n"

	"	return scr;\n"
	"}"
},
