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
	SET_BATTERY_RAM_FILE_EVEY_TOT,
	SET_BCK_PAUSE,
	SET_CHEAT_MODE,
	SET_SAVE_SETTINGS_ON_EXIT,
#if defined (WITH_OPENGL)
	SET_RENDERING,
#endif
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
	SET_FILE_SHADER,
	SET_PALETTE,
	SET_FILE_PALETTE,
	SET_SWAP_EMPHASIS_PAL,
	SET_VSYNC,
	SET_INTERPOLATION,
	SET_TEXT_ON_SCREEN,
	SET_INPUT_DISPLAY,
	SET_DISABLE_TV_NOISE,
	SET_DISABLE_SEPIA_PAUSE,
#if defined (WITH_OPENGL)
	SET_DISABLE_SRGB_FBO,
#endif
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
	SET_APU_EXTRA,
	SET_HIDE_SPRITES,
	SET_HIDE_BACKGROUND,
	SET_UNLIMITED_SPRITES,
};
enum pgs_element {
	SET_PGS_SLOT,
	SET_PGS_FILE_SAVE,
	SET_PGS_OVERSCAN,
	SET_PGS_DIPSWITCH,
	SET_PGS_PPU_OVERCLOCK,
	SET_PGS_PPU_OVERCLOCK_7BIT,
	SET_PGS_PPU_OVERCLOCK_VB_SCLINE,
	SET_PGS_PPU_OVERCLOCK_PR_SCLINE
};
enum inp_element {
	SET_INP_SC_OPEN,
	SET_INP_SC_QUIT,
	SET_INP_SC_TURN_OFF,
	SET_INP_SC_HARD_RESET,
	SET_INP_SC_SOFT_RESET,
	SET_INP_SC_INSERT_COIN,
	SET_INP_SC_SWITCH_SIDES,
	SET_INP_SC_EJECT_DISK,
	SET_INP_SC_FULLSCREEN,
	SET_INP_SC_PAUSE,
	SET_INP_SC_FAST_FORWARD,
	SET_INP_SC_SCREENSHOT,
	SET_INP_SC_MODE_PAL,
	SET_INP_SC_MODE_NTSC,
	SET_INP_SC_MODE_DENDY,
	SET_INP_SC_MODE_AUTO,
	SET_INP_SC_SCALE_1X,
	SET_INP_SC_SCALE_2X,
	SET_INP_SC_SCALE_3X,
	SET_INP_SC_SCALE_4X,
	SET_INP_SC_SCALE_5X,
	SET_INP_SC_SCALE_6X,
	SET_INP_SC_INTERPOLATION,
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

typedef struct _opt {
	const uTCHAR *lname;
	const uTCHAR *sname;
	int value;
} _opt;
typedef struct _settings {
	const uTCHAR *grp;
	const uTCHAR *key;
	const uTCHAR *def;

	const uTCHAR *cmt;
	const uTCHAR *hlp;

	struct _opts {
		const int count;
		const _opt *opt;
	} opts;
} _settings;
typedef struct _list_settings {
	const _settings *cfg;
	const int count;
} _list_settings;

static const _opt opt_no_yes[] = {
	{NULL, uL("no") , FALSE},
	{NULL, uL("yes"), TRUE}
};
static const _opt opt_off_on[] = {
	{NULL, uL("off"), FALSE},
	{NULL, uL("on") , TRUE},
};
static const _opt opt_mode[] = {
	{uL("Auto"),  uL("auto") , AUTO},
	{uL("NTSC"),  uL("ntsc") , NTSC},
	{uL("PAL") ,  uL("pal")  , PAL},
	{uL("Dendy"), uL("dendy"), DENDY}
};
static const _opt opt_ff_velocity[] = {
	{NULL, uL("2x"), FF_2X},
	{NULL, uL("3x"), FF_3X},
	{NULL, uL("4x"), FF_4X},
	{NULL, uL("5x"), FF_5X}
};
#if defined (WITH_OPENGL)
static const _opt opt_rend[] = {
	{uL("Software"), uL("software"), RENDER_SOFTWARE},
	{uL("GLSL")    , uL("glsl")    , RENDER_GLSL}
};
#endif
static const _opt opt_fps[] = {
	{NULL, uL("default"), FPS_DEFAULT},
	{NULL, uL("60")     , FPS_60},
	{NULL, uL("59")     , FPS_59},
	{NULL, uL("58")     , FPS_58},
	{NULL, uL("57")     , FPS_57},
	{NULL, uL("56")     , FPS_56},
	{NULL, uL("55")     , FPS_55},
	{NULL, uL("54")     , FPS_54},
	{NULL, uL("53")     , FPS_53},
	{NULL, uL("52")     , FPS_52},
	{NULL, uL("51")     , FPS_51},
	{NULL, uL("50")     , FPS_50},
	{NULL, uL("49")     , FPS_49},
	{NULL, uL("48")     , FPS_48},
	{NULL, uL("47")     , FPS_47},
	{NULL, uL("46")     , FPS_46},
	{NULL, uL("45")     , FPS_45},
	{NULL, uL("44")     , FPS_44}
};
static const _opt opt_fsk[] = {
	{NULL, uL("default"), 0},
	{NULL, uL("1")      , 1},
	{NULL, uL("2")      , 2},
	{NULL, uL("3")      , 3},
	{NULL, uL("4")      , 4},
	{NULL, uL("5")      , 5},
	{NULL, uL("6")      , 6},
	{NULL, uL("7")      , 7},
	{NULL, uL("8")      , 8},
	{NULL, uL("9")      , 9}
};
static const _opt opt_scale[] = {
	{NULL, uL("1x"), X1},
	{NULL, uL("2x"), X2},
	{NULL, uL("3x"), X3},
	{NULL, uL("4x"), X4},
	{NULL, uL("5x"), X5},
	{NULL, uL("6x"), X6}
};
static const _opt opt_par[] = {
	{NULL, uL("1:1") , PAR11},
	{NULL, uL("5:4") , PAR54},
	{NULL, uL("8:7") , PAR87},
	{NULL, uL("11:8"), PAR118}
};
static const _opt opt_oscan[] = {
	{NULL, uL("off")    , OSCAN_OFF},
	{NULL, uL("on")     , OSCAN_ON},
	{NULL, uL("default"), OSCAN_DEFAULT}
};
static const _opt opt_filter[] = {
	{uL("no filter")            , uL("none")        , NO_FILTER},
	{uL("Scale2X")              , uL("scale2x")     , SCALE2X},
	{uL("Scale3X")              , uL("scale3x")     , SCALE3X},
	{uL("Scale4X")              , uL("scale4x")     , SCALE4X},
	{uL("Hq2X")                 , uL("hq2x")        , HQ2X},
	{uL("Hq3X")                 , uL("hq3x")        , HQ3X},
	{uL("Hq4X")                 , uL("hq4x")        , HQ4X},
	{uL("NTSC")                 , uL("ntsc")        , NTSC_FILTER},
	{uL("xBRZ 2x")              , uL("xbrz2x")      , XBRZ2X},
	{uL("xBRZ 3x")              , uL("xbrz3x")      , XBRZ3X},
	{uL("xBRZ 4x")              , uL("xbrz4x")      , XBRZ4X},
	{uL("xBRZ 5x")              , uL("xbrz5x")      , XBRZ5X},
	{uL("xBRZ 6x")              , uL("xbrz6x")      , XBRZ6X},
	// per filtri CPU futuri
	{NULL                       , NULL              , NO_FILTER},
	{NULL                       , NULL              , NO_FILTER},
	{NULL                       , NULL              , NO_FILTER},
	{NULL                       , NULL              , NO_FILTER},
	{NULL                       , NULL              , NO_FILTER},
	{NULL                       , NULL              , NO_FILTER},
	{NULL                       , NULL              , NO_FILTER},
	{NULL                       , NULL              , NO_FILTER},
	{NULL                       , NULL              , NO_FILTER},
	{NULL                       , NULL              , NO_FILTER},
	{NULL                       , NULL              , NO_FILTER},
	{NULL                       , NULL              , NO_FILTER},
	{NULL                       , NULL              , NO_FILTER},
	// shaders
	{uL("CRT Dotmask")          , uL("crtdotmask")  , SHADER_CRTDOTMASK},
	{uL("CRT Scanlines")        , uL("crtscanlines"), SHADER_CRTSCANLINES},
	{uL("CRT With Curve")       , uL("crtcurve")    , SHADER_CRTWITHCURVE},
	{uL("Emboss")               , uL("emboss")      , SHADER_EMBOSS},
	{uL("Noise")                , uL("noise")       , SHADER_NOISE},
	{uL("NTSC 2Phase Composite"), uL("ntsc2phcomp") , SHADER_NTSC2PHASECOMPOSITE},
	{uL("Old TV")               , uL("oldtv")       , SHADER_OLDTV},
	{uL("Extern")               , uL("file")        , SHADER_FILE}
};
static const _opt opt_ntsc[] = {
	{uL("Composite"), uL("composite"), COMPOSITE},
	{uL("S-Video")  , uL("svideo")   , SVIDEO},
	{uL("RGB")      , uL("rgb")      , RGBMODE}
};
static const _opt opt_palette[] = {
	{uL("PAL palette")      , uL("pal")   , PALETTE_PAL},
	{uL("NTSC palette")     , uL("ntsc")  , PALETTE_NTSC},
	{uL("Sony CXA2025AS US"), uL("sony")  , PALETTE_SONY},
	{uL("Monochrome")       , uL("mono")  , PALETTE_MONO},
	{uL("Green")            , uL("green") , PALETTE_GREEN},
	{uL("Extern")           , uL("file")  , PALETTE_FILE},
	{uL("Firebrandx Nstlg") , uL("frbnst"), PALETTE_FRBX_NOSTALGIA},
	{uL("Firebrandx YUV")   , uL("frbyuv"), PALETTE_FRBX_YUV}
};
static const _opt opt_audio_buffer_factor[] = {
	{NULL, uL("0"),  0},
	{NULL, uL("1"),  1},
	{NULL, uL("2"),  2},
	{NULL, uL("3"),  3},
	{NULL, uL("4"),  4},
	{NULL, uL("5"),  5},
	{NULL, uL("6"),  6},
	{NULL, uL("7"),  7},
	{NULL, uL("8"),  8},
	{NULL, uL("9"),  9},
	{NULL, uL("10"), 10},
	{NULL, uL("11"), 11},
	{NULL, uL("12"), 12},
	{NULL, uL("13"), 13},
	{NULL, uL("14"), 14},
	{NULL, uL("15"), 15}
};
static const _opt opt_samplerate[] = {
	{NULL, uL("48000"), S48000},
	{NULL, uL("44100"), S44100},
	{NULL, uL("22050"), S22050},
	{NULL, uL("11025"), S11025}
};
static const _opt opt_channels[] = {
	{NULL, uL("mono"),    CH_MONO},
	{NULL, uL("delay"),   CH_STEREO_DELAY},
	{NULL, uL("panning"), CH_STEREO_PANNING},
};
static const _opt opt_audio_quality[] = {
	{NULL, uL("low"),  AQ_LOW},
	{NULL, uL("high"), AQ_HIGH}
};
static const _opt opt_cheat_mode[] = {
	{NULL, uL("disabled"),   NOCHEAT_MODE},
	{NULL, uL("gamegenie"),  GAMEGENIE_MODE},
	{NULL, uL("cheatslist"), CHEATSLIST_MODE}
};
static const _opt opt_languages[] = {
	{NULL, uL("english"), LNG_ENGLISH},
	{NULL, uL("italian"), LNG_ITALIAN},
	{NULL, uL("russian"), LNG_RUSSIAN}
};

static const _opt opt_slot_pgs[] = {
	{NULL, uL("0"), 0},
	{NULL, uL("1"), 1},
	{NULL, uL("2"), 2},
	{NULL, uL("3"), 3},
	{NULL, uL("4"), 4},
	{NULL, uL("5"), 5}
};

static const _opt opt_controller[] = {
	{NULL, uL("disable"),  CTRL_DISABLED},
	{NULL, uL("standard"), CTRL_STANDARD},
	{NULL, uL("zapper"),   CTRL_ZAPPER}
};
static const _opt opt_controller_mode[] = {
	{NULL, uL("nes"),        CTRL_MODE_NES},
	{NULL, uL("famicom"),    CTRL_MODE_FAMICOM},
	{NULL, uL("four score"), CTRL_MODE_FOUR_SCORE}
};
static const _opt opt_pad_type[] = {
	{NULL, uL("auto"),     CTRL_PAD_AUTO},
	{NULL, uL("original"), CTRL_PAD_ORIGINAL},
	{NULL, uL("3rdparty"), CTRL_PAD_3RD_PARTY},
};

static const _settings main_cfg[] = {
	{
		uL("system"), uL("preferred mode"), uL("auto"),
		uL("# possible values: pal, ntsc, dendy, auto"),
		uL("-m, --mode                preferred mode        : pal, ntsc, dendy, auto"),
		{LENGTH(opt_mode), opt_mode}
	},
	{
		uL("system"), uL("fast forward velocity"), uL("2x"),
		uL("# possible values: 2x, 3x, 4x, 5x"),
		NULL,
		{LENGTH(opt_ff_velocity), opt_ff_velocity}
	},
	{
		uL("system"), uL("save battery ram file every 180 sec"), uL("no"),
		uL("# possible values: yes, no"),
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("system"), uL("pause when in background"), uL("yes"),
		uL("# possible values: yes, no"),
		uL("    --background-pause                          : yes, no"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("system"), uL("cheat mode"), uL("disabled"),
		uL("# possible values: disabled, gamegenie, cheatslist"),
		uL("-g, --cheat-mode          cheat mode            : disabled, gamegenie, cheatslist"),
		{LENGTH(opt_cheat_mode), opt_cheat_mode}
	},
	{
		uL("system"), uL("save settings on exit"), uL("no"),
		uL("# possible values: yes, no"),
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
	},
#if defined (WITH_OPENGL)
	{
		uL("video"), uL("rendering"), uL("glsl"),
		uL("# possible values: software, glsl"),
		uL("-r, --rendering           type of rendering     : software, glsl"),
		{LENGTH(opt_rend), opt_rend}
	},
#endif
	{
		uL("video"), uL("frames per second"), uL("default"),
		uL("# possible values: default, 58, 57. ..., 45, 44"),
		uL("-f, --fps                 frames per second     : default, 60, ..., 44"),
		{LENGTH(opt_fps), opt_fps}
	},
	{
		uL("video"), uL("frame skip"), uL("default"),
		uL("# possible values: default, 1, ..., 9"),
		uL("-k, --frameskip           frames to skip        : default, 1, ..., 9"),
		{LENGTH(opt_fsk), opt_fsk}
	},
	{
		uL("video"), uL("size window"), uL("2x"),
		uL("# possible values: 1x, 2x, 3x, 4x, 5x, 6x" NEWLINE)
		uL("# Note : 1x works only with \'filter=none\'" NEWLINE)
		uL("# and software filters ScaleXX, HqXX and NTSC" NEWLINE)
		uL("# don't supports 5x and 6x."),
		uL("-s, --size                window size           : 1x, 2x, 3x, 4x, 5x, 6x"),
		{LENGTH(opt_scale), opt_scale}
	},
	{
		uL("video"), uL("pixel aspect ratio"), uL("8:7"),
		uL("# possible values: 1:1, 5:4, 8:7, 11:8"),
		uL("-e, --pixel-aspect-ratio  enable aspect ratio   : 1:1, 5:4, 8:7, 11:8"),
		{LENGTH(opt_par), opt_par}
	},
	{
		uL("video"), uL("pixel aspect ratio soft stretch"), uL("yes"),
		uL("# possible values: yes, no"),
		uL("    --par-soft-stretch    improves the          : yes, no" NEWLINE)
		uL("                          stretched image"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("video"), uL("overscan default"), uL("off"),
		uL("# possible values: on, off"),
		uL("-o, --overscan            default overscan      : on, off"),
		{LENGTH(opt_oscan), opt_oscan}
	},
	{
		uL("video"), uL("overscan bordes NTSC"), uL("8,8,8,9"),
		uL("# possible values: 0-17" NEWLINE)
		uL("# format string  : [UP],[DOWN],[LEFT],[RIGHT]"),
		uL("    --overscan-brd-ntsc   borders in pixels     : [UP],[DOWN],[LEFT],[RIGHT]"),
		{0, NULL}
	},
	{
		uL("video"), uL("overscan bordes PAL"), uL("8,8,8,9"),
		uL("# possible values: 0-17" NEWLINE)
		uL("# format string  : [UP],[DOWN],[LEFT],[RIGHT]"),
		uL("    --overscan-brd-pal    borders in pixels     : [UP],[DOWN],[LEFT],[RIGHT]"),
		{0, NULL}
	},
	{
		uL("video"), uL("filter"), uL("none"),
		uL("# possible values: none, scale2x, scale3x, scale4x, hq2x, hq3x," NEWLINE)
		uL("#                  hq4x, xbrz2x, xbrz3x, xbrz4x, xbrz5x, xbrz6x, ntsc," NEWLINE)
		uL("#                  crtdotmask, crtscanlines, crtcurve, emboss, noise," NEWLINE)
		uL("#                  ntsc2phcomp, oldtv, file"),
		uL("-i, --filter              filter to apply       : nofilter, scale2x," NEWLINE)
		uL("                                                  scale3x, scale4x, hq2x," NEWLINE)
		uL("                                                  hq3x, hq4x, xbrz2x, xbrz3x," NEWLINE)
		uL("                                                  xbrz4x, xbrz5x, xbrz6x, ntsc," NEWLINE)
		uL("                                                  crtdotmask, crtscanlines," NEWLINE)
		uL("                                                  crtcurve, emboss, noise," NEWLINE)
		uL("                                                  ntsc2phcomp, oldtv, file"),
		{LENGTH(opt_filter), opt_filter}
	},
	{
		uL("video"), uL("ntsc filter format"), uL("composite"),
		uL("# possible values: composite, svideo, rgb"),
		uL("-n, --ntsc-format         format of ntsc filter : composite, svideo, rgb"),
		{LENGTH(opt_ntsc), opt_ntsc}
	},
	{
		uL("video"), uL("shader file"), NULL,
		uL("# possible values: [PATH/NAME]"),
		NULL,
		{0, NULL}
	},
	{
		uL("video"), uL("palette"), uL("ntsc"),
		uL("# possible values: pal, ntsc, sony, frbyuv, frbuns, mono, green, file"),
		uL("-p, --palette             type of palette       : pal, ntsc, sony, frbyuv," NEWLINE)
		uL("                                                  frbuns, mono, green, file"),
		{LENGTH(opt_palette), opt_palette}
	},
	{
		uL("video"), uL("palette file"), NULL,
		uL("# possible values: [PATH/NAME]"),
		NULL,
		{0, NULL}
	},
	{
		uL("video"), uL("disable swap emphasis PAL-Dendy"), uL("no"),
		uL("# possible values: yes, no"),
		uL("    --swap-emphasis       disable swap emphasis : yes, no" NEWLINE)
		uL("                          in PAL/Dendy mode"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("video"), uL("vsync"), uL("on"),
		uL("# possible values: on, off"),
		uL("-v, --vsync               use of vsync          : on, off"),
		{LENGTH(opt_off_on), opt_off_on}
	},
	{
		uL("video"), uL("interpolation"), uL("no"),
		uL("# possible values: yes, no"),
		uL("-j, --interpolation       enable interpolation  : yes, no"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("video"), uL("text on screen"), uL("yes"),
		uL("# possible values: yes, no"),
		uL("    --txt-on-screen       enable messages       : yes, no"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("video"), uL("input display"), uL("no"),
		uL("# possible values: yes, no"),
		uL("    --input-display       enable input gui      : yes, no"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("video"), uL("disable tv noise emulation"), uL("no"),
		uL("# possible values: yes, no"),
		uL("    --disable-tv-noise    disable tv noise      : yes, no"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("video"), uL("disable sepia color on pause"), uL("no"),
		uL("# possible values: yes, no"),
		uL("    --disable-sepia       disable sepia color   : yes, no" NEWLINE)
		uL("                          during the pause"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
#if defined (WITH_OPENGL)
	{
		uL("video"), uL("disable sRGB FBO (Shaders)"), uL("no"),
		uL("# possible values: yes, no"),
		uL("    --disable-srgb-fbo    disable in the        : yes, no" NEWLINE)
		uL("                          shaders the use of" NEWLINE)
		uL("                          sRGB FBO"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
#endif
	{
		uL("video"), uL("fullscreen"), uL("no"),
		uL("# possible values: yes, no"),
		uL("-u, --fullscreen          no comment            : yes, no"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("video"), uL("stretch in fullscreen"), uL("no"),
		uL("# possible values: yes, no"),
		uL("-t, --stretch-fullscreen  stretch image         : yes, no"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("audio"), uL("buffer factor"), uL("1"),
		uL("# possible values: [0-15]"),
		uL("-b, --audio-buffer-factor buffer size factor    : [0-15]"),
		{LENGTH(opt_audio_buffer_factor), opt_audio_buffer_factor}
	},
	{
		uL("audio"), uL("sample rate"), uL("44100"),
		uL("# possible values: 48000, 44100, 22050, 11025"),
		uL("-l, --samplerate          sample rate           : 48000, 44100, 22050, 11025"),
		{LENGTH(opt_samplerate), opt_samplerate}
	},
	{
		uL("audio"), uL("channels"), uL("delay"),
		uL("# possible values: mono, delay, panning"),
		uL("-c, --channels            audio channels        : mono, delay, panning"),
		{LENGTH(opt_channels), opt_channels}
	},
	{
		uL("audio"), uL("stereo delay"), uL("30"),
		uL("# possible values: [5 - 100]"),
		uL("-d, --stereo-delay        stereo effect delay   : [5 - 100]"),
		{0, NULL}
	},
	{
		uL("audio"), uL("audio quality"), uL("high"),
		uL("# possible values: low, high"),
		uL("-q, --audio-quality       audio quality         : low, high"),
		{LENGTH(opt_audio_quality), opt_audio_quality}
	},
	{
		uL("audio"), uL("swap duty cycles (Famicom clone chip audio emulation)"), uL("no"),
		uL("# possible values: yes, no"),
		uL("    --swap-duty           swap duty cycles      : yes, no"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("audio"), uL("audio"), uL("on"),
		uL("# possible values: on, off"),
		uL("-a, --audio                                     : on, off"),
		{LENGTH(opt_off_on), opt_off_on}
	},
	{
		uL("GUI"), uL("last open path"), NULL,
		uL("# possible values: [PATH]"),
		NULL,
		{0, NULL}
	},
	{
		uL("GUI"), uL("last position of window"), uL("0,0"),
		uL("# possible values: [X],[Y]"),
		NULL,
		{0, NULL}
	},
	{
		uL("GUI"), uL("language"), uL("english"),
		uL("# possible values: english,italian,russian"),
		uL("    --language            GUI language          : english,italian,russian"),
		{LENGTH(opt_languages), opt_languages}
	},
	{
		uL("apu channels"), uL("master"), uL("on,100"),
		uL("# possible values: [on, off],[0 - 100]"),
		NULL,
		{0, NULL}
	},
	{
		uL("apu channels"), uL("square1"), uL("on,100"),
		uL("# possible values: [on, off],[0 - 100]"),
		NULL,
		{0, NULL}
	},
	{
		uL("apu channels"), uL("square2"), uL("on,100"),
		uL("# possible values: [on, off],[0 - 100]"),
		NULL,
		{0, NULL}
	},
	{
		uL("apu channels"), uL("triangle"), uL("on,100"),
		uL("# possible values: [on, off],[0 - 100]"),
		NULL,
		{0, NULL}
	},
	{
		uL("apu channels"), uL("noise"), uL("on,100"),
		uL("# possible values: [on, off],[0 - 100]"),
		NULL,
		{0, NULL}
	},
	{
		uL("apu channels"), uL("dmc"), uL("on,100"),
		uL("# possible values: [on, off],[0 - 100]"),
		NULL,
		{0, NULL}
	},
	{
		uL("apu channels"), uL("extra"), uL("on,100"),
		uL("# possible values: [on, off],[0 - 100]"),
		NULL,
		{0, NULL}
	},
	{
		uL("ppu"), uL("hide sprites"), uL("no"),
		uL("# possible values: yes, no"),
		uL("    --hide-sprites                              : yes, no"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("ppu"), uL("hide background"), uL("no"),
		uL("# possible values: yes, no"),
		uL("    --hide-background                           : yes, no"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("ppu"), uL("unlimited sprites"), uL("no"),
		uL("# possible values: yes, no"),
		uL("    --unlimited-sprites                         : yes, no"),
		{LENGTH(opt_no_yes), opt_no_yes}
	}
};

static const _settings pgs_cfg[] = {
	{
		uL("state"), uL("last save slot"), uL("0"),
		uL("# possible values: 0, 1, 2, 3, 4, 5"),
		NULL,
		{LENGTH(opt_slot_pgs), opt_slot_pgs}
	},
	{
		uL("state"), uL("last save file used"), NULL,
		uL("# possible values: [PATH/NAME]"),
		NULL,
		{0, NULL}
	},
	{
		uL("video"), uL("overscan"), uL("default"),
		uL("# possible values: on, off, default"),
		NULL,
		{LENGTH(opt_oscan), opt_oscan}
	},
	{
		uL("system"), uL("dipswitch"), uL("65280"),
		uL("# possible values: [it depends on the mapper]"),
		NULL,
		{0, NULL}
	},
	{
		uL("system"), uL("ppu overclock"), uL("no"),
		uL("# possible values: yes, no"),
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("system"), uL("disable 7bit sample control"), uL("no"),
		uL("# possible values: yes, no"),
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("system"), uL("extra vblank scanlines"), uL("0"),
		uL("# possible values: [0 - 1000]"),
		NULL,
		{0, NULL}
	},
	{
		uL("system"), uL("extra postrender scanlines"), uL("0"),
		uL("# possible values: [0 - 1000]"),
		NULL,
		{0, NULL}
	}
};

static const _settings inp_cfg[] = {
	{uL("shortcuts"), uL("open"),                     uL("Alt+O,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("quit"),                     uL("Alt+Q,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("turn off"),                 uL("Alt+R,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("hard reset"),               uL("F11,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("soft reset"),               uL("F12,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("insert coin"),              uL("8,NULL"),          NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("switch sides"),             uL("Alt+S,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("eject disk"),               uL("Alt+E,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("video fullscreen"),         uL("Alt+Return,NULL"), NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("pause"),                    uL("Pause,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("fast forward"),             uL("Tab,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("save screenshot"),          uL("Alt+X,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("mode pal"),                 uL("F6,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("mode ntsc"),                uL("F7,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("mode dendy"),               uL("F8,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("mode auto"),                uL("F9,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("scale 1x"),                 uL("Alt+1,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("scale 2x"),                 uL("Alt+2,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("scale 3x"),                 uL("Alt+3,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("scale 4x"),                 uL("Alt+4,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("scale 5x"),                 uL("Alt+5,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("scale 6x"),                 uL("Alt+6,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("video interpolation"),      uL("0,NULL"),          NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("video stretch fullscreen"), uL("Alt+P,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("audio enable"),             uL("Alt+A,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("save settings"),            uL("Alt+W,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("save state"),               uL("F1,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("load state"),               uL("F4,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("increment state slot"),     uL("F3,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("decrement state slot"),     uL("F2,NULL"),         NULL, NULL, {0, NULL}},

	{uL("shortcuts"), uL("joystick Id"),              uL("NULL"),            NULL, NULL, {0, NULL}},

	{uL("special keys"), uL("timeline key"),          uL("LCtrl"),           NULL, NULL, {0, NULL}},

	{
		uL("port 1"), uL("controller 1"), uL("standard"),
		uL("# possible values: disable, standard, zapper"),
		NULL,
		{LENGTH(opt_controller), opt_controller}
	},
	{
		uL("port 1"), uL("pad 1 type"), uL("auto"),
		uL("# possible values: auto, original, 3rdparty"),
		NULL,
		{LENGTH(opt_pad_type), opt_pad_type}
	},
	{uL("port 1"), uL("P1K A"),       uL("S"),           uL("# player 1 keyboard"), NULL, {0, NULL}},
	{uL("port 1"), uL("P1K B"),       uL("A"),           NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1K Select"),  uL("Z"),           NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1K Start"),   uL("X"),           NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1K Up"),      uL("Up"),          NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1K Down"),    uL("Down"),        NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1K Left"),    uL("Left"),        NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1K Right"),   uL("Right"),       NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1K TurboA"),  uL("W"),           NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1K TurboB"),  uL("Q"),           NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1J A"),       uL("JB1"),         uL("# player 1 joystick"), NULL, {0, NULL}},
	{uL("port 1"), uL("P1J B"),       uL("JB0"),         NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1J Select"),  uL("JB8"),         NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1J Start"),   uL("JB9"),         NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1J Up"),      uL("JA1MIN"),      NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1J Down"),    uL("JA1PLS"),      NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1J Left"),    uL("JA0MIN"),      NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1J Right"),   uL("JA0PLS"),      NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1J TurboA"),  uL("JB2"),         NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1J TurboB"),  uL("JB3"),         NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1J Id"),      uL("JOYSTICKID1"), NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1 TA Delay"), NULL,          NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1 TB Delay"), NULL,          NULL, NULL, {0, NULL}},
	{
		uL("port 2"), uL("controller 2"), uL("disable"),
		uL("# possible values: disable, standard, zapper"),
		NULL,
		{LENGTH(opt_controller), opt_controller}
	},
	{
		uL("port 2"), uL("pad 2 type"), uL("auto"),
		uL("# possible values: auto, original, 3rdparty"),
		NULL,
		{LENGTH(opt_pad_type), opt_pad_type}
	},
	{uL("port 2"), uL("P2K A"),       uL("PgDown"),      uL("# player 2 keyboard"), NULL, {0, NULL}},
	{uL("port 2"), uL("P2K B"),       uL("End"),         NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2K Select"),  uL("Ins"),         NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2K Start"),   uL("Del"),         NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2K Up"),      uL("NumPad8"),     NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2K Down"),    uL("NumPad2"),     NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2K Left"),    uL("NumPad4"),     NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2K Right"),   uL("NumPad6"),     NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2K TurboA"),  uL("Home"),        NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2K TurboB"),  uL("PgUp"),        NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2J A"),       uL("JB1"),         uL("# player 2 joystick"), NULL, {0, NULL}},
	{uL("port 2"), uL("P2J B"),       uL("JB0"),         NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2J Select"),  uL("JB8"),         NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2J Start"),   uL("JB9"),         NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2J Up"),      uL("JA1MIN"),      NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2J Down"),    uL("JA1PLS"),      NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2J Left"),    uL("JA0MIN"),      NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2J Right"),   uL("JA0PLS"),      NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2J TurboA"),  uL("JB2"),         NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2J TurboB"),  uL("JB3"),         NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2J Id"),      uL("JOYSTICKID2"), NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2 TA Delay"), NULL,          NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2 TB Delay"), NULL,          NULL, NULL, {0, NULL}},
	{
		uL("port 3"), uL("controller 3"), uL("disable"),
		uL("# possible values: disable, standard"),
		NULL,
		{LENGTH(opt_controller) - 1, opt_controller}
	},
	{
		uL("port 3"), uL("pad 3 type"), uL("auto"),
		uL("# possible values: auto, original, 3rdparty"),
		NULL,
		{LENGTH(opt_pad_type), opt_pad_type}
	},
	{uL("port 3"), uL("P3K A"),       uL("NULL"),        uL("# player 3 keyboard"), NULL, {0, NULL}},
	{uL("port 3"), uL("P3K B"),       uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3K Select"),  uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3K Start"),   uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3K Up"),      uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3K Down"),    uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3K Left"),    uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3K Right"),   uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3K TurboA"),  uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3K TurboB"),  uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3J A"),       uL("JB1"),         uL("# player 3 joystick"), NULL, {0, NULL}},
	{uL("port 3"), uL("P3J B"),       uL("JB0"),         NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3J Select"),  uL("JB8"),         NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3J Start"),   uL("JB9"),         NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3J Up"),      uL("JA1MIN"),      NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3J Down"),    uL("JA1PLS"),      NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3J Left"),    uL("JA0MIN"),      NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3J Right"),   uL("JA0PLS"),      NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3J TurboA"),  uL("JB2"),         NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3J TurboB"),  uL("JB3"),         NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3J Id"),      uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3 TA Delay"), NULL,          NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3 TB Delay"), NULL,          NULL, NULL, {0, NULL}},
	{
		uL("port 4"), uL("controller 4"), uL("disable"),
		uL("# possible values: disable, standard"),
		NULL,
		{LENGTH(opt_controller) - 1, opt_controller}
	},
	{
		uL("port 4"), uL("pad 4 type"), uL("auto"),
		uL("# possible values: auto, original, 3rdparty"),
		NULL,
		{LENGTH(opt_pad_type), opt_pad_type}
	},
	{uL("port 4"), uL("P4K A"),       uL("NULL"),        uL("# player 4 keyboard"), NULL, {0, NULL}},
	{uL("port 4"), uL("P4K B"),       uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4K Select"),  uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4K Start"),   uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4K Up"),      uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4K Down"),    uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4K Left"),    uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4K Right"),   uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4K TurboA"),  uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4K TurboB"),  uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4J A"),       uL("JB1"),         uL("# player 4 joystick"), NULL, {0, NULL}},
	{uL("port 4"), uL("P4J B"),       uL("JB0"),         NULL, NULL, {0, NULL} },
	{uL("port 4"), uL("P4J Select"),  uL("JB8"),         NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4J Start"),   uL("JB9"),         NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4J Up"),      uL("JA1MIN"),      NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4J Down"),    uL("JA1PLS"),      NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4J Left"),    uL("JA0MIN"),      NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4J Right"),   uL("JA0PLS"),      NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4J TurboA"),  uL("JB2"),         NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4J TurboB"),  uL("JB3"),         NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4J Id"),      uL("NULL"),        NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4 TA Delay"), NULL,          NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4 TB Delay"), NULL,          NULL, NULL, {0, NULL}},
	{
		uL("system"), uL("controller mode"), uL("nes"),
		uL("# possible values: nes, famicom, fourscore"),
		NULL,
		{LENGTH(opt_controller_mode), opt_controller_mode}
	},
	{
		uL("system"), uL("permit up+down left+right"), uL("no"),
		uL("# possible values: yes, no"),
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
EXTERNC int settings_val_to_int(int index, const uTCHAR *buffer);
EXTERNC double settings_val_to_double(WORD round, const uTCHAR *buffer);
EXTERNC void settings_val_to_oscan(int index, _overscan_borders *ob, const uTCHAR *buffer);

EXTERNC void settings_pgs_parse(void);
EXTERNC void settings_pgs_save(void);

EXTERNC void *settings_inp_rd_sc(int index, int type);
EXTERNC void settings_inp_wr_sc(void *str, int index, int type);
EXTERNC void settings_inp_all_default(_config_input *config_input, _array_pointers_port *array);
EXTERNC void settings_inp_port_default(_port *port, int index, int mode);
EXTERNC void settings_inp_save(void);

#undef EXTERNC

#endif /* SETTINGS_H_ */
