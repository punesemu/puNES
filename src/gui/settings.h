/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "common.h"
#include "info.h"
#include "gfx.h"
#include "fps.h"
#include "snd.h"
#include "cheat.h"
#include "audio/quality.h"
#include "audio/channels.h"
#include "overscan.h"
#include "input.h"

#if defined (__WIN32__)
#define NEWLINE "\r\n"
#else
#define NEWLINE "\n"
#endif

enum set_element {
	SET_MODE,
	SET_FF_VELOCITY,
	SET_BCK_PAUSE,
	SET_CHEAT_MODE,
	SET_SAVE_SETTINGS_ON_EXIT,
	SET_RENDERING,
	SET_FPS,
	SET_FRAMESKIP,
	SET_SCALE,
	SET_PAR,
	SET_PAR_SOFT_STRETCH,
	SET_OVERSCAN_DEFAULT,
	SET_OVERSCAN_BRD_NTSC,
	SET_OVERSCAN_BRD_PAL,
	SET_FILTER,
	SET_NTSC_FORMAT,
	SET_PALETTE,
	SET_FILE_PALETTE,
	SET_SWAP_EMPHASIS_PAL,
	SET_VSYNC,
	SET_INTERPOLATION,
	SET_TEXT_ON_SCREEN,
	SET_FULLSCREEN,
	SET_STRETCH_FULLSCREEN,
	SET_AUDIO_BUFFER_FACTOR,
	SET_SAMPLERATE,
	SET_CHANNELS,
	SET_STEREO_DELAY,
	SET_AUDIO_QUALITY,
	SET_SWAP_DUTY,
	SET_AUDIO,
	SET_GUI_OPEN_PATH,
	SET_GUI_LAST_POSITION,
	SET_GUI_LANGUAGE,
	SET_APU_MASTER,
	SET_APU_SQUARE1,
	SET_APU_SQUARE2,
	SET_APU_TRIANGLE,
	SET_APU_NOISE,
	SET_APU_DMC,
	SET_APU_EXTRA
};
enum pgs_element {
	SET_PGS_SLOT,
	SET_PGS_FILE_SAVE,
	SET_PGS_OVERSCAN
};
enum inp_element {
	SET_INP_SC_OPEN,
	SET_INP_SC_QUIT,
	SET_INP_SC_HARD_RESET,
	SET_INP_SC_SOFT_RESET,
	SET_INP_SC_SWITCH_SIDES,
	SET_INP_SC_EJECT_DISK,
	SET_INP_SC_PAUSE,
	SET_INP_SC_FAST_FORWARD,
	SET_INP_SC_MODE_PAL,
	SET_INP_SC_MODE_NTSC,
	SET_INP_SC_MODE_DENDY,
	SET_INP_SC_MODE_AUTO,
	SET_INP_SC_SCALE_1X,
	SET_INP_SC_SCALE_2X,
	SET_INP_SC_SCALE_3X,
	SET_INP_SC_SCALE_4X,
#if defined (SDL)
	SET_INP_SC_EFFECT_CUBE,
#endif
	SET_INP_SC_INTERPOLATION,
	SET_INP_SC_FULLSCREEN,
	SET_INP_SC_STRETCH_FULLSCREEN,
	SET_INP_SC_AUDIO_ENABLE,
	SET_INP_SC_SAVE_SETTINGS,
	SET_INP_SC_SAVE_STATE,
	SET_INP_SC_LOAD_STATE,
	SET_INP_SC_INC_SLOT,
	SET_INP_SC_DEC_SLOT,

	SET_INP_SC_JOYSTICK_ID,

	SET_INP_SK_TIMELINE_KEY,

	SET_INP_P1_CONTROLLER,
	SET_INP_P1_PAD_TYPE,
	SET_INP_P1K_A,
	SET_INP_P1K_B,
	SET_INP_P1K_SELECT,
	SET_INP_P1K_START,
	SET_INP_P1K_UP,
	SET_INP_P1K_DOWN,
	SET_INP_P1K_LEFT,
	SET_INP_P1K_RIGHT,
	SET_INP_P1K_TURBOA,
	SET_INP_P1K_TURBOB,
	SET_INP_P1J_A,
	SET_INP_P1J_B,
	SET_INP_P1J_SELECT,
	SET_INP_P1J_START,
	SET_INP_P1J_UP,
	SET_INP_P1J_DOWN,
	SET_INP_P1J_LEFT,
	SET_INP_P1J_RIGHT,
	SET_INP_P1J_TURBOA,
	SET_INP_P1J_TURBOB,
	SET_INP_P1J_ID,
	SET_INP_P1_TURBOA_DELAY,
	SET_INP_P1_TURBOB_DELAY,

