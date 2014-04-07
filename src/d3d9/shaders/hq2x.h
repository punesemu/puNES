/*****************************************************************************************/
/* HQ2X                                                                                  */
/*****************************************************************************************/
{
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

	"static const float mx = 0.325;     // start smoothing wt.\n"
	"static const float k = -0.250;     // wt. decrease factor\n"
	"static const float max_w = 0.25;   // max filter weigth\n"
	"static const float min_w =-0.05;   // min filter weigth\n"
	"static const float lum_add = 0.25; // effects smoothing\n"

	"float4 Ps(float2 texCoord : TEXCOORD0) : COLOR {\n"
	"	float x = 0.5 * (1.0 / size_texture.x);\n"
	"	float y = 0.5 * (1.0 / size_texture.y);\n"

	"	float2 dx = float2(x, 0.0);\n"
	"	float2 dy = float2(0.0, y);\n"
	"	float2 dg1 = float2( x, y);\n"
	"	float2 dg2 = float2(-x, y);\n"

	"	float4 v_texCoord[5];\n"

	"	v_texCoord[0] = float4(texCoord.xy, 0.0, 0.0);\n"
	"	v_texCoord[1].xy = v_texCoord[0].xy - dg1;\n"
	"	v_texCoord[1].zw = v_texCoord[0].xy - dy;\n"
	"	v_texCoord[2].xy = v_texCoord[0].xy - dg2;\n"
	"	v_texCoord[2].zw = v_texCoord[0].xy + dx;\n"
	"	v_texCoord[3].xy = v_texCoord[0].xy + dg1;\n"
	"	v_texCoord[3].zw = v_texCoord[0].xy + dy;\n"
	"	v_texCoord[4].xy = v_texCoord[0].xy + dg2;\n"
	"	v_texCoord[4].zw = v_texCoord[0].xy - dx;\n"

	"	float3 c00 = tex2D(s0, v_texCoord[1].xy).xyz;\n"
	"	float3 c10 = tex2D(s0, v_texCoord[1].zw).xyz;\n"
	"	float3 c20 = tex2D(s0, v_texCoord[2].xy).xyz;\n"
	"	float3 c01 = tex2D(s0, v_texCoord[4].zw).xyz;\n"
	"	float3 c11 = tex2D(s0, v_texCoord[0].xy).xyz;\n"
	"	float3 c21 = tex2D(s0, v_texCoord[2].zw).xyz;\n"
	"	float3 c02 = tex2D(s0, v_texCoord[4].xy).xyz;\n"
	"	float3 c12 = tex2D(s0, v_texCoord[3].zw).xyz;\n"
	"	float3 c22 = tex2D(s0, v_texCoord[3].xy).xyz;\n"

	"	float3 dt = float3(1.0, 1.0, 1.0);\n"

	"	float md1 = dot(abs(c00 - c22), dt);\n"
	"	float md2 = dot(abs(c02 - c20), dt);\n"

	"	float w1 = dot(abs(c22 - c11), dt) * md2;\n"
	"	float w2 = dot(abs(c02 - c11), dt) * md1;\n"
	"	float w3 = dot(abs(c00 - c11), dt) * md2;\n"
	"	float w4 = dot(abs(c20 - c11), dt) * md1;\n"

	"	float t1 = w1 + w3;\n"
	"	float t2 = w2 + w4;\n"
	"	float ww = max(t1, t2) + 0.0001;\n"

	"	c11 = (w1 * c00 + w2 * c20 + w3 * c22 + w4 * c02 + ww * c11) / (t1 + t2 + ww);\n"

	"	float lc1 = k / (0.12 * dot(c10 + c12 + c11, dt) + lum_add);\n"
	"	float lc2 = k / (0.12 * dot(c01 + c21 + c11, dt) + lum_add);\n"

	"	w1 = clamp(lc1 * dot(abs(c11 - c10), dt) + mx, min_w, max_w);\n"
	"	w2 = clamp(lc2 * dot(abs(c11 - c21), dt) + mx, min_w, max_w);\n"
	"	w3 = clamp(lc1 * dot(abs(c11 - c12), dt) + mx, min_w, max_w);\n"
	"	w4 = clamp(lc2 * dot(abs(c11 - c01), dt) + mx, min_w, max_w);\n"

	"	float4 scr = float4(w1 * c10 + w2 * c21 + w3 * c12 + w4 * c01 +"
	"			  (1.0 - w1 - w2 - w3 - w4) * c11, 1.0);\n"

	"	return scr;\n"
	"}"
},
