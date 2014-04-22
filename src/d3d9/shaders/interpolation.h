	"	float sharpness = 0.8;\n"

	"	if (full_interpolation == 1.0) {\n"
	"		sharpness = 0.0;\n"
	"	}\n"

	"	float2 scale = (size_video_mode / float2(pixel_aspect_ratio, 1.0)) / size_screen_emu;\n"
	"	float2 interp = (scale - lerp(scale, 1.0, sharpness)) / (scale * 2.0);\n"
	"	float2 pnt = texCoord.xy;\n"

	"	interp = saturate(interp);\n"
	"	pnt = (pnt * size_texture) + 0.5;\n"

	"	float2 i = floor(pnt);\n"
	"	float2 f = pnt - i;\n"

	"	f = (f - interp) / (1.0 - interp * 2.0);\n"
	"	f = saturate(f);\n"

	"	pnt = i + f;\n"
	"	pnt = (pnt - 0.5) / size_texture;\n"