	SET_INP_P2_CONTROLLER,
	SET_INP_P2_PAD_TYPE,
	SET_INP_P2K_A,
	SET_INP_P2K_B,
	SET_INP_P2K_SELECT,
	SET_INP_P2K_START,
	SET_INP_P2K_UP,
	SET_INP_P2K_DOWN,
	SET_INP_P2K_LEFT,
	SET_INP_P2K_RIGHT,
	SET_INP_P2K_TURBOA,
	SET_INP_P2K_TURBOB,
	SET_INP_P2J_A,
	SET_INP_P2J_B,
	SET_INP_P2J_SELECT,
	SET_INP_P2J_START,
	SET_INP_P2J_UP,
	SET_INP_P2J_DOWN,
	SET_INP_P2J_LEFT,
	SET_INP_P2J_RIGHT,
	SET_INP_P2J_TURBOA,
	SET_INP_P2J_TURBOB,
	SET_INP_P2J_ID,
	SET_INP_P2_TURBOA_DELAY,
	SET_INP_P2_TURBOB_DELAY,

	SET_INP_P3_CONTROLLER,
	SET_INP_P3_PAD_TYPE,
	SET_INP_P3K_A,
	SET_INP_P3K_B,
	SET_INP_P3K_SELECT,
	SET_INP_P3K_START,
	SET_INP_P3K_UP,
	SET_INP_P3K_DOWN,
	SET_INP_P3K_LEFT,
	SET_INP_P3K_RIGHT,
	SET_INP_P3K_TURBOA,
	SET_INP_P3K_TURBOB,
	SET_INP_P3J_A,
	SET_INP_P3J_B,
	SET_INP_P3J_SELECT,
	SET_INP_P3J_START,
	SET_INP_P3J_UP,
	SET_INP_P3J_DOWN,
	SET_INP_P3J_LEFT,
	SET_INP_P3J_RIGHT,
	SET_INP_P3J_TURBOA,
	SET_INP_P3J_TURBOB,
	SET_INP_P3J_ID,
	SET_INP_P3_TURBOA_DELAY,
	SET_INP_P3_TURBOB_DELAY,

	SET_INP_P4_CONTROLLER,
	SET_INP_P4_PAD_TYPE,
	SET_INP_P4K_A,
	SET_INP_P4K_B,
	SET_INP_P4K_SELECT,
	SET_INP_P4K_START,
	SET_INP_P4K_UP,
	SET_INP_P4K_DOWN,
	SET_INP_P4K_LEFT,
	SET_INP_P4K_RIGHT,
	SET_INP_P4K_TURBOA,
	SET_INP_P4K_TURBOB,
	SET_INP_P4J_A,
	SET_INP_P4J_B,
	SET_INP_P4J_SELECT,
	SET_INP_P4J_START,
	SET_INP_P4J_UP,
	SET_INP_P4J_DOWN,
	SET_INP_P4J_LEFT,
	SET_INP_P4J_RIGHT,
	SET_INP_P4J_TURBOA,
	SET_INP_P4J_TURBOB,
	SET_INP_P4J_ID,
	SET_INP_P4_TURBOA_DELAY,
	SET_INP_P4_TURBOB_DELAY,

	SET_INP_CONTROLLER_MODE,
	SET_INP_LEFTRIGHT
};

enum set_num_shortcut { SET_MAX_NUM_SC = SET_INP_SC_JOYSTICK_ID - SET_INP_SC_OPEN};

enum list_settings_element {
	LSET_SET,
	LSET_PGS,
	LSET_INP
};

typedef struct {
	const char *lname;
	const char *sname;
	int value;
} _opt;
typedef struct {
	const char *grp;
	const char *key;
	const char *def;

	const char *cmt;
	const char *hlp;

	struct {
		const int count;
		const _opt *opt;
	} opts;
} _settings;
typedef struct {
	const _settings *cfg;
	const int count;
} _list_settings;

