/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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
#include "video/gfx.h"
#include "fps.h"
#include "audio/snd.h"
#include "cheat.h"
#include "audio/channels.h"
#include "overscan.h"
#include "input.h"
#include "nsf.h"
#include "rewind.h"
#include "palette.h"

#if defined (_WIN32)
#define NEWLINE "\r\n"
#else
#define NEWLINE "\n"
#endif

enum set_element {
	SET_MODE,
	SET_FF_VELOCITY,
	SET_REWIND_MINUTES,
	SET_BATTERY_RAM_FILE_EVEY_TOT,
	SET_BCK_PAUSE,
	SET_CHEAT_MODE,
	SET_FILE_GAME_GENIE_ROM,
	SET_FILE_FDS_BIOS,
	SET_LAST_IMPORT_CHEAT_PATH,
	SET_SAVE_SETTINGS_ON_EXIT,
	SET_SCALE,
	SET_PAR,
	SET_PAR_SOFT_STRETCH,
	SET_OVERSCAN_BLACK_BORDERS,
	SET_OVERSCAN_BLACK_BORDERS_FSCR,
	SET_OVERSCAN_DEFAULT,
	SET_OVERSCAN_BRD_NTSC,
	SET_OVERSCAN_BRD_PAL,
	SET_FILTER,
	SET_NTSC_FORMAT,
	SET_SHADER,
	SET_FILE_SHADER,
	SET_PALETTE,
	SET_FILE_PALETTE,
	SET_SWAP_EMPHASIS_PAL,
	SET_VSYNC,
	SET_INTERPOLATION,
	SET_TEXT_ON_SCREEN,
	SET_SHOW_FPS,
	SET_INPUT_DISPLAY,
	SET_DISABLE_TV_NOISE,
	SET_DISABLE_SEPIA_PAUSE,
#if defined (WITH_OPENGL)
	SET_DISABLE_SRGB_FBO,
#endif
	SET_FULLSCREEN,
	SET_FULLSCREEN_IN_WINDOW,
	SET_INTEGER_FULLSCREEN,
	SET_STRETCH_FULLSCREEN,
	SET_SCREEN_ROTATION,
	SET_TEXT_ROTATION,
	SET_AUDIO_OUTPUT_DEVICE,
	SET_AUDIO_BUFFER_FACTOR,
	SET_SAMPLERATE,
	SET_CHANNELS,
	SET_STEREO_DELAY,
	SET_SWAP_DUTY,
	SET_AUDIO,
	SET_GUI_OPEN_PATH,
	SET_GUI_OPEN_PATCH_PATH,
	SET_GUI_LAST_POSITION,
	SET_GUI_LAST_POSITION_SETTINGS,
	SET_GUI_LANGUAGE,
	SET_GUI_TOOLBAR_AREA,
	SET_GUI_TOOLBAR_HIDDEN,
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
	SET_NSF_PLAYER_EFFECT,
	SET_NSF_PLAYER_NSFE_PLAYLIST,
	SET_NSF_PLAYER_NSFE_FADEOUT,
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
	SET_INP_SC_WAV,
	SET_INP_SC_FULLSCREEN,
	SET_INP_SC_SCREENSHOT,
	SET_INP_SC_SCREENSHOT_1X,
	SET_INP_SC_PAUSE,
	SET_INP_SC_FAST_FORWARD,
	SET_INP_SC_TOGGLE_GUI_IN_WINDOW,
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
	SET_INP_SC_INTEGER_FULLSCREEN,
	SET_INP_SC_STRETCH_FULLSCREEN,
	SET_INP_SC_AUDIO_ENABLE,
	SET_INP_SC_SAVE_SETTINGS,
	SET_INP_SC_SAVE_STATE,
	SET_INP_SC_LOAD_STATE,
	SET_INP_SC_INC_SLOT,
	SET_INP_SC_DEC_SLOT,
	SET_INP_SC_RWND_ACTIVE_MODE,
	SET_INP_SC_RWND_STEP_BACKWARD,
	SET_INP_SC_RWND_STEP_FORWARD,
	SET_INP_SC_RWND_FAST_BACKWARD,
	SET_INP_SC_RWND_FAST_FORWARD,
	SET_INP_SC_RWND_PLAY,
	SET_INP_SC_RWND_PAUSE,

