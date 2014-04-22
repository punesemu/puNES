	"	float sharpness = 0.8;\n"

	"	if (full_interpolation == 1.0) {\n"
	"		sharpness = 0.0;\n"
	"	}\n"

	"	vec2 scale = (size_video_mode / vec2(pixel_aspect_ratio, 1.0)) / size_screen_emu;\n"
	"	vec2 interp = (scale - mix(scale, vec2(1.0), vec2(sharpness))) / (scale * 2.0);\n"
	"	vec2 pnt = v_texCoord.xy;\n"

	"	interp = clamp(interp, 0.0, 1.0);\n"
	"	pnt = (pnt * size_texture) + 0.5;\n"

	"	vec2 i = floor(pnt);\n"
	"	vec2 f = pnt - i;\n"

	"	f = (f - interp) / (1.0 - interp * 2.0);\n"
	"	f = clamp(f, 0.0, 1.0);\n"

	"	pnt = i + f;\n"
	"	pnt = (pnt - 0.5) / size_texture;\n"