static const _opt opt_no_yes[] = {
	{NULL, "no" , FALSE},
	{NULL, "yes", TRUE}
};
static const _opt opt_off_on[] = {
	{NULL, "off", FALSE},
	{NULL, "on" , TRUE},
};
static const _opt opt_mode[] = {
	{"Auto",  "auto" , AUTO},
	{"NTSC",  "ntsc" , NTSC},
	{"PAL" ,  "pal"  , PAL},
	{"Dendy", "dendy", DENDY}
};
static const _opt opt_ff_velocity[] = {
	{NULL, "2x", FF_2X},
	{NULL, "3x", FF_3X},
	{NULL, "4x", FF_4X},
	{NULL, "5x", FF_5X}
};
static const _opt opt_rend[] = {
	{"Software", "software", RENDER_SOFTWARE},
#if defined (SDL)
	{"OpenGL"  , "opengl"  , RENDER_OPENGL},
	{"GLSL"    , "glsl"    , RENDER_GLSL}
#elif defined (D3D9)
	{"HLSL"    , "hlsl"    , RENDER_HLSL}
#endif
};
static const _opt opt_fps[] = {
	{NULL, "default", FPS_DEFAULT},
	{NULL, "60"     , FPS_60},
	{NULL, "59"     , FPS_59},
	{NULL, "58"     , FPS_58},
	{NULL, "57"     , FPS_57},
	{NULL, "56"     , FPS_56},
	{NULL, "55"     , FPS_55},
	{NULL, "54"     , FPS_54},
	{NULL, "53"     , FPS_53},
	{NULL, "52"     , FPS_52},
	{NULL, "51"     , FPS_51},
	{NULL, "50"     , FPS_50},
	{NULL, "49"     , FPS_49},
	{NULL, "48"     , FPS_48},
	{NULL, "47"     , FPS_47},
	{NULL, "46"     , FPS_46},
	{NULL, "45"     , FPS_45},
	{NULL, "44"     , FPS_44}
};
static const _opt opt_fsk[] = {
	{NULL, "default", 0},
	{NULL, "1"      , 1},
	{NULL, "2"      , 2},
	{NULL, "3"      , 3},
	{NULL, "4"      , 4},
	{NULL, "5"      , 5},
	{NULL, "6"      , 6},
	{NULL, "7"      , 7},
	{NULL, "8"      , 8},
	{NULL, "9"      , 9}
};
static const _opt opt_scale[] = {
	{NULL, "1x", X1},
	{NULL, "2x", X2},
	{NULL, "3x", X3},
	{NULL, "4x", X4}
};
static const _opt opt_par[] = {
	{NULL, "1:1", PAR11},
	{NULL, "5:4", PAR54},
	{NULL, "8:7", PAR87}
};
static const _opt opt_oscan[] = {
	{NULL, "off"    , OSCAN_OFF},
	{NULL, "on"     , OSCAN_ON},
	{NULL, "default", OSCAN_DEFAULT}
};
static const _opt opt_filter[] = {
	{"no filter" , "none"      , NO_FILTER},
	{"Scale2X"   , "scale2x"   , SCALE2X},
	{"Scale3X"   , "scale3x"   , SCALE3X},
	{"Scale4X"   , "scale4x"   , SCALE4X},
	{"Hq2X"      , "hq2x"      , HQ2X},
	{"Hq3X"      , "hq3x"      , HQ3X},
	{"Hq4X"      , "hq4x"      , HQ4X},
	{"NTSC"      , "ntsc"      , NTSC_FILTER},
	{"Phoshpor"  , "phosphor"  , PHOSPHOR},
	{"Scanline"  , "scanline"  , SCANLINE},
	{"DBL"       , "dbl"       , DBL},
	{"CRTCURVE"  , "crtcurve"  , CRT_CURVE},
	{"CRTNOCURVE", "crtnocurve", CRT_NO_CURVE},
	{"Phosphor2" , "phosphor2" , PHOSPHOR2},
	{"DarkRoom"  , "darkroom"  , DARK_ROOM},
	{"xBRZ 2x"   , "xbrz2x"    , XBRZ2X},
	{"xBRZ 3x"   , "xbrz3x"    , XBRZ3X},
	{"xBRZ 4x"   , "xbrz4x"    , XBRZ4X},
};
static const _opt opt_ntsc[] = {
	{"Composite", "composite", COMPOSITE},
	{"S-Video"  , "svideo"   , SVIDEO},
	{"RGB"      , "rgb"      , RGBMODE}
};
static const _opt opt_palette[] = {
	{"PAL palette"      , "pal"  , PALETTE_PAL},
	{"NTSC palette"     , "ntsc" , PALETTE_NTSC},
	{"Sony CXA2025AS US", "sony" , PALETTE_SONY},
	{"Monochrome"       , "mono" , PALETTE_MONO},
	{"Green"            , "green", PALETTE_GREEN},
	{"Extern"           , "file" , PALETTE_FILE}
};
static const _opt opt_audio_buffer_factor[] = {
	{NULL, "0"      , 0},
	{NULL, "1"      , 1},
	{NULL, "2"      , 2},
	{NULL, "3"      , 3},
	{NULL, "4"      , 4},
	{NULL, "5"      , 5},
	{NULL, "6"      , 6},
	{NULL, "7"      , 7},
	{NULL, "8"      , 8},
	{NULL, "9"      , 9},
	{NULL, "10"     , 10},
	{NULL, "11"     , 11},
	{NULL, "12"     , 12},
	{NULL, "13"     , 13},
	{NULL, "14"     , 14},
	{NULL, "15"     , 15}
};
static const _opt opt_samplerate[] = {
	{NULL, "48000", S48000},
	{NULL, "44100", S44100},
	{NULL, "22050", S22050},
	{NULL, "11025", S11025}
};
static const _opt opt_channels[] = {
	{NULL,   "mono",    CH_MONO},
	{NULL,   "delay",   CH_STEREO_DELAY},
	{NULL,   "panning", CH_STEREO_PANNING},
};
static const _opt opt_audio_quality[] = {
	{NULL, "low",  AQ_LOW},
	{NULL, "high", AQ_HIGH}
};
static const _opt opt_cheat_mode[] = {
	{NULL, "disabled",   NOCHEAT_MODE},
	{NULL, "gamegenie",  GAMEGENIE_MODE},
	{NULL, "cheatslist", CHEATSLIST_MODE}
};
static const _opt opt_languages[] = {
	{NULL, "english", LNG_ENGLISH},
	{NULL, "italian", LNG_ITALIAN},
	{NULL, "russian", LNG_RUSSIAN}
};

static const _opt opt_slot_pgs[] = {
	{NULL, "0", 0},
	{NULL, "1", 1},
	{NULL, "2", 2},
	{NULL, "3", 3},
	{NULL, "4", 4},
	{NULL, "5", 5}
};