	SET_INP_SC_JOYSTICK_ID,

	SET_INP_EXPANSION_PORT,
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
	SET_INP_LEFTRIGHT,
	SET_INP_HIDE_ZAPPER_CURSOR
};

enum set_num_shortcut { SET_MAX_NUM_SC = SET_INP_SC_JOYSTICK_ID - SET_INP_SC_OPEN};

enum list_settings_element {
	LSET_SET,
	LSET_PGS,
	LSET_INP,
	LSET_NONE
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
static const _opt opt_screen_rotation[] = {
	{NULL, uL("0"), ROTATE_0},
	{NULL, uL("90"), ROTATE_90},
	{NULL, uL("180"), ROTATE_180},
	{NULL, uL("270"), ROTATE_270}
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
	{uL("xBRZ 2x MT")           , uL("xbrz2mtx")    , XBRZ2XMT},
	{uL("xBRZ 3x MT")           , uL("xbrz3xmt")    , XBRZ3XMT},
	{uL("xBRZ 4x MT")           , uL("xbrz4xmt")    , XBRZ4XMT},
	{uL("xBRZ 5x MT")           , uL("xbrz5xmt")    , XBRZ5XMT},
	{uL("xBRZ 6x MT")           , uL("xbrz6xmt")    , XBRZ6XMT},
};
static const _opt opt_ntsc[] = {
	{uL("Composite"), uL("composite"), COMPOSITE},
	{uL("S-Video")  , uL("svideo")   , SVIDEO},
	{uL("RGB")      , uL("rgb")      , RGBMODE}
};
static const _opt opt_shader[] = {
	{uL("no shader")            , uL("none")        , NO_SHADER},
	{uL("CRT Dotmask")          , uL("crtdotmask")  , SHADER_CRTDOTMASK},
	{uL("CRT Scanlines")        , uL("crtscanlines"), SHADER_CRTSCANLINES},
	{uL("CRT With Curve")       , uL("crtcurve")    , SHADER_CRTWITHCURVE},
	{uL("Emboss")               , uL("emboss")      , SHADER_EMBOSS},
	{uL("Noise")                , uL("noise")       , SHADER_NOISE},
	{uL("NTSC 2Phase Composite"), uL("ntsc2phcomp") , SHADER_NTSC2PHASECOMPOSITE},
	{uL("Old TV")               , uL("oldtv")       , SHADER_OLDTV},
	{uL("Extern")               , uL("file")        , SHADER_FILE}
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
static const _opt opt_cheat_mode[] = {
	{NULL, uL("disabled"),   NOCHEAT_MODE},
	{NULL, uL("gamegenie"),  GAMEGENIE_MODE},
	{NULL, uL("cheatslist"), CHEATSLIST_MODE}
};
static const _opt opt_rewind[] = {
	{NULL, uL("disabled"), RWND_0_MINUTES},
	{NULL, uL("2"), RWND_2_MINUTES},
	{NULL, uL("5"), RWND_5_MINUTES},
	{NULL, uL("15"), RWND_15_MINUTES},
	{NULL, uL("30"), RWND_30_MINUTES},
	{NULL, uL("60"), RWND_60_MINUTES},
	{NULL, uL("unlimited"), RWND_UNLIMITED_MINUTES}
};
static const _opt opt_languages[] = {
	{NULL, uL("english"), LNG_ENGLISH},
	{NULL, uL("italian"), LNG_ITALIAN},
	{NULL, uL("portuguese"), LNG_PORTUGUESEBR},
	{NULL, uL("russian"), LNG_RUSSIAN},
	{NULL, uL("spanish"), LNG_SPANISH},
	{NULL, uL("hungarian"), LNG_HUNGARIAN},
	{NULL, uL("turkish"), LNG_TURKISH}
};
static const _opt opt_nsf_player_effect[] = {
	{NULL, uL("bars"),     NSF_EFFECT_BARS},
	{NULL, uL("raw"), NSF_EFFECT_RAW},
	{NULL, uL("raw full"), NSF_EFFECT_RAW_FULL},
	{NULL, uL("hanning"), NSF_EFFECT_HANNING},
	{NULL, uL("hannig full"), NSF_EFFECT_HANNING_FULL}
};
static const _opt opt_toolbar_area[] = {
	{NULL, uL("top"), TLB_TOP},
	{NULL, uL("left"), TLB_LEFT},
	{NULL, uL("bottom"), TLB_BOTTOM},
	{NULL, uL("right"), TLB_RIGHT}
};

static const _opt opt_slot_pgs[] = {
	{NULL, uL("0"), 0},
	{NULL, uL("1"), 1},
	{NULL, uL("2"), 2},
	{NULL, uL("3"), 3},
	{NULL, uL("4"), 4},
	{NULL, uL("5"), 5}
};

static const _opt opt_controller_mode[] = {
	{NULL, uL("nes"),        CTRL_MODE_NES},
	{NULL, uL("famicom"),    CTRL_MODE_FAMICOM},
	{NULL, uL("four score"), CTRL_MODE_FOUR_SCORE}
};
static const _opt opt_expansion[] = {
	{NULL, uL("standard"),         CTRL_STANDARD},
	{NULL, uL("zapper"),           CTRL_ZAPPER},
	{NULL, uL("arkanoid paddle"),  CTRL_ARKANOID_PADDLE},
	{NULL, uL("oeka kids tablet"), CTRL_OEKA_KIDS_TABLET}
};
static const _opt opt_controller[] = {
	{NULL, uL("disable"),    CTRL_DISABLED},
	{NULL, uL("standard"),   CTRL_STANDARD},
	{NULL, uL("zapper"),     CTRL_ZAPPER},
	{NULL, uL("snes mouse"), CTRL_SNES_MOUSE},
	{NULL, uL("arkanoid paddle"), CTRL_ARKANOID_PADDLE}
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
		uL("system"), uL("rewind minutes"), uL("15"),
		uL("# possible values: disabled, 2, 5, 15, 30, 60, unlimited"),
		uL("    --rewind-minutes      rewind minutes        : disabled, 2, 5, 15, 30, 60, unlimited"),
		{LENGTH(opt_rewind), opt_rewind}
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
		uL("system"), uL("game genie rom file"), NULL,
		uL("# possible values: [PATH/NAME]"),
		NULL,
		{0, NULL}
	},
	{
		uL("system"), uL("fds bios file"), NULL,
		uL("# possible values: [PATH/NAME]"),
		NULL,
		{0, NULL}
	},
	{
		uL("system"), uL("last cheats file"), NULL,
		uL("# possible values: [PATH/NAME]"),
		NULL,
		{0, NULL}
	},
	{
		uL("system"), uL("save settings on exit"), uL("no"),
		uL("# possible values: yes, no"),
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("video"), uL("size window"), uL("2x"),
		uL("# possible values: 1x, 2x, 3x, 4x, 5x, 6x"),
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
		uL("video"), uL("overscan black borders in window"), uL("off"),
		uL("# possible values: on, off"),
		uL("    --overscan-blk-brd    enable black borders  : on, off" NEWLINE)
		uL("                          in window mode"),
		{LENGTH(opt_off_on), opt_off_on}
	},
	{
		uL("video"), uL("overscan black borders in fullscreen"), uL("on"),
		uL("# possible values: on, off"),
		uL("    --overscan-blk-brd-f  enable black borders  : on, off" NEWLINE)
		uL("                          in fullscreen"),
		{LENGTH(opt_off_on), opt_off_on}
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
		uL("#                  hq4x, xbrz2x, xbrz3x, xbrz4x, xbrz5x, xbrz6x," NEWLINE)
		uL("#                  xbrz2xmt, xbrz3xmt, xbrz4xmt, xbrz5xmt, xbrz6xmt, ntsc"),
		uL("-i, --filter              filter to apply       : nofilter, scale2x, scale3x, scale4x, hq2x, hq3x," NEWLINE)
		uL("                                                  hq4x, xbrz2x, xbrz3x, xbrz4x, xbrz5x, xbrz6x," NEWLINE)
		uL("                                                  xbrz2xmt, xbrz3xmt, xbrz4xmt, xbrz5xmt, xbrz6xmt, ntsc"),
		{LENGTH(opt_filter), opt_filter}
	},
	{
		uL("video"), uL("ntsc filter format"), uL("composite"),
		uL("# possible values: composite, svideo, rgb"),
		uL("-n, --ntsc-format         format of ntsc filter : composite, svideo, rgb"),
		{LENGTH(opt_ntsc), opt_ntsc}
	},
	{
		uL("video"), uL("shader"), uL("none"),
		uL("# possible values: none, crtdotmask, crtscanlines, crtcurve, emboss, noise," NEWLINE)
		uL("#                  ntsc2phcomp, oldtv, file"),
		uL("    --shader              shader to apply       : none, crtdotmask, crtscanlines, crtcurve," NEWLINE)
		uL("                                                  emboss, noise, ntsc2phcomp, oldtv, file"),
		{LENGTH(opt_shader), opt_shader}
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
		uL("-p, --palette             type of palette       : pal, ntsc, sony, frbyuv, frbuns, mono, green, file"),
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
		uL("video"), uL("show fps"), uL("no"),
		uL("# possible values: yes, no"),
		NULL,
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
		uL("video"), uL("fullscreen in window"), uL("no"),
		uL("# possible values: yes, no"),
		uL("    --fullscreen-window   the next fullscreen   : yes, no"  NEWLINE)
		uL("                          will be performed by" NEWLINE)
		uL("                          maximizing the window"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("video"), uL("use integer scaliung in fullscreen"), uL("no"),
		uL("# possible values: yes, no"),
		uL("-r, --int-scl-fullscreen  use integer scaling   : yes, no"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("video"), uL("stretch in fullscreen"), uL("no"),
		uL("# possible values: yes, no"),
		uL("-t, --stretch-fullscreen  stretch image         : yes, no"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("video"), uL("screen rotation"), uL("0"),
		uL("# possible values: 0, 90, 180, 270"),
		uL("    --screen-rotation     degree scrn rotation  : 0, 90, 180, 270"),
		{LENGTH(opt_screen_rotation), opt_screen_rotation}
	},
	{
		uL("video"), uL("text rotation"), uL("no"),
		uL("# possible values: yes, no"),
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("audio"), uL("output device"), uL("default"),
#if defined(__linux__)
		uL("# possible values: default, plughw:[x,x]"),
		uL("    --audio-output-device                       : default, plughw:[x,x]"),
# else
		uL("# possible values: default, [DEVICEID]"),
		uL("    --audio-output-device                       : default, [DEVICEID]"),
#endif
		{0, NULL}
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
		uL("GUI"), uL("last open patch path"), NULL,
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
		uL("GUI"), uL("last position of settings"), uL("0,0"),
		uL("# possible values: [X],[Y]"),
		NULL,
		{0, NULL}
	},
	{
		uL("GUI"), uL("language"), uL("english"),
		uL("# possible values: english, italian, russian, spanish, portuguese, hungarian, turkish"),
		uL("    --language            GUI language          : english, italian, russian, spanish," NEWLINE)
		uL("                                                  portuguese, hungarian, turkish"),
		{LENGTH(opt_languages), opt_languages}
	},
	{
		uL("GUI"), uL("toolbar area"), uL("top"),
		uL("# possible values: top, bottom, left, right"),
		NULL,
		{LENGTH(opt_toolbar_area), opt_toolbar_area}
	},
	{
		uL("GUI"), uL("toolbar hidden"), uL("no"),
		uL("# possible values: yes, no"),
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
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
	},
	{
		uL("player"), uL("effect"), uL("bars"),
		uL("# possible values: bars, raw, raw full, hanning, hanning full"),
		NULL,
		{LENGTH(opt_nsf_player_effect), opt_nsf_player_effect}
	},
	{
		uL("player"), uL("enable playlist"), uL("yes"),
		uL("# possible values: yes, no"),
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("player"), uL("enable fadeout song"), uL("yes"),
		uL("# possible values: yes, no"),
		NULL,
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
	{uL("shortcuts"), uL("open"),                        uL("Alt+O,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("quit"),                        uL("Alt+Q,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("turn off"),                    uL("Alt+R,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("hard reset"),                  uL("F11,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("soft reset"),                  uL("F12,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("insert coin"),                 uL("8,NULL"),          NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("switch sides"),                uL("Alt+S,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("eject disk"),                  uL("Alt+E,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("start or stop WAV recording"), uL("Alt+V,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("video fullscreen"),            uL("Alt+Return,NULL"), NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("save screenshot"),             uL("Alt+X,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("save unalterd nes screen"),    uL("Alt+Z,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("pause"),                       uL("Pause,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("fast forward"),                uL("Tab,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("toggle gui in window"),        uL("Alt+G,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("mode pal"),                    uL("F6,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("mode ntsc"),                   uL("F7,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("mode dendy"),                  uL("F8,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("mode auto"),                   uL("F9,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("scale 1x"),                    uL("Alt+1,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("scale 2x"),                    uL("Alt+2,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("scale 3x"),                    uL("Alt+3,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("scale 4x"),                    uL("Alt+4,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("scale 5x"),                    uL("Alt+5,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("scale 6x"),                    uL("Alt+6,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("video interpolation"),         uL("0,NULL"),          NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("integer scaling fullscreen"),  uL("Alt+L,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("stretch fullscreen"),          uL("Alt+P,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("audio enable"),                uL("Alt+A,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("save settings"),               uL("Alt+W,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("save state"),                  uL("F1,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("load state"),                  uL("F4,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("increment state slot"),        uL("F3,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("decrement state slot"),        uL("F2,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("start or stop rewind mode"),   uL("Ctrl+Left,NULL"),  NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("rewind step backward"),        uL("Left,NULL"),       NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("rewind step forward"),         uL("Right,NULL"),      NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("rewind fast backward"),        uL("Down,NULL"),       NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("rewind fast forward"),         uL("Up,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("rewind play"),                 uL("Del,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("rewind pause"),                uL("PgDown,NULL"),     NULL, NULL, {0, NULL}},
#if defined (_WIN32)
	{uL("shortcuts"), uL("joystick GUID"),               uL("NULL"),            NULL, NULL, {0, NULL}},
#else
	{uL("shortcuts"), uL("joystick Id"),                 uL("NULL"),            NULL, NULL, {0, NULL}},
#endif
	{
		uL("expansion port"), uL("expansion port"), uL("standard"),
		uL("# possible values: standard, zapper, arkanoid paddle, oeka kids tablet"),
		NULL,
		{LENGTH(opt_expansion), opt_expansion}
	},
	{
		uL("port 1"), uL("controller 1"), uL("standard"),
		uL("# possible values: disable, standard, zapper, snes mouse, arkanoid paddle"),
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
	{uL("port 1"), uL("P1J TurboA"),  uL("JB3"),         NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1J TurboB"),  uL("JB2"),         NULL, NULL, {0, NULL}},
#if defined (_WIN32)
	{uL("port 1"), uL("P1J GUID"),    uL("NULL"),        NULL, NULL, {0, NULL}},
#else
	{uL("port 1"), uL("P1J Id"),      uL("JOYSTICKID1"), NULL, NULL, {0, NULL}},
#endif
	{uL("port 1"), uL("P1 TA Delay"), NULL,              NULL, NULL, {0, NULL}},
	{uL("port 1"), uL("P1 TB Delay"), NULL,              NULL, NULL, {0, NULL}},
	{
		uL("port 2"), uL("controller 2"), uL("disable"),
		uL("# possible values: disable, standard, zapper, snes mouse, arkanoid paddle"),
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
	{uL("port 2"), uL("P2J TurboA"),  uL("JB3"),         NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2J TurboB"),  uL("JB2"),         NULL, NULL, {0, NULL}},
#if defined (_WIN32)
	{uL("port 2"), uL("P2J GUID"),    uL("NULL"),        NULL, NULL, {0, NULL}},
#else
	{uL("port 2"), uL("P2J Id"),      uL("JOYSTICKID2"), NULL, NULL, {0, NULL}},
#endif
	{uL("port 2"), uL("P2 TA Delay"), NULL,              NULL, NULL, {0, NULL}},
	{uL("port 2"), uL("P2 TB Delay"), NULL,              NULL, NULL, {0, NULL}},
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
	{uL("port 3"), uL("P3J TurboA"),  uL("JB3"),         NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3J TurboB"),  uL("JB2"),         NULL, NULL, {0, NULL}},
#if defined (_WIN32)
	{uL("port 3"), uL("P3J GUID"),    uL("NULL"),        NULL, NULL, {0, NULL}},
#else
	{uL("port 3"), uL("P3J Id"),      uL("NULL"),        NULL, NULL, {0, NULL}},
#endif
	{uL("port 3"), uL("P3 TA Delay"), NULL,              NULL, NULL, {0, NULL}},
	{uL("port 3"), uL("P3 TB Delay"), NULL,              NULL, NULL, {0, NULL}},
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
	{uL("port 4"), uL("P4J TurboA"),  uL("JB3"),         NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4J TurboB"),  uL("JB2"),         NULL, NULL, {0, NULL}},
#if defined (_WIN32)
	{uL("port 4"), uL("P4J GUID"),    uL("NULL"),        NULL, NULL, {0, NULL}},
#else
	{uL("port 4"), uL("P4J Id"),      uL("NULL"),        NULL, NULL, {0, NULL}},
#endif
	{uL("port 4"), uL("P4 TA Delay"), NULL,              NULL, NULL, {0, NULL}},
	{uL("port 4"), uL("P4 TB Delay"), NULL,              NULL, NULL, {0, NULL}},
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
	},
	{
		uL("system"), uL("hide zapper cursor"), uL("no"),
		uL("# possible values: yes, no"),
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
	}
};

static const _list_settings list_settings[] = {
	{main_cfg, LENGTH(main_cfg)},
	{pgs_cfg, LENGTH(pgs_cfg)},
	{inp_cfg, LENGTH(inp_cfg)},
	{NULL, 0},
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
EXTERNC void settings_cpy_utchar_to_val(int index, uTCHAR *buffer);
EXTERNC void settings_val_to_oscan(int index, _overscan_borders *ob, const uTCHAR *buffer);

EXTERNC void *settings_inp_rd_sc(int index, int type);
EXTERNC void settings_inp_wr_sc(void *str, int index, int type);
EXTERNC void settings_inp_all_default(_config_input *config_input, _array_pointers_port *array);
EXTERNC void settings_inp_port_default(_port *port, int index, int mode);
EXTERNC void settings_inp_save(void);

EXTERNC void settings_pgs_parse(void);
EXTERNC void settings_pgs_save(void);

EXTERNC void settings_shp_parse(void);
EXTERNC void settings_shp_save(void);

#undef EXTERNC

#endif /* SETTINGS_H_ */
