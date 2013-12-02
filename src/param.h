/*
 * param.h
 *
 *  Created on: 01/ago/2011
 *      Author: fhorse
 */

#ifndef PARAM_H_
#define PARAM_H_

#define param_search(start, buffer, param, cmd)\
{\
	BYTE index;\
	for(index = start; index < LENGTH(param); index++) {\
		if (strcmp(buffer, param[index].sname) == 0) {\
			cmd;\
			break;\
		}\
	}\
}
#define _param_num_search(buffer, param, round, td, calc, max)\
{\
	WORD tmp = atoi(buffer);\
	if (round > 0) {\
		tmp = emu_round_WORD(tmp, round);\
	}\
	param = ((td) tmp) calc;\
	if (param > max) {\
		param = max;\
	}\
}
#define param_double_search(buffer, param, round)\
	_param_num_search(buffer, param, round, double, / 100.0f, 1.0f)
#define param_apu_channel_search(start, buffer, param, ch)\
{\
	BYTE index;\
	char *c = strtok(buffer, ",");\
	for(index = start; index < LENGTH(param); index++) {\
		if (strcmp(c, param[index].sname) == 0) {\
			cfg_from_file.apu.channel[ch] = index;\
			if ((c = strtok(NULL, ",")) != NULL) {\
				param_double_search(c, cfg_from_file.apu.volume[ch], 0);\
			}\
		}\
	}\
}

enum {
	P_MODE,
	P_FPS,
	P_FSK,
	P_SIZE,
	P_OVERSCAN,
	P_FILTER,
	P_NTSCFORMAT,
	P_PALETTE,
	P_SAVEONEXIT,
	P_RENDER,
	P_VSYNC,
	P_FSCREEN,
	P_STRETCH,
	P_AUDIO,
	P_SAMPLERATE,
	P_CHANNELS,
	P_AUDIO_QUALITY,
	P_SWAP_DUTY,
	P_GAMEGENIE,
	P_STEREODELAY
};
enum {
	PGS_SLOT,
	PGS_OVERSCAN
 };

typedef struct {
	char *lname;
	char *sname;
	char *comment1;
	char *comment2;
	char *help;
} _param;

#ifdef __CMDLINE__
static const char *opt_short = "m:f:k:s:o:i:n:p:r:v:u:t:a:l:c:d:q:g:Vh?";
static const struct option opt_long[] = {
	{ "mode",               required_argument, NULL, 'm'},
	{ "fps",                required_argument, NULL, 'f'},
	{ "frameskip",          required_argument, NULL, 'k'},
	{ "size",               required_argument, NULL, 's'},
	{ "overscan",           required_argument, NULL, 'o'},
	{ "filter",             required_argument, NULL, 'i'},
	{ "ntsc-format",        required_argument, NULL, 'n'},
	{ "palette",            required_argument, NULL, 'p'},
	{ "rendering",          required_argument, NULL, 'r'},
	{ "vsync",              required_argument, NULL, 'v'},
	{ "fullscreen",         required_argument, NULL, 'u'},
	{ "stretch-fullscreen", required_argument, NULL, 't'},
	{ "audio",              required_argument, NULL, 'a'},
	{ "samplerate",         required_argument, NULL, 'l'},
	{ "channels",           required_argument, NULL, 'c'},
	{ "stereo-delay",       required_argument, NULL, 'd'},
	{ "audio-quality",      required_argument, NULL, 'q'},
	{ "swap-duty",          required_argument, NULL,  0 },
	{ "gamegenie",          required_argument, NULL, 'g'},
	{ "help",               no_argument,       NULL, 'h'},
	{ "version",            no_argument,       NULL, 'V'},
	{ "portable",           no_argument,       NULL,  0 },
	{ 0,                    0,                 0,     0 }
};
#endif