static const _opt opt_controller[] = {
	{NULL, "disable",  CTRL_DISABLED},
	{NULL, "standard", CTRL_STANDARD},
	{NULL, "zapper",   CTRL_ZAPPER}
};
static const _opt opt_controller_mode[] = {
	{NULL, "nes",        CTRL_MODE_NES},
	{NULL, "famicom",    CTRL_MODE_FAMICOM},
	{NULL, "four score", CTRL_MODE_FOUR_SCORE}
};
static const _opt opt_pad_type[] = {
	{NULL, "auto",     CTRL_PAD_AUTO},
	{NULL, "original", CTRL_PAD_ORIGINAL},
	{NULL, "3rdparty", CTRL_PAD_3RD_PARTY},
};

static const _settings main_cfg[] = {
	{
		"system", "preferred mode", "auto",
		"# possible values: pal, ntsc, dendy, auto",
		"-m, --mode                preferred mode        : pal, ntsc, dendy, auto",
		{LENGTH(opt_mode), opt_mode}
	},
	{
		"system", "fast forward velocity", "2x",
		"# possible values: 2x, 3x, 4x, 5x",
		NULL,
		{LENGTH(opt_ff_velocity), opt_ff_velocity}
	},
	{
		"system", "pause when in background", "yes",
		"# possible values: yes, no",
		"    --background-pause                          : yes, no",
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		"system", "cheat mode", "disabled",
		"# possible values: disabled, gamegenie, cheatslist",
		"-g, --cheat-mode          cheat mode            : disabled, gamegenie, cheatslist",
		{LENGTH(opt_cheat_mode), opt_cheat_mode}
	},
	{
		"system", "save settings on exit", "no",
		"# possible values: yes, no",
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		"video", "rendering",
#if defined (SDL)
		 "glsl",
		"# possible values: software, opengl, glsl",
		"-r, --rendering           type of rendering     : software, opengl, glsl",
#elif defined (D3D9)
		 "hlsl",
		"# possible values: software, hlsl",
		"-r, --rendering           type of rendering     : software, hlsl",
#endif
		{LENGTH(opt_rend), opt_rend}
	},
	{
		"video", "frames per second", "default",
		"# possible values: default, 58, 57. ..., 45, 44",
		"-f, --fps                 frames per second     : default, 60, ..., 44",
		{LENGTH(opt_fps), opt_fps}
	},
	{
		"video", "frame skip", "default",
		"# possible values: default, 1, ..., 9",
		"-k, --frameskip           frames to skip        : default, 1, ..., 9",
		{LENGTH(opt_fsk), opt_fsk}
	},
	{
		"video", "size window", "2x",
		"# possible values: 1x, 2x, 3x, 4x" NEWLINE
		"# Note : 1x works only with \'filter=none\'",
		"-s, --size                window size           : 1x, 2x, 3x, 4x",
		{LENGTH(opt_scale), opt_scale}
	},
	{
		"video", "pixel aspect ratio", "8:7",
		"# possible values: 1:1, 5:4, 8:7",
		"-e, --pixel-aspect-ratio  enable aspect ratio   : 1:1, 5:4, 8:7",
		{LENGTH(opt_par), opt_par}
	},
	{
		"video", "pixel aspect ratio soft stretch", "yes",
		"# possible values: yes, no",
		"    --par-soft-stretch    improves the          : yes, no\n"
		"                          stretched image",
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		"video", "overscan default", "off",
		"# possible values: on, off",
		"-o, --overscan            default overscan      : on, off",
		{LENGTH(opt_oscan), opt_oscan}
	},
	{
		"video", "overscan bordes NTSC", "8,8,8,9",
		"# possible values: 0-17" NEWLINE
		"# format string  : [UP],[DOWN],[LEFT],[RIGHT]",
		"    --overscan-brd-ntsc   borders in pixels     : [UP],[DOWN],[LEFT],[RIGHT]",
		{0, NULL}
	},
	{
		"video", "overscan bordes PAL", "8,8,8,9",
		"# possible values: 0-17" NEWLINE
		"# format string  : [UP],[DOWN],[LEFT],[RIGHT]",
		"    --overscan-brd-pal    borders in pixels     : [UP],[DOWN],[LEFT],[RIGHT]",
		{0, NULL}
	},
	{
		"video", "filter", "none",
		"# possible values: none, scale2x, scale3x, scale4x, hq2x, hq3x, hq4x," NEWLINE
		"#                  xbrz2x, xbrz3x, xbrz4x, ntsc, phosphor, scanline," NEWLINE
		"#                  dbl, crtcurve, crtnocurve, phosphor2, dark_room",
		"-i, --filter              filter to apply       : nofilter, scale2x," NEWLINE
		"                                                  scale3x, scale4x, hq2x," NEWLINE
		"                                                  hq3x, hq4x, xbrz2x, xbrz3x," NEWLINE
		"                                                  xbrz4x,ntsc, phosphor," NEWLINE
		"                                                  scanline, dbl, crtcurve," NEWLINE
		"                                                  crtnocurve, phosphor2," NEWLINE
		"                                                  dark_room",
		{LENGTH(opt_filter), opt_filter}
	},
	{
		"video", "ntsc filter format", "composite",
		"# possible values: composite, svideo, rgb",
		"-n, --ntsc-format         format of ntsc filter : composite, svideo, rgb",
		{LENGTH(opt_ntsc), opt_ntsc}
	},
	{
		"video", "palette", "ntsc",
		"# possible values: pal, ntsc, sony, mono, green, file",
		"-p, --palette             type of palette       : pal, ntsc, sony, mono, green, file",
		{LENGTH(opt_palette), opt_palette}
	},
	{
		"video", "palette file", NULL,
		"# possible values: [PATH/NAME]",
		NULL,
		{0, NULL}
	},
	{
		"video", "disable swap emphasis PAL-Dendy", "no",
		"# possible values: yes, no",
		"    --swap-emphasis       disable swap emphasis : yes, no" NEWLINE
		"                          in PAL/Dendy mode",
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		"video", "vsync", "on",
		"# possible values: on, off",
		"-v, --vsync               use of vsync          : on, off",
		{LENGTH(opt_off_on), opt_off_on}
	},
	{
		"video", "interpolation", "no",
		"# possible values: yes, no",
		"-j, --interpolation       enable interpolation  : yes, no",
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		"video", "text on screen", "yes",
		"# possible values: yes, no",
		"    --txt-on-screen       enable messages       : yes, no",
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		"video", "fullscreen", "no",
		"# possible values: yes, no",
		"-u, --fullscreen          no comment            : yes, no",
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		"video", "stretch in fullscreen", "no",
		"# possible values: yes, no",
		"-t, --stretch-fullscreen  stretch image         : yes, no",
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		"audio", "buffer factor", "1",
		"# possible values: [0-15]",
		"-b, --audio-buffer-factor buffer size factor    : [0-15]",
		{LENGTH(opt_audio_buffer_factor), opt_audio_buffer_factor}
	},
	{
		"audio", "sample rate", "44100",
		"# possible values: 48000, 44100, 22050, 11025",
		"-l, --samplerate          sample rate           : 48000, 44100, 22050, 11025",
		{LENGTH(opt_samplerate), opt_samplerate}
	},
	{
		"audio", "channels", "delay",
		"# possible values: mono, delay, panning",
		"-c, --channels            audio channels        : mono, delay, panning",
		{LENGTH(opt_channels), opt_channels}
	},
	{
		"audio", "stereo delay", "30",
		"# possible values: [5 - 100]",
		"-d, --stereo-delay        stereo effect delay   : [5 - 100]",
		{0, NULL}
	},
	{
		"audio", "audio quality", "high",
		"# possible values: low, high",
		"-q, --audio-quality       audio quality         : low, high",
		{LENGTH(opt_audio_quality), opt_audio_quality}
	},
	{
		"audio", "swap duty cycles (Famicom clone chip audio emulation)", "no",
		"# possible values: yes, no",
		"    --swap-duty           swap duty cycles      : yes, no",
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		"audio", "audio", "on",
		"# possible values: on, off",
		"-a, --audio                                     : on, off",
		{LENGTH(opt_off_on), opt_off_on}
	},
	{
		"GUI", "last open path", NULL,
		"# possible values: [PATH]",
		NULL,
		{0, NULL}
	},
	{
		"GUI", "last position of window", "0,0",
		"# possible values: [X],[Y]",
		NULL,
		{0, NULL}
	},
	{
		"GUI", "language", "english",
		"# possible values: english,italian,russian",
		"    --language            GUI language          : english,italian,russian",
		{LENGTH(opt_languages), opt_languages}
	},
	{
		"apu channels", "master", "on,100",
		"# possible values: [on, off],[0 - 100]",
		NULL,
		{0, NULL}
	},
	{
		"apu channels", "square1", "on,100",
		"# possible values: [on, off],[0 - 100]",
		NULL,
		{0, NULL}
	},
	{
		"apu channels", "square2", "on,100",
		"# possible values: [on, off],[0 - 100]",
		NULL,
		{0, NULL}
	},
	{
		"apu channels", "triangle", "on,100",
		"# possible values: [on, off],[0 - 100]",
		NULL,
		{0, NULL}
	},
	{
		"apu channels", "noise", "on,100",
		"# possible values: [on, off],[0 - 100]",
		NULL,
		{0, NULL}
	},
	{
		"apu channels", "dmc", "on,100",
		"# possible values: [on, off],[0 - 100]",
		NULL,
		{0, NULL}
	},
	{
		"apu channels", "extra", "on,100",
		"# possible values: [on, off],[0 - 100]",
		NULL,
		{0, NULL}
	}
};

