/*
 * param.h
 *
 *  Created on: 01/ago/2011
 *      Author: fhorse
 */

#ifndef PARAM_H_
#define PARAM_H_

#define paramSearch(start, buffer, param, cmd)\
{\
	BYTE index;\
	for(index = start; index < LENGTH(param); index++) {\
		if (strcmp(buffer, param[index].sname) == 0) {\
			cmd;\
			break;\
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
#ifdef OPENGL
	P_RENDER,
	P_VSYNC,
	P_FSCREEN,
	P_STRETCH,
#endif
	P_AUDIO,
	P_SAMPLERATE,
	P_CHANNELS,
	P_GAMEGENIE
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
static const char *optShort = "m:f:k:s:o:i:n:p:"
#ifdef OPENGL
		"r:v:u:t:"
#endif
		"a:l:c:g:Vh?";

static const struct option optLong[] = {
	{ "mode",        required_argument, NULL, 'm'},
	{ "fps",         required_argument, NULL, 'f'},
	{ "frameskip",   required_argument, NULL, 'k'},
	{ "size",        required_argument, NULL, 's'},
	{ "overscan",    required_argument, NULL, 'o'},
	{ "filter",      required_argument, NULL, 'i'},
	{ "ntsc-format", required_argument, NULL, 'n'},
	{ "palette",     required_argument, NULL, 'p'},
#ifdef OPENGL
    { "rendering",          required_argument, NULL, 'r'},
    { "vsync",              required_argument, NULL, 'v'},
    { "fullscreen",         required_argument, NULL, 'u'},
    { "stretch-fullscreen", required_argument, NULL, 't'},
#endif
    { "audio",       required_argument, NULL, 'a'},
    { "samplerate",  required_argument, NULL, 'l'},
    { "channels",    required_argument, NULL, 'c'},
    { "gamegenie",   required_argument, NULL, 'g'},
    { "help",        no_argument,       NULL, 'h'},
    { "version",     no_argument,       NULL, 'V'},
    { 0,             0,                 0,     0 }
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
		"-f, --fps                 frames per second     : default, 58, ..., 44"
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
		"# possible values: none, bilinear, scale2x, scale3x, scale4x, hq2x, hq3x, hq4x, ntsc",
		NULL,
		"-i, --filter              filter to apply       : nofilter, bilinear, scale2x,\n"
		"                                                  scale3x, scale4x, hq2x, hq3x,\n"
		"                                                  hq4x, ntsc"
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
#ifdef OPENGL
	{
		"rendering",
		NULL,
		"# possible values: software, opengl",
		NULL,
		"-r, --rendering           type of rendering     : software, opengl"
	},
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
#endif
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
		"-c, --channels            channels audio        : mono, stereo"
	},
	{
		"gamegenie",
		NULL,
		"# possible values: yes, no",
		NULL,
		"-g, --gamegenie           active game genie     : yes, no"
	},
};
static const _param paramPgs[] = {
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
static const _param paramInputCtrl[] = {
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
	}
};
static const _param paramInputP1K[] = {
	{ "P1K A     " }, { "P1K B     " },	{ "P1K Select" }, { "P1K Start " },
	{ "P1K Up    " }, { "P1K Down  " },	{ "P1K Left  " }, { "P1K Right " },
	{ "P1K TurboA" }, { "P1K TurboB" }
};
static const _param paramInputP1J[] = {
	{ "P1J A     " }, { "P1J B     " }, { "P1J Select" }, { "P1J Start " },
	{ "P1J Up    " }, { "P1J Down  " }, { "P1J Left  " }, { "P1J Right " },
	{ "P1J TurboA" }, { "P1J TurboB" }, { "P1J Id    " }
};
static const _param paramInputP2K[] = {
	{ "P2K A     " }, { "P2K B     " },	{ "P2K Select" }, { "P2K Start " },
	{ "P2K Up    " }, { "P2K Down  " },	{ "P2K Left  " }, { "P2K Right " },
	{ "P2K TurboA" }, { "P2K TurboB" }
};
static const _param paramInputP2J[] = {
	{ "P2J A     " }, { "P2J B     " }, { "P2J Select" }, { "P2J Start " },
	{ "P2J Up    " }, { "P2J Down  " }, { "P2J Left  " }, { "P2J Right " },
	{ "P2J TurboA" }, { "P2J TurboB" }, { "P2J Id    " }
};
static const _param pNoYes[] = {
	{"No",  "no" },
	{"Yes", "yes"}
};
static const _param pOffOn[] = {
	{"Off",     "off"},
	{"On",      "on" }
};
static const _param pMode[] = {
	{"Auto",  "auto" },
	{"NTSC",  "ntsc" },
	{"PAL",   "pal"  },
	{"Dendy", "dendy"}
};
static const _param pFps[] = {
	{"Default", "default"},
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
static const _param pFsk[] = {
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
static const _param pSize[] = {
	{"NONE", "none"},
	{"1X",    "1x" },
	{"2X",    "2x" },
	{"3X",    "3x" },
	{"4X",    "4x" }
};
static const _param pFilter[] = {
	{"no filter", "none"    },
	{"Scale2X",   "scale2x" },
	{"Scale3X",   "scale3x" },
	{"Scale4X",   "scale4x" },
	{"Hq2X",      "hq2x"    },
	{"Hq3X",      "hq3x"    },
	{"Hq4X",      "hq4x"    },
	{"NTSC",      "ntsc"    },
	{"Bilinear",  "bilinear"},
};
static const _param pNtsc[] = {
	{"Composite", "composite"},
	{"S-Video",   "svideo"   },
	{"RGB",       "rgb"      }
};
static const _param pPalette[] = {
	{"PAL palette",       "pal"  },
	{"NTSC palette",      "ntsc" },
	{"Sony CXA2025AS US", "sony" },
	{"Monochrome",        "mono" },
	{"Green",             "green"}
};
#ifdef OPENGL
static const _param pRendering[] = {
	{"Software", "sotfware"},
	{"OpenGL",   "opengl"  }
};
#endif
static const _param pSamplerate[] = {
	{"44100", "44100"},
	{"22050", "22050"},
	{"11025", "11025"}
};
static const _param pChannels[] = {
	{"NULL",   "NULL"  },
	{"Mono",   "mono"  },
	{"Stereo", "stereo"}
};
static const _param pSlot[] = {
	{"0", "0"},
	{"1", "1"},
	{"2", "2"},
	{"3", "3"},
	{"4", "4"},
	{"5", "5"}
};
static const _param pOverscan[] = {
	{"Off",     "off"     },
	{"On",      "on"      },
	{"Default", "default" }
};
static const _param pController[] = {
	{"Disable",  "disable" },
	{"Standard", "standard"},
	{"Zapper",   "zapper"  }
};

#endif /* PARAM_H_ */