static const _param param[] = {
	{
		"preferred mode",
		NULL,
		"# possible values: pal, ntsc, dendy, auto",
		NULL,
		"-m, --mode                preferred mode        : pal, ntsc, dendy, auto"
	},
	{
		"frames per second",
		NULL,
		"# possible values: default, 58, 57. ..., 45, 44",
		NULL,
		"-f, --fps                 frames per second     : default, 60, ..., 44"
	},
	{
		"frame skip",
		NULL,
		"# possible values: default, 1, ... 9",
		NULL,
		"-k, --frameskip           frames to skip        : default, 1, ..., 9"
	},
	{
		"size window",
		NULL,
		"# possible values: 1x, 2x, 3x, 4x",
		"# Note : 1x works only with \'filter = none\'",
		"-s, --size                window size           : 1x, 2x, 3x, 4x"
	},
	{
		"overscan default",
		NULL,
		"# possible values: on, off",
		NULL,
		"-o, --overscan            default overscan      : on, off"
	},
	{
		"filter",
		NULL,
		"# possible values: none, bilinear, scale2x, scale3x, scale4x, hq2x, hq3x, hq4x,\n"
		"#                  ntsc, posphor, scanline, dbl, crtcurve, crtnocurve",
		NULL,
		"-i, --filter              filter to apply       : nofilter, bilinear, scale2x,\n"
		"                                                  scale3x, scale4x, hq2x, hq3x,\n"
		"                                                  hq4x, ntsc, posphor, scanline,\n"
		"                                                  dbl, crtcurve, crtnocurve"
	},
	{
		"ntsc filter format",
		NULL,
		"# possible values: composite, svideo, rgb",
		NULL,
		"-n, --ntsc-format         format of ntsc filter : composite, svideo, rgb"
	},
	{
		"palette",
		NULL,
		"# possible values: pal, ntsc, sony, mono, green",
		NULL,
		"-p, --palette             type of palette       : pal, ntsc, sony, mono, green"
	},
	{
		"save settings on exit",
		NULL,
		"# possible values: yes, no",
		NULL,
		NULL
	},
#if defined SDL
	{
		"rendering",
		NULL,
		"# possible values: software, opengl, glsl",
		NULL,
		"-r, --rendering           type of rendering     : software, opengl, glsl"
	},
#elif defined D3D9
	{
		"rendering",
		NULL,
		"# possible values: software, hlsl",
		NULL,
		"-r, --rendering           type of rendering     : software, hlsl"
	},
#endif
	{
		"vsync",
		NULL,
		"# possible values: on, off",
		NULL,
		"-v, --vsync               use of vsync          : on, off"
	},
	{
		"fullscreen",
		NULL,
		"# possible values: yes, no",
		NULL,
		"-u, --fullscreen          no comment            : yes, no"
	},
	{
		"stretch in fullscreen",
		NULL,
		"# possible values: yes, no",
		NULL,
		"-t, --stretch-fullscreen  adjust aspect ratio   : yes, no"
	},
	{
		"audio",
		NULL,
		"# possible values: on, off",
		NULL,
		"-a, --audio                                     : on, off"
	},
	{
		"sample rate",
		NULL,
		"# possible values: 44100, 22050, 11025",
		NULL,
		"-l, --samplerate          sample rate           : 44100, 22050, 11025"
	},
	{
		"channels",
		NULL,
		"# possible values: mono, stereo",
		NULL,
		"-c, --channels            audio channels        : mono, stereo"
	},
	{
		"audio quality",
		NULL,
		"# possible values: low, high",
		NULL,
		"-q, --audio-quality       audio quality         : low, high"
	},
	{
		"swap duty cycles (Famicom clone chip audio emulation)",
		NULL,
		"# possible values: yes, no",
		NULL,
		"    --swap-duty           swap duty cycles      : yes, no"
	},
	{
		"gamegenie",
		NULL,
		"# possible values: yes, no",
		NULL,
		"-g, --gamegenie           active game genie     : yes, no"
	},
	{
		"stereo delay",
		NULL,
		"# possible values: [5 - 100]",
		NULL,
		"-d, --stereo-delay        stereo effect delay   : [5 - 100]"
	},
};
static const _param param_pgs[] = {
	{
		"last save slot",
		NULL,
		"# possible values: 0, 1, 2, 3, 4, 5",
		NULL,
		NULL
	},
	{
		"overscan",
		NULL,
		"# possible values: on, off, default",
		NULL,
		NULL
	},
};
static const _param param_apu_channel[] = {
	{
		"square1",
		NULL,
		"# possible values: [on, off],[0 - 100]",
		NULL,
		NULL
	},
	{
		"square2",
		NULL,
		"# possible values: [on, off],[0 - 100]",
		NULL,
		NULL
	},
	{
		"triangle",
		NULL,
		"# possible values: [on, off],[0 - 100]",
		NULL,
		NULL
	},
	{
		"noise",
		NULL,
		"# possible values: [on, off],[0 - 100]",
		NULL,
		NULL
	},
	{
		"dmc",
		NULL,
		"# possible values: [on, off],[0 - 100]",
		NULL,
		NULL
	},
	{
		"extra",
		NULL,
		"# possible values: [on, off],[0 - 100]",
		NULL,
		NULL
	},
	{
		"master volume",
		NULL,
		"# possible values: [0 - 100]",
		NULL,
		NULL
	},
};
static const _param param_input_base[] = {
	{
		"controller 1",
		NULL,
		"# possible values: disable, standard, zapper",
		NULL,
		NULL
	},
	{
		"controller 2",
		NULL,
		"# possible values: disable, standard, zapper",
		NULL,
		NULL
	},
	{
		"permit up+down left+right",
		NULL,
		"# possible values: yes, no",
		NULL,
		NULL
	}
};
static const _param param_input_p1k[] = {
	{ "P1K A       " }, { "P1K B       " }, { "P1K Select  " }, { "P1K Start   " },
	{ "P1K Up      " }, { "P1K Down    " }, { "P1K Left    " }, { "P1K Right   " },
	{ "P1K TurboA  " }, { "P1K TurboB  " }
};
static const _param param_input_p1j[] = {
	{ "P1J A       " }, { "P1J B       " }, { "P1J Select  " }, { "P1J Start   " },
	{ "P1J Up      " }, { "P1J Down    " }, { "P1J Left    " }, { "P1J Right   " },
	{ "P1J TurboA  " }, { "P1J TurboB  " }, { "P1J Id      " }
};
static const _param param_turbo_delay_p1[] = {
	{ "P1 TA Delay " }, { "P1 TB Delay " }
};
static const _param param_input_p2k[] = {
	{ "P2K A       " }, { "P2K B       " }, { "P2K Select  " }, { "P2K Start   " },
	{ "P2K Up      " }, { "P2K Down    " }, { "P2K Left    " }, { "P2K Right   " },
	{ "P2K TurboA  " }, { "P2K TurboB  " }
};
static const _param param_input_p2j[] = {
	{ "P2J A       " }, { "P2J B       " }, { "P2J Select  " }, { "P2J Start   " },
	{ "P2J Up      " }, { "P2J Down    " }, { "P2J Left    " }, { "P2J Right   " },
	{ "P2J TurboA  " }, { "P2J TurboB  " }, { "P2J Id      " }
};
static const _param param_turbo_delay_p2[] = {
	{ "P2 TA Delay " }, { "P2 TB Delay " }
};
static const _param param_no_yes[] = {
	{"No",  "no" },
	{"Yes", "yes"}
};
static const _param param_off_on[] = {
	{"Off", "off"},
	{"On",  "on" }
};
static const _param param_mode[] = {
	{"Auto",  "auto" },
	{"NTSC",  "ntsc" },
	{"PAL",   "pal"  },
	{"Dendy", "dendy"}
};
static const _param param_fps[] = {
	{"Default", "default"},
	{"60",      "60"     },
	{"59",      "59"     },
	{"58",      "58"     },
	{"57",      "57"     },
	{"56",      "56"     },
	{"55",      "55"     },
	{"54",      "54"     },
	{"53",      "53"     },
	{"52",      "52"     },
	{"51",      "51"     },
	{"50",      "50"     },
	{"49",      "49"     },
	{"48",      "48"     },
	{"47",      "47"     },
	{"46",      "46"     },
	{"45",      "45"     },
	{"44",      "44"     }
};
static const _param param_fsk[] = {
	{"Default", "default"},
	{"1",       "1"      },
	{"2",       "2"      },
	{"3",       "3"      },
	{"4",       "4"      },
	{"5",       "5"      },
	{"6",       "6"      },
	{"7",       "7"      },
	{"8",       "8"      },
	{"9",       "9"      }
};
static const _param param_size[] = {
	{"NONE", "none"},
	{"1x",   "1x"  },
	{"2x",   "2x"  },
	{"3x",   "3x"  },
	{"4x",   "4x"  }
};
static const _param param_filter[] = {
	{"no filter",  "none"      },
	{"Scale2X",    "scale2x"   },
	{"Scale3X",    "scale3x"   },
	{"Scale4X",    "scale4x"   },
	{"Hq2X",       "hq2x"      },
	{"Hq3X",       "hq3x"      },
	{"Hq4X",       "hq4x"      },
	{"NTSC",       "ntsc"      },
	{"Bilinear",   "bilinear"  },
	{"Poshpor",    "posphor"   },
	{"Scanline",   "scanline"  },
	{"DBL",        "dbl"       },
	{"CRTCURVE",   "crtcurve"  },
	{"CRTNOCURVE", "crtnocurve"},
};
static const _param param_ntsc[] = {
	{"Composite", "composite"},
	{"S-Video",   "svideo"   },
	{"RGB",       "rgb"      }
};
static const _param param_palette[] = {
	{"PAL palette",       "pal"  },
	{"NTSC palette",      "ntsc" },
	{"Sony CXA2025AS US", "sony" },
	{"Monochrome",        "mono" },
	{"Green",             "green"}
};
#if defined SDL
static const _param param_render[] = {
	{"Software", "software"},
	{"OpenGL",   "opengl"  },
	{"GLSL",     "glsl"    }
};
#elif defined D3D9
static const _param param_render[] = {
	{"Software", "software"},
	{"HLSL",     "hlsl"    }
};
#endif
static const _param param_samplerate[] = {
	{"44100", "44100"},
	{"22050", "22050"},
	{"11025", "11025"}
};
static const _param param_channels[] = {
	{"NULL",   "NULL"  },
	{"Mono",   "mono"  },
	{"Stereo", "stereo"}
};
static const _param param_audio_quality[] = {
	{"Low",  "low" },
	{"High", "high"}
};
static const _param param_slot[] = {
	{"0", "0"},
	{"1", "1"},
	{"2", "2"},
	{"3", "3"},
	{"4", "4"},
	{"5", "5"}
};
static const _param param_oscan[] = {
	{"Off",     "off"     },
	{"On",      "on"      },
	{"Default", "default" }
};
static const _param param_controller[] = {
	{"Disable",  "disable" },
	{"Standard", "standard"},
	{"Zapper",   "zapper"  }
};

#endif /* PARAM_H_ */