static const _settings pgs_cfg[] = {
	{
		"state", "last save slot", "0",
		"# possible values: 0, 1, 2, 3, 4, 5",
		NULL,
		{LENGTH(opt_slot_pgs), opt_slot_pgs}
	},
	{
		"state", "last save file used", NULL,
		"# possible values: [PATH/NAME]",
		NULL,
		{0, NULL}
	},
	{
		"video", "overscan", "default",
		"# possible values: on, off, default",
		NULL,
		{LENGTH(opt_oscan), opt_oscan}
	}
};

static const _settings inp_cfg[] = {
	{"shortcuts", "open",                     "Alt+O,NULL",      NULL, NULL, {0, NULL}},
	{"shortcuts", "quit",                     "Alt+Q,NULL",      NULL, NULL, {0, NULL}},
	{"shortcuts", "hard reset",               "F11,NULL",        NULL, NULL, {0, NULL}},
	{"shortcuts", "soft reset",               "F12,NULL",        NULL, NULL, {0, NULL}},
	{"shortcuts", "switch sides",             "Alt+S,NULL",      NULL, NULL, {0, NULL}},
	{"shortcuts", "eject disk",               "Alt+E,NULL",      NULL, NULL, {0, NULL}},
	{"shortcuts", "pause",                    "Pause,NULL",      NULL, NULL, {0, NULL}},
	{"shortcuts", "fast forward",             "Tab,NULL",        NULL, NULL, {0, NULL}},
	{"shortcuts", "mode pal",                 "F6,NULL",         NULL, NULL, {0, NULL}},
	{"shortcuts", "mode ntsc",                "F7,NULL",         NULL, NULL, {0, NULL}},
	{"shortcuts", "mode dendy",               "F8,NULL",         NULL, NULL, {0, NULL}},
	{"shortcuts", "mode auto",                "F9,NULL",         NULL, NULL, {0, NULL}},
	{"shortcuts", "scale 1x",                 "Alt+1,NULL",      NULL, NULL, {0, NULL}},
	{"shortcuts", "scale 2x",                 "Alt+2,NULL",      NULL, NULL, {0, NULL}},
	{"shortcuts", "scale 3x",                 "Alt+3,NULL",      NULL, NULL, {0, NULL}},
	{"shortcuts", "scale 4x",                 "Alt+4,NULL",      NULL, NULL, {0, NULL}},
#if defined (SDL)
	{"shortcuts", "cube effect",              "ALt+R,NULL",      NULL, NULL, {0, NULL}},
#endif
	{"shortcuts", "video interpolation",      "0,NULL",          NULL, NULL, {0, NULL}},
	{"shortcuts", "video fullscreen",         "Alt+Return,NULL", NULL, NULL, {0, NULL}},
	{"shortcuts", "video stretch fullscreen", "Alt+P,NULL",      NULL, NULL, {0, NULL}},
	{"shortcuts", "audio enable",             "Alt+A,NULL",      NULL, NULL, {0, NULL}},
	{"shortcuts", "save settings",            "Alt+W,NULL",      NULL, NULL, {0, NULL}},
	{"shortcuts", "save state",               "F1,NULL",         NULL, NULL, {0, NULL}},
	{"shortcuts", "load state",               "F4,NULL",         NULL, NULL, {0, NULL}},
	{"shortcuts", "increment state slot",     "F3,NULL",         NULL, NULL, {0, NULL}},
	{"shortcuts", "decrement state slot",     "F2,NULL",         NULL, NULL, {0, NULL}},

	{"shortcuts", "joystick Id",              "NULL",            NULL, NULL, {0, NULL}},

	{"special keys", "timeline key",          "LCtrl",           NULL, NULL, {0, NULL}},

	{
		"port 1", "controller 1", "standard",
		"# possible values: disable, standard, zapper",
		NULL,
		{LENGTH(opt_controller), opt_controller}
	},
	{
		"port 1", "pad 1 type", "auto",
		"# possible values: auto, original, 3rdparty",
		NULL,
		{LENGTH(opt_pad_type), opt_pad_type}
	},
	{"port 1", "P1K A",       "S",           "# player 1 keyboard", NULL, {0, NULL}},
	{"port 1", "P1K B",       "A",           NULL, NULL, {0, NULL}},
	{"port 1", "P1K Select",  "Z",           NULL, NULL, {0, NULL}},
	{"port 1", "P1K Start",   "X",           NULL, NULL, {0, NULL}},
	{"port 1", "P1K Up",      "Up",          NULL, NULL, {0, NULL}},
	{"port 1", "P1K Down",    "Down",        NULL, NULL, {0, NULL}},
	{"port 1", "P1K Left",    "Left",        NULL, NULL, {0, NULL}},
	{"port 1", "P1K Right",   "Right",       NULL, NULL, {0, NULL}},
	{"port 1", "P1K TurboA",  "W",           NULL, NULL, {0, NULL}},
	{"port 1", "P1K TurboB",  "Q",           NULL, NULL, {0, NULL}},
	{"port 1", "P1J A",       "JB1",         "# player 1 joystick", NULL, {0, NULL}},
	{"port 1", "P1J B",       "JB0",         NULL, NULL, {0, NULL}},
	{"port 1", "P1J Select",  "JB8",         NULL, NULL, {0, NULL}},
	{"port 1", "P1J Start",   "JB9",         NULL, NULL, {0, NULL}},
	{"port 1", "P1J Up",      "JA1MIN",      NULL, NULL, {0, NULL}},
	{"port 1", "P1J Down",    "JA1PLS",      NULL, NULL, {0, NULL}},
	{"port 1", "P1J Left",    "JA0MIN",      NULL, NULL, {0, NULL}},
	{"port 1", "P1J Right",   "JA0PLS",      NULL, NULL, {0, NULL}},
	{"port 1", "P1J TurboA",  "JB2",         NULL, NULL, {0, NULL}},
	{"port 1", "P1J TurboB",  "JB3",         NULL, NULL, {0, NULL}},
	{"port 1", "P1J Id",      "JOYSTICKID1", NULL, NULL, {0, NULL}},
	{"port 1", "P1 TA Delay", NULL,          NULL, NULL, {0, NULL}},
	{"port 1", "P1 TB Delay", NULL,          NULL, NULL, {0, NULL}},
	{
		"port 2", "controller 2", "disable",
		"# possible values: disable, standard, zapper",
		NULL,
		{LENGTH(opt_controller), opt_controller}
	},
	{
		"port 2", "pad 2 type", "auto",
		"# possible values: auto, original, 3rdparty",
		NULL,
		{LENGTH(opt_pad_type), opt_pad_type}
	},
	{"port 2", "P2K A",       "PgDown",      "# player 2 keyboard", NULL, {0, NULL}},
	{"port 2", "P2K B",       "End",         NULL, NULL, {0, NULL}},
	{"port 2", "P2K Select",  "Ins",         NULL, NULL, {0, NULL}},
	{"port 2", "P2K Start",   "Del",         NULL, NULL, {0, NULL}},
	{"port 2", "P2K Up",      "NumPad8",     NULL, NULL, {0, NULL}},
	{"port 2", "P2K Down",    "NumPad2",     NULL, NULL, {0, NULL}},
	{"port 2", "P2K Left",    "NumPad4",     NULL, NULL, {0, NULL}},
	{"port 2", "P2K Right",   "NumPad6",     NULL, NULL, {0, NULL}},
	{"port 2", "P2K TurboA",  "Home",        NULL, NULL, {0, NULL}},
	{"port 2", "P2K TurboB",  "PgUp",        NULL, NULL, {0, NULL}},
	{"port 2", "P2J A",       "JB1",         "# player 2 joystick", NULL, {0, NULL}},
	{"port 2", "P2J B",       "JB0",         NULL, NULL, {0, NULL}},
	{"port 2", "P2J Select",  "JB8",         NULL, NULL, {0, NULL}},
	{"port 2", "P2J Start",   "JB9",         NULL, NULL, {0, NULL}},
	{"port 2", "P2J Up",      "JA1MIN",      NULL, NULL, {0, NULL}},
	{"port 2", "P2J Down",    "JA1PLS",      NULL, NULL, {0, NULL}},
	{"port 2", "P2J Left",    "JA0MIN",      NULL, NULL, {0, NULL}},
	{"port 2", "P2J Right",   "JA0PLS",      NULL, NULL, {0, NULL}},
	{"port 2", "P2J TurboA",  "JB2",         NULL, NULL, {0, NULL}},
	{"port 2", "P2J TurboB",  "JB3",         NULL, NULL, {0, NULL}},
	{"port 2", "P2J Id",      "JOYSTICKID2", NULL, NULL, {0, NULL}},
	{"port 2", "P2 TA Delay", NULL,          NULL, NULL, {0, NULL}},
	{"port 2", "P2 TB Delay", NULL,          NULL, NULL, {0, NULL}},
	{
		"port 3", "controller 3", "disable",
		"# possible values: disable, standard",
		NULL,
		{LENGTH(opt_controller) - 1, opt_controller}
	},
	{
		"port 3", "pad 3 type", "auto",
		"# possible values: auto, original, 3rdparty",
		NULL,
		{LENGTH(opt_pad_type), opt_pad_type}
	},
	{"port 3", "P3K A",       "NULL",        "# player 3 keyboard", NULL, {0, NULL}},
	{"port 3", "P3K B",       "NULL",        NULL, NULL, {0, NULL}},
	{"port 3", "P3K Select",  "NULL",        NULL, NULL, {0, NULL}},
	{"port 3", "P3K Start",   "NULL",        NULL, NULL, {0, NULL}},
	{"port 3", "P3K Up",      "NULL",        NULL, NULL, {0, NULL}},
	{"port 3", "P3K Down",    "NULL",        NULL, NULL, {0, NULL}},
	{"port 3", "P3K Left",    "NULL",        NULL, NULL, {0, NULL}},
	{"port 3", "P3K Right",   "NULL",        NULL, NULL, {0, NULL}},
	{"port 3", "P3K TurboA",  "NULL",        NULL, NULL, {0, NULL}},
	{"port 3", "P3K TurboB",  "NULL",        NULL, NULL, {0, NULL}},
	{"port 3", "P3J A",       "JB1",         "# player 3 joystick", NULL, {0, NULL}},
	{"port 3", "P3J B",       "JB0",         NULL, NULL, {0, NULL}},
	{"port 3", "P3J Select",  "JB8",         NULL, NULL, {0, NULL}},
	{"port 3", "P3J Start",   "JB9",         NULL, NULL, {0, NULL}},
	{"port 3", "P3J Up",      "JA1MIN",      NULL, NULL, {0, NULL}},
	{"port 3", "P3J Down",    "JA1PLS",      NULL, NULL, {0, NULL}},
	{"port 3", "P3J Left",    "JA0MIN",      NULL, NULL, {0, NULL}},
	{"port 3", "P3J Right",   "JA0PLS",      NULL, NULL, {0, NULL}},
	{"port 3", "P3J TurboA",  "JB2",         NULL, NULL, {0, NULL}},
	{"port 3", "P3J TurboB",  "JB3",         NULL, NULL, {0, NULL}},
	{"port 3", "P3J Id",      "NULL",        NULL, NULL, {0, NULL}},
	{"port 3", "P3 TA Delay", NULL,          NULL, NULL, {0, NULL}},
	{"port 3", "P3 TB Delay", NULL,          NULL, NULL, {0, NULL}},
	{
		"port 4", "controller 4", "disable",
		"# possible values: disable, standard",
		NULL,
		{LENGTH(opt_controller) - 1, opt_controller}
	},
	{
		"port 4", "pad 4 type", "auto",
		"# possible values: auto, original, 3rdparty",
		NULL,
		{LENGTH(opt_pad_type), opt_pad_type}
	},
	{"port 4", "P4K A",       "NULL",        "# player 4 keyboard", NULL, {0, NULL}},
	{"port 4", "P4K B",       "NULL",        NULL, NULL, {0, NULL}},
	{"port 4", "P4K Select",  "NULL",        NULL, NULL, {0, NULL}},
	{"port 4", "P4K Start",   "NULL",        NULL, NULL, {0, NULL}},
	{"port 4", "P4K Up",      "NULL",        NULL, NULL, {0, NULL}},
	{"port 4", "P4K Down",    "NULL",        NULL, NULL, {0, NULL}},
	{"port 4", "P4K Left",    "NULL",        NULL, NULL, {0, NULL}},
	{"port 4", "P4K Right",   "NULL",        NULL, NULL, {0, NULL}},
	{"port 4", "P4K TurboA",  "NULL",        NULL, NULL, {0, NULL}},
	{"port 4", "P4K TurboB",  "NULL",        NULL, NULL, {0, NULL}},
	{"port 4", "P4J A",       "JB1",         "# player 4 joystick", NULL, {0, NULL}},
	{"port 4", "P4J B",       "JB0",         NULL, NULL, {0, NULL} },
	{"port 4", "P4J Select",  "JB8",         NULL, NULL, {0, NULL}},
	{"port 4", "P4J Start",   "JB9",         NULL, NULL, {0, NULL}},
	{"port 4", "P4J Up",      "JA1MIN",      NULL, NULL, {0, NULL}},
	{"port 4", "P4J Down",    "JA1PLS",      NULL, NULL, {0, NULL}},
	{"port 4", "P4J Left",    "JA0MIN",      NULL, NULL, {0, NULL}},
	{"port 4", "P4J Right",   "JA0PLS",      NULL, NULL, {0, NULL}},
	{"port 4", "P4J TurboA",  "JB2",         NULL, NULL, {0, NULL}},
	{"port 4", "P4J TurboB",  "JB3",         NULL, NULL, {0, NULL}},
	{"port 4", "P4J Id",      "NULL",        NULL, NULL, {0, NULL}},
	{"port 4", "P4 TA Delay", NULL,          NULL, NULL, {0, NULL}},
	{"port 4", "P4 TB Delay", NULL,          NULL, NULL, {0, NULL}},
	{
		"system", "controller mode", "nes",
		"# possible values: nes, famicom, fourscore",
		NULL,
		{LENGTH(opt_controller_mode), opt_controller_mode}
	},
	{
		"system", "permit up+down left+right", "no",
		"# possible values: yes, no",
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
	}
};

static const _list_settings list_settings[] = {
	{main_cfg, LENGTH(main_cfg)},
	{pgs_cfg, LENGTH(pgs_cfg)},
	{inp_cfg, LENGTH(inp_cfg)}
};

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void settings_init(void);
EXTERNC void settings_save(void);
EXTERNC void settings_save_GUI(void);
EXTERNC void settings_set_overscan_default(_overscan_borders *ob, BYTE mode);
EXTERNC int settings_val_to_int(int index, const char *buffer);
EXTERNC double settings_val_to_double(WORD round, const char *buffer);
EXTERNC void settings_val_to_oscan(int index, _overscan_borders *ob, const char *buffer);

EXTERNC void settings_pgs_parse(void);
EXTERNC void settings_pgs_save(void);

EXTERNC void *settings_inp_rd_sc(int index, int type);
EXTERNC void settings_inp_wr_sc(void *str, int index, int type);
EXTERNC void settings_inp_all_default(_config_input *config_input, _array_pointers_port *array);
EXTERNC void settings_inp_port_default(_port *port, int index, int mode);
EXTERNC void settings_inp_save(void);

#undef EXTERNC

#endif /* SETTINGS_H_ */
