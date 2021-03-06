"float3 col = tex2D(s0, vertex.tex).rgb;\n"
"float3 yiq = rgb2yiq(col);\n"
"\n"
"#if defined(TWO_PHASE)\n"
"float chroma_phase = PI * (fmod(vertex.pix_no.y, 2.0) + IN.frame_count);\n"
"#elif defined(THREE_PHASE)\n"
"float chroma_phase = 0.6667 * PI * (fmod(vertex.pix_no.y, 3.0) + IN.frame_count);\n"
"#endif\n"
"\n"
"float mod_phase = chroma_phase + vertex.pix_no.x * CHROMA_MOD_FREQ;\n"
"\n"
"float i_mod = cos(mod_phase);\n"
"float q_mod = sin(mod_phase);\n"
"\n"
"yiq.yz *= float2(i_mod, q_mod); // Modulate.\n"
"yiq = mul(mix_mat, yiq); // Cross-talk.\n"
"yiq.yz *= float2(i_mod, q_mod); // Demodulate.\n"
"return float4(yiq, 1.0);\n"
"\n"

