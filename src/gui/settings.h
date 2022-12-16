/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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
#include "ppu.h"
#if defined (WITH_FFMPEG)
#include "recording.h"
#endif

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
	SET_NTSC_COMPOSITE_PARAM,
	SET_NTSC_SVIDEO_PARAM,
	SET_NTSC_RGB_PARAM,
	SET_SHADER,
	SET_FILE_SHADER,
	SET_PALETTE,
	SET_FILE_PALETTE,
	SET_SWAP_EMPHASIS_PAL,
	SET_VSYNC,
	SET_INTERPOLATION,
	SET_TEXT_ON_SCREEN,
	SET_SHOW_FPS,
	SET_SHOW_FRAMES_AND_LAGS,
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
#if defined (FULLSCREEN_RESFREQ)
	SET_ADAPTIVE_RRATE_FULLSCREEN,
	SET_RESOLUTION_FULLSCREEN,
#endif
	SET_HORIZONTAL_FLIP_SCREEN,
	SET_SCREEN_ROTATION,
	SET_INPUT_ROTATION,
	SET_TEXT_ROTATION,
	SET_AUDIO_OUTPUT_DEVICE,
	SET_AUDIO_BUFFER_FACTOR,
	SET_SAMPLERATE,
	SET_CHANNELS,
	SET_STEREO_DELAY,
	SET_REVERSE_BITS_DPCM,
	SET_SWAP_DUTY,
	SET_AUDIO,
	SET_GUI_OPEN_PATH,
	SET_GUI_OPEN_PATCH_PATH,
	SET_GUI_LAST_POSITION,
	SET_GUI_LAST_GEOMETRY_SETTINGS,
	SET_GUI_LAST_GEOMETRY_NES_KEYBOARD,
	SET_GUI_LAST_GEOMETRY_LOG,
	SET_GUI_LAST_GEOMETRY_HEADER_EDITOR,
	SET_GUI_LANGUAGE,
	SET_GUI_TOOLBAR_AREA,
	SET_GUI_TOOLBAR_HIDDEN,
#if defined (WITH_FFMPEG)
	SET_GUI_REC_LAST_TYPE,
	SET_GUI_REC_LAST_VIDEO_PATH,
#endif
	SET_GUI_REC_LAST_AUDIO_PATH,
	SET_GUI_MULTIPLE_INSTANCES,
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
	SET_UNLIMITED_SPRITES_AUTO,
	SET_PPU_ALIGNMENT,
	SET_NSF_PLAYER_EFFECT,
	SET_NSF_PLAYER_NSFE_PLAYLIST,
	SET_NSF_PLAYER_NSFE_FADEOUT,
	SET_FDS_DISK1SIDEA_AT_RESET,
	SET_FDS_SWITCH_SIDE_AUTOMATICALLY,
	SET_FDS_FAST_FORWARD,
#if defined (WITH_FFMPEG)
	SET_REC_AUDIO_FORMAT,
	SET_REC_VIDEO_FORMAT,
	SET_REC_QUALITY,
	SET_REC_OUTPUT_RESOLUTION,
	SET_REC_OUTPUT_CUSTOM_WIDTH,
	SET_REC_OUTPUT_CUSTOM_HEIGHT,
	SET_REC_USE_EMU_RESOLUTION,
	SET_REC_FOLLOW_ROTATION,
#endif
	SET_ONLYCMDLINE_HIDDEN_GUI
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
	SET_INP_SC_REC_AUDIO,
#if defined (WITH_FFMPEG)
	SET_INP_SC_REC_VIDEO,
#endif
	SET_INP_SC_FULLSCREEN,
	SET_INP_SC_SCREENSHOT,
	SET_INP_SC_SCREENSHOT_1X,
	SET_INP_SC_PAUSE,
	SET_INP_SC_TOGGLE_FAST_FORWARD,
	SET_INP_SC_HOLD_FAST_FORWARD,
	SET_INP_SC_TOGGLE_GUI_IN_WINDOW,
	SET_INP_SC_SHOUT_INTO_MIC,
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
	SET_INP_SC_TOGGLE_MENUBAR_IN_FULLSCREEN,
	SET_INP_SC_TOGGLE_CAPTURE_INPUT,
	SET_INP_SC_TOGGLE_NES_KEYBOARD,
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

	SET_INP_SC_JOYSTICK_GUID,

	SET_INP_EXPANSION_PORT,
	SET_INP_P1_CONTROLLER,
	SET_INP_P1_PAD_TYPE,
	SET_INP_P1J_GUID,
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
	SET_INP_P1_TURBOA_DELAY,
	SET_INP_P1_TURBOB_DELAY,

	SET_INP_P2_CONTROLLER,
	SET_INP_P2_PAD_TYPE,
	SET_INP_P2J_GUID,
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
	SET_INP_P2_TURBOA_DELAY,
	SET_INP_P2_TURBOB_DELAY,

	SET_INP_P3_CONTROLLER,
	SET_INP_P3_PAD_TYPE,
	SET_INP_P3J_GUID,
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
	SET_INP_P3_TURBOA_DELAY,
	SET_INP_P3_TURBOB_DELAY,

	SET_INP_P4_CONTROLLER,
	SET_INP_P4_PAD_TYPE,
	SET_INP_P4J_GUID,
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
	SET_INP_P4_TURBOA_DELAY,
	SET_INP_P4_TURBOB_DELAY,

	SET_INP_CONTROLLER_MODE,
	SET_INP_LEFTRIGHT,
	SET_INP_HIDE_ZAPPER_CURSOR,

	SET_INP_VK_SIZE,

	SET_INP_FBKB_0,
	SET_INP_FBKB_1,
	SET_INP_FBKB_2,
	SET_INP_FBKB_3,
	SET_INP_FBKB_4,
	SET_INP_FBKB_5,
	SET_INP_FBKB_6,
	SET_INP_FBKB_7,
	SET_INP_FBKB_8,
	SET_INP_FBKB_9,
	SET_INP_FBKB_10,
	SET_INP_FBKB_11,
	SET_INP_FBKB_12,
	SET_INP_FBKB_13,
	SET_INP_FBKB_14,
	SET_INP_FBKB_15,
	SET_INP_FBKB_16,
	SET_INP_FBKB_17,
	SET_INP_FBKB_18,
	SET_INP_FBKB_19,
	SET_INP_FBKB_20,
	SET_INP_FBKB_21,
	SET_INP_FBKB_22,
	SET_INP_FBKB_23,
	SET_INP_FBKB_24,
	SET_INP_FBKB_25,
	SET_INP_FBKB_26,
	SET_INP_FBKB_27,
	SET_INP_FBKB_28,
	SET_INP_FBKB_29,
	SET_INP_FBKB_30,
	SET_INP_FBKB_31,
	SET_INP_FBKB_32,
	SET_INP_FBKB_33,
	SET_INP_FBKB_34,
	SET_INP_FBKB_35,
	SET_INP_FBKB_36,
	SET_INP_FBKB_37,
	SET_INP_FBKB_38,
	SET_INP_FBKB_39,
	SET_INP_FBKB_40,
	SET_INP_FBKB_41,
	SET_INP_FBKB_42,
	SET_INP_FBKB_43,
	SET_INP_FBKB_44,
	SET_INP_FBKB_45,
	SET_INP_FBKB_46,
	SET_INP_FBKB_47,
	SET_INP_FBKB_48,
	SET_INP_FBKB_49,
	SET_INP_FBKB_50,
	SET_INP_FBKB_51,
	SET_INP_FBKB_52,
	SET_INP_FBKB_53,
	SET_INP_FBKB_54,
	SET_INP_FBKB_55,
	SET_INP_FBKB_56,
	SET_INP_FBKB_57,
	SET_INP_FBKB_58,
	SET_INP_FBKB_59,
	SET_INP_FBKB_60,
	SET_INP_FBKB_61,
	SET_INP_FBKB_62,
	SET_INP_FBKB_63,
	SET_INP_FBKB_64,
	SET_INP_FBKB_65,
	SET_INP_FBKB_66,
	SET_INP_FBKB_67,
	SET_INP_FBKB_68,
	SET_INP_FBKB_69,
	SET_INP_FBKB_70,
	SET_INP_FBKB_71,
	SET_INP_FBKB_END = SET_INP_FBKB_71,

	SET_INP_SBKB_EXTENDED_MODE,
	SET_INP_SBKB_0,
	SET_INP_SBKB_1,
	SET_INP_SBKB_2,
	SET_INP_SBKB_3,
	SET_INP_SBKB_4,
	SET_INP_SBKB_5,
	SET_INP_SBKB_6,
	SET_INP_SBKB_7,
	SET_INP_SBKB_8,
	SET_INP_SBKB_9,
	SET_INP_SBKB_10,
	SET_INP_SBKB_11,
	SET_INP_SBKB_12,
	SET_INP_SBKB_13,
	SET_INP_SBKB_14,
	SET_INP_SBKB_15,
	SET_INP_SBKB_16,
	SET_INP_SBKB_17,
	SET_INP_SBKB_18,
	SET_INP_SBKB_19,
	SET_INP_SBKB_20,
	SET_INP_SBKB_21,
	SET_INP_SBKB_22,
	SET_INP_SBKB_23,
	SET_INP_SBKB_24,
	SET_INP_SBKB_25,
	SET_INP_SBKB_26,
	SET_INP_SBKB_27,
	SET_INP_SBKB_28,
	SET_INP_SBKB_29,
	SET_INP_SBKB_30,
	SET_INP_SBKB_31,
	SET_INP_SBKB_32,
	SET_INP_SBKB_33,
	SET_INP_SBKB_34,
	SET_INP_SBKB_35,
	SET_INP_SBKB_36,
	SET_INP_SBKB_37,
	SET_INP_SBKB_38,
	SET_INP_SBKB_39,
	SET_INP_SBKB_40,
	SET_INP_SBKB_41,
	SET_INP_SBKB_42,
	SET_INP_SBKB_43,
	SET_INP_SBKB_44,
	SET_INP_SBKB_45,
	SET_INP_SBKB_46,
	SET_INP_SBKB_47,
	SET_INP_SBKB_48,
	SET_INP_SBKB_49,
	SET_INP_SBKB_50,
	SET_INP_SBKB_51,
	SET_INP_SBKB_52,
	SET_INP_SBKB_53,
	SET_INP_SBKB_54,
	SET_INP_SBKB_55,
	SET_INP_SBKB_56,
	SET_INP_SBKB_57,
	SET_INP_SBKB_58,
	SET_INP_SBKB_59,
	SET_INP_SBKB_60,
	SET_INP_SBKB_61,
	SET_INP_SBKB_62,
	SET_INP_SBKB_63,
	SET_INP_SBKB_64,
	SET_INP_SBKB_65,
	SET_INP_SBKB_66,
	SET_INP_SBKB_67,
	SET_INP_SBKB_68,
	SET_INP_SBKB_69,
	SET_INP_SBKB_70,
	SET_INP_SBKB_71,
	SET_INP_SBKB_72,
	SET_INP_SBKB_73,
	SET_INP_SBKB_74,
	SET_INP_SBKB_75,
	SET_INP_SBKB_76,
	SET_INP_SBKB_77,
	SET_INP_SBKB_78,
	SET_INP_SBKB_79,
	SET_INP_SBKB_80,
	SET_INP_SBKB_81,
	SET_INP_SBKB_82,
	SET_INP_SBKB_83,
	SET_INP_SBKB_84,
	SET_INP_SBKB_85,
	SET_INP_SBKB_86,
	SET_INP_SBKB_87,
	SET_INP_SBKB_88,
	SET_INP_SBKB_89,
	SET_INP_SBKB_90,
	SET_INP_SBKB_91,
	SET_INP_SBKB_92,
	SET_INP_SBKB_93,
	SET_INP_SBKB_94,
	SET_INP_SBKB_95,
	SET_INP_SBKB_96,
	SET_INP_SBKB_97,
	SET_INP_SBKB_98,
	SET_INP_SBKB_99,
	SET_INP_SBKB_END = SET_INP_SBKB_99
};
enum jsc_element {
	SET_JSC_PAD_A,
	SET_JSC_PAD_B,
	SET_JSC_PAD_SELECT,
	SET_JSC_PAD_START,
	SET_JSC_PAD_UP,
	SET_JSC_PAD_DOWN,
	SET_JSC_PAD_LEFT,
	SET_JSC_PAD_RIGHT,
	SET_JSC_PAD_TURBOA,
	SET_JSC_PAD_TURBOB,
	SET_JSC_DEADZONE,
	SET_JSC_BUTTONS_ENABLED,
	SET_JSC_AXES_ENABLED,
};

enum set_num_shortcut { SET_MAX_NUM_SC = SET_INP_SC_JOYSTICK_GUID - SET_INP_SC_OPEN};

enum list_settings_element {
	LSET_SET,
	LSET_PGS,
	LSET_INP,
	LSET_JSC,
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
	{NULL, uL("on") , TRUE}
};
static const _opt opt_mode[] = {
	{uL("Auto") , uL("auto") , AUTO},
	{uL("NTSC") , uL("ntsc") , NTSC},
	{uL("PAL")  , uL("pal")  , PAL},
	{uL("Dendy"), uL("dendy"), DENDY}
};
static const _opt opt_ff_velocity[] = {
	{NULL, uL("2x"), FF_2X},
	{NULL, uL("3x"), FF_3X},
	{NULL, uL("4x"), FF_4X},
	{NULL, uL("5x"), FF_5X}
};
static const _opt opt_screen_rotation[] = {
	{NULL, uL("0")  , ROTATE_0},
	{NULL, uL("90") , ROTATE_90},
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
	{uL("no filter")  , uL("none")      , NO_FILTER},
	{uL("Scale2X")    , uL("scale2x")   , SCALE2X},
	{uL("Scale3X")    , uL("scale3x")   , SCALE3X},
	{uL("Scale4X")    , uL("scale4x")   , SCALE4X},
	{uL("Hq2X")       , uL("hq2x")      , HQ2X},
	{uL("Hq3X")       , uL("hq3x")      , HQ3X},
	{uL("Hq4X")       , uL("hq4x")      , HQ4X},
	{uL("NTSC")       , uL("ntsc")      , NTSC_FILTER},
	{uL("xBRZ 2x")    , uL("xbrz2x")    , XBRZ2X},
	{uL("xBRZ 3x")    , uL("xbrz3x")    , XBRZ3X},
	{uL("xBRZ 4x")    , uL("xbrz4x")    , XBRZ4X},
	{uL("xBRZ 5x")    , uL("xbrz5x")    , XBRZ5X},
	{uL("xBRZ 6x")    , uL("xbrz6x")    , XBRZ6X},
	{uL("xBRZ 2x MT") , uL("xbrz2mtx")  , XBRZ2XMT},
	{uL("xBRZ 3x MT") , uL("xbrz3xmt")  , XBRZ3XMT},
	{uL("xBRZ 4x MT") , uL("xbrz4xmt")  , XBRZ4XMT},
	{uL("xBRZ 5x MT") , uL("xbrz5xmt")  , XBRZ5XMT},
	{uL("xBRZ 6x MT") , uL("xbrz6xmt")  , XBRZ6XMT},
	{uL("2xSaI")      , uL("2xsai")     , SCALE2XSAI},
	{uL("Super 2xSaI"), uL("super2xsai"), SUPER2XSAI},
	{uL("Super Eagle"), uL("supereagle"), SUPEREAGLE},
	{uL("TV2x")       , uL("tv2x")      , TV2X},
	{uL("TV3x")       , uL("tv3x")      , TV3X},
	{uL("TV4x")       , uL("tv4x")      , TV4X},
	{uL("Dot Matrix") , uL("dotmatrix") , DOTMATRIX},
	{uL("Pal TV1x")   , uL("paltv1x")   , PALTV1X},
	{uL("Pal TV2x")   , uL("paltv2x")   , PALTV2X},
	{uL("Pal TV3x")   , uL("paltv3x")   , PALTV3X},
	{uL("Pal TV4x")   , uL("paltv4x")   , PALTV4X}
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
	{uL("Firebrandx YUV")   , uL("frbyuv"), PALETTE_FRBX_YUV},
	{uL("Raw")              , uL("raw")   , PALETTE_RAW}
};
static const _opt opt_audio_buffer_factor[] = {
	{NULL, uL("0") , 0},
	{NULL, uL("1") , 1},
	{NULL, uL("2") , 2},
	{NULL, uL("3") , 3},
	{NULL, uL("4") , 4},
	{NULL, uL("5") , 5},
	{NULL, uL("6") , 6},
	{NULL, uL("7") , 7},
	{NULL, uL("8") , 8},
	{NULL, uL("9") , 9},
	{NULL, uL("10"), 10},
	{NULL, uL("11"), 11},
	{NULL, uL("12"), 12},
	{NULL, uL("13"), 13},
	{NULL, uL("14"), 14},
	{NULL, uL("15"), 15}
};
static const _opt opt_samplerate[] = {
	{NULL, uL("192000"), S192000},
	{NULL, uL("96000") , S96000},
	{NULL, uL("48000") , S48000},
	{NULL, uL("44100") , S44100},
	{NULL, uL("22050") , S22050},
	{NULL, uL("11025") , S11025}
};
static const _opt opt_channels[] = {
	{NULL, uL("mono")   , CH_MONO},
	{NULL, uL("delay")  , CH_STEREO_DELAY},
	{NULL, uL("panning"), CH_STEREO_PANNING}
};
static const _opt opt_cheat_mode[] = {
	{NULL, uL("disabled")  , NOCHEAT_MODE},
	{NULL, uL("gamegenie") , GAMEGENIE_MODE},
	{NULL, uL("cheatslist"), CHEATSLIST_MODE}
};
static const _opt opt_rewind[] = {
	{NULL, uL("disabled") , RWND_0_MINUTES},
	{NULL, uL("2")        , RWND_2_MINUTES},
	{NULL, uL("5")        , RWND_5_MINUTES},
	{NULL, uL("15")       , RWND_15_MINUTES},
	{NULL, uL("30")       , RWND_30_MINUTES},
	{NULL, uL("60")       , RWND_60_MINUTES},
	{NULL, uL("unlimited"), RWND_UNLIMITED_MINUTES}
};
static const _opt opt_languages[] = {
	{NULL, uL("english")   , LNG_ENGLISH},
	{NULL, uL("italian")   , LNG_ITALIAN},
	{NULL, uL("russian")   , LNG_RUSSIAN},
	{NULL, uL("spanish")   , LNG_SPANISH},
	{NULL, uL("hungarian") , LNG_HUNGARIAN},
	{NULL, uL("turkish")   , LNG_TURKISH},
	{NULL, uL("portuguese"), LNG_PORTUGUESEBR},
	{NULL, uL("chinese simplified"), LNG_CHINESE_SIMPLIFIED}
};
static const _opt opt_nsf_player_effect[] = {
	{NULL, uL("bars")       , NSF_EFFECT_BARS},
	{NULL, uL("bars mixed") , NSF_EFFECT_BARS_MIXED},
	{NULL, uL("raw")        , NSF_EFFECT_RAW},
	{NULL, uL("raw full")   , NSF_EFFECT_RAW_FULL},
	{NULL, uL("hanning")    , NSF_EFFECT_HANNING},
	{NULL, uL("hannig full"), NSF_EFFECT_HANNING_FULL}
};
#if defined (WITH_FFMPEG)
static const _opt opt_recording_format_type[] = {
	{NULL, uL("video")  , REC_FORMAT_VIDEO},
	{NULL, uL("audio")  , REC_FORMAT_AUDIO}
};
static const _opt opt_recording_audio_format[] = {
	{NULL, uL("mp3")  , REC_FORMAT_AUDIO_MP3},
	{NULL, uL("aac")  , REC_FORMAT_AUDIO_AAC},
	{NULL, uL("flac") , REC_FORMAT_AUDIO_FLAC},
	{NULL, uL("ogg")  , REC_FORMAT_AUDIO_OGG},
	{NULL, uL("wav")  , REC_FORMAT_AUDIO_WAV},
	{NULL, uL("opus") , REC_FORMAT_AUDIO_OPUS}
};
static const _opt opt_recording_video_format[] = {
	{NULL, uL("mpeg1"), REC_FORMAT_VIDEO_MPG_MPEG1},
	{NULL, uL("mpeg2"), REC_FORMAT_VIDEO_MPG_MPEG2},
	{NULL, uL("mpeg4"), REC_FORMAT_VIDEO_MP4_MPEG4},
	{NULL, uL("h264") , REC_FORMAT_VIDEO_MP4_H264},
	{NULL, uL("hevc") , REC_FORMAT_VIDEO_MKV_HEVC},
	{NULL, uL("webm") , REC_FORMAT_VIDEO_WEB_WEBM},
	{NULL, uL("wmv")  , REC_FORMAT_VIDEO_AVI_WMV},
	{NULL, uL("ffv")  , REC_FORMAT_VIDEO_AVI_FFV},
	{NULL, uL("raw")  , REC_FORMAT_VIDEO_AVI_RAW}
};
static const _opt opt_recording_quality[] = {
	{NULL, uL("low")   , REC_QUALITY_LOW},
	{NULL, uL("medium"), REC_QUALITY_MEDIUM},
	{NULL, uL("high")  , REC_QUALITY_HIGH}
};
static const _opt opt_recording_output_resolution[] = {
	{NULL, uL("custom")   , REC_RES_CUSTOM},
	{NULL, uL("256x240")  , REC_RES_256x240},
	{NULL, uL("292x240")  , REC_RES_292x240},
	{NULL, uL("320x240")  , REC_RES_320x240},
	{NULL, uL("354x240")  , REC_RES_354x240},
	{NULL, uL("512x480")  , REC_RES_512x480},
	{NULL, uL("584x480")  , REC_RES_584x480},
	{NULL, uL("640x480")  , REC_RES_640x480},
	{NULL, uL("708x480")  , REC_RES_708x480},
	{NULL, uL("768x720")  , REC_RES_768x720},
	{NULL, uL("876x720")  , REC_RES_876x720},
	{NULL, uL("960x720")  , REC_RES_960x720},
	{NULL, uL("1064x720") , REC_RES_1064x720},
	{NULL, uL("1024x960") , REC_RES_1024x960},
	{NULL, uL("1170x960") , REC_RES_1170x960},
	{NULL, uL("1280x960") , REC_RES_1280x960},
	{NULL, uL("1418x960") , REC_RES_1418x960},
	{NULL, uL("1280x720") , REC_RES_1280x720},
	{NULL, uL("1920x1080"), REC_RES_1920x1080}
};
#endif
static const _opt opt_cpuppu_aligment[] = {
	{NULL, uL("default")           , PPU_ALIGMENT_DEFAULT},
	{NULL, uL("randomize")         , PPU_ALIGMENT_RANDOMIZE},
	{NULL, uL("increment at reset"), PPU_ALIGMENT_INC_AT_RESET}
};
static const _opt opt_toolbar_area[] = {
	{NULL, uL("top")   , TLB_TOP},
	{NULL, uL("left")  , TLB_LEFT},
	{NULL, uL("bottom"), TLB_BOTTOM},
	{NULL, uL("right") , TLB_RIGHT}
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
	{NULL, uL("nes")       , CTRL_MODE_NES},
	{NULL, uL("famicom")   , CTRL_MODE_FAMICOM},
	{NULL, uL("four score"), CTRL_MODE_FOUR_SCORE}
};
static const _opt opt_expansion[] = {
	{NULL, uL("standard")             , CTRL_STANDARD},
	{NULL, uL("zapper")               , CTRL_ZAPPER},
	{NULL, uL("arkanoid paddle")      , CTRL_ARKANOID_PADDLE},
	{NULL, uL("oeka kids tablet")     , CTRL_OEKA_KIDS_TABLET},
	{NULL, uL("family basic keyboard"), CTRL_FAMILY_BASIC_KEYBOARD},
	{NULL, uL("subor keyboard")       , CTRL_SUBOR_KEYBOARD}
};
static const _opt opt_controller[] = {
	{NULL, uL("disable")        , CTRL_DISABLED},
	{NULL, uL("standard")       , CTRL_STANDARD},
	{NULL, uL("zapper")         , CTRL_ZAPPER},
	{NULL, uL("snes mouse")     , CTRL_SNES_MOUSE},
	{NULL, uL("arkanoid paddle"), CTRL_ARKANOID_PADDLE}
};
static const _opt opt_pad_type[] = {
	{NULL, uL("auto")    , CTRL_PAD_AUTO},
	{NULL, uL("original"), CTRL_PAD_ORIGINAL},
	{NULL, uL("3rdparty"), CTRL_PAD_3RD_PARTY}
};
static const _opt opt_vk_size[] = {
	{NULL, uL("1.0x")    , VK_SIZE_10X},
	{NULL, uL("1.5x")    , VK_SIZE_15X},
	{NULL, uL("2.0x")    , VK_SIZE_20X},
	{NULL, uL("2.5x")    , VK_SIZE_25X}
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
		uL("system"), uL("last cheats path"), NULL,
		uL("# possible values: [PATH]"),
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
		uL("#                  xbrz2xmt, xbrz3xmt, xbrz4xmt, xbrz5xmt, xbrz6xmt," NEWLINE)
		uL("#                  ntsc, 2xsai, super2xsai, supereagle, tv2x, tv3x," NEWLINE)
		uL("#                  tv4x, dotmatrix, paltv1x, paltv2x, paltv3x, paltv4x"),
		uL("-i, --filter              filter to apply       : nofilter, scale2x, scale3x, scale4x, hq2x, hq3x," NEWLINE)
		uL("                                                  hq4x, xbrz2x, xbrz3x, xbrz4x, xbrz5x, xbrz6x," NEWLINE)
		uL("                                                  xbrz2xmt, xbrz3xmt, xbrz4xmt, xbrz5xmt, xbrz6xmt," NEWLINE)
		uL("                                                  ntsc, 2xsai, super2xsai, supereagle, tv2x,tv3x," NEWLINE)
		uL("                                                  tv4x, dotmatrix, paltv1x, paltv2x, paltv3x," NEWLINE)
		uL("                                                  paltv4x"),
		{LENGTH(opt_filter), opt_filter}
	},
	{
		uL("video"), uL("ntsc filter format"), uL("composite"),
		uL("# possible values: composite, svideo, rgb"),
		uL("-n, --ntsc-format         format of ntsc filter : composite, svideo, rgb"),
		{LENGTH(opt_ntsc), opt_ntsc}
	},
	{
		uL("video"), uL("ntsc filter parameters composite"), uL("0,0,0,0,0,0,0,0,0,0,0,1,88"),
		uL("# possible values: [hue       : -100/100],[saturation  : -100/100],[contrast      : -100/100]," NEWLINE)
		uL("#                  [brightness: -100/100],[sharpness   : -100/100],[gamma         : -100/100]," NEWLINE)
		uL("#                  [resolution: -100/100],[artifacts   : -100/100],[fringing      : -100/100]," NEWLINE)
		uL("#                  [bleed     : -100/100],[merge fields:      0/1],[vertical blend:      0/1]," NEWLINE)
		uL("#                  [scanline  :    0/100]"),
		NULL,
		{0, NULL}
	},
	{
		uL("video"), uL("ntsc filter parameters svideo"), uL("0,0,0,0,20,0,20,-20,-20,0,0,1,88"),
		NULL,
		NULL,
		{0, NULL}
	},
	{
		uL("video"), uL("ntsc filter parameters rgb"), uL("0,0,0,0,20,0,70,-20,-20,-100,0,1,88"),
		NULL,
		NULL,
		{0, NULL}
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
		uL("-p, --palette             type of palette       : pal, ntsc, sony, frbyuv, frbuns, mono, " NEWLINE)
		uL("                                                : green, raw, file"),
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
		uL("video"), uL("show frames and lags counters"), uL("no"),
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
		uL("    --fullscreen-window   the next fullscreen   : yes, no" NEWLINE)
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
#if defined (FULLSCREEN_RESFREQ)
	{
		uL("video"), uL("adaptive refresh rate in fullscreen"), uL("no"),
		uL("# possible values: yes, no"),
		uL("    --adaptive-rrate      try to adapte the     : yes, no" NEWLINE)
		uL("                          refresh rate to the rom"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("video"), uL("preferred fullscreen resolution"), NULL,
		uL("# possible values: automatic, [width]x[height]"),
		uL("    --fullscreen-res      fullscreen resolution : automatic, [width]x[height]" NEWLINE)
		uL("                          if supported by the monitor"),
		{0, NULL}
	},
#endif
	{
		uL("video"), uL("horizontal flip screen"), uL("no"),
		uL("# possible values: yes, no"),
		uL("    --hflip-screen         horizontal flip      : yes, no"),
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("video"), uL("screen rotation"), uL("0"),
		uL("# possible values: 0, 90, 180, 270"),
		uL("    --screen-rotation     degree scrn rotation  : 0, 90, 180, 270"),
		{LENGTH(opt_screen_rotation), opt_screen_rotation}
	},
	{
		uL("video"), uL("input rotation"), uL("yes"),
		uL("# possible values: yes, no"),
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
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
		uL("# possible values: 192000, 96000, 48000, 44100, 22050, 11025"),
		uL("-l, --samplerate          sample rate           : 192000, 96000, 48000, 44100, 22050, 11025"),
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
		uL("audio"), uL("reverse bits of DPCM"), uL("no"),
		uL("# possible values: yes, no"),
		uL("    --reverse-bits-dpcm   reverse bits of dpcm  : yes, no"),
		{LENGTH(opt_no_yes), opt_no_yes}
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
		uL("GUI"), uL("last position of window"), uL("80, 80"),
		uL("# possible values: [X],[Y]"),
		NULL,
		{0, NULL}
	},
	{
		uL("GUI"), uL("last geometry of settings"), uL("80, 80, 0, 0"),
		uL("# possible values: [X],[Y],[W],[H]"),
		NULL,
		{0, NULL}
	},
	{
		uL("GUI"), uL("last geometry of nes keyboard"), uL("80, 80, 0, 0"),
		uL("# possible values: [X],[Y],[W],[H]"),
		NULL,
		{0, NULL}
	},
	{
			uL("GUI"), uL("last geometry of log"), uL("80, 80, 0, 0"),
			uL("# possible values: [X],[Y],[W],[H]"),
			NULL,
			{0, NULL}
	},
	{
			uL("GUI"), uL("last geometry of head editor"), uL("80, 80, 0, 0"),
			uL("# possible values: [X],[Y],[W],[H]"),
			NULL,
			{0, NULL}
	},
	{
		uL("GUI"), uL("language"), uL("english"),
		uL("# possible values: english, italian, russian, spanish, hungarian, turkish, portuguese," NEWLINE)
		uL("#                  chinese simplified"),
		uL("    --language            GUI language          : english, italian, russian, spanish," NEWLINE)
		uL("                                                  hungarian, turkish, portuguese,"  NEWLINE)
		uL("                                                  chinese simplified"),
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
#if defined (WITH_FFMPEG)
	{
		uL("GUI"), uL("last recording type"), uL("audio"),
		uL("# possible values: video, audio"),
		NULL,
		{LENGTH(opt_recording_format_type), opt_recording_format_type}
	},
	{
		uL("GUI"), uL("last video recording path"), NULL,
		uL("# possible values: [PATH]"),
		NULL,
		{0, NULL}
	},
#endif
	{
		uL("GUI"), uL("last audio recording path"), NULL,
		uL("# possible values: [PATH]"),
		NULL,
		{0, NULL}
	},
	{
		uL("GUI"), uL("allow multiple instances of the emulator"), uL("no"),
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
		uL("ppu"), uL("unlimited sprites auto no glitch"), uL("yes"),
		uL("# possible values: yes, no"),
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("ppu"), uL("cpu ppu alignment"), uL("default"),
		uL("# possible values: default, randomize, increment at reset"),
		NULL,
		{LENGTH(opt_cpuppu_aligment), opt_cpuppu_aligment}
	},
	{
		uL("player"), uL("effect"), uL("bars"),
		uL("# possible values: bars, bars mixed, raw, raw full, hanning, hanning full"),
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
	},
	{
		uL("fds"), uL("disk 1 sida A at reset"), uL("yes"),
		uL("# possible values: yes, no"),
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("fds"), uL("switch the side automatically"), uL("yes"),
		uL("# possible values: yes, no"),
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("fds"), uL("fast forward on bios and switch side"), uL("yes"),
		uL("# possible values: yes, no"),
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
	},
#if defined (WITH_FFMPEG)
	{
		uL("recording"), uL("audio format"), uL("wav"),
		uL("# possible values: wav, mp3, aac, flac, ogg, opus"),
		NULL,
		{LENGTH(opt_recording_audio_format), opt_recording_audio_format}
	},
	{
		uL("recording"), uL("video format"), uL("mpeg1"),
		uL("# possible values: mpeg1, mpeg2, mpeg4, h264, hevc, webm, wvm, ffv, raw"),
		NULL,
		{LENGTH(opt_recording_video_format), opt_recording_video_format}
	},
	{
		uL("recording"), uL("quality"), uL("medium"),
		uL("# possible values: low, medium, high"),
		NULL,
		{LENGTH(opt_recording_quality), opt_recording_quality}
	},
	{
		uL("recording"), uL("output resolution"), uL("512x480"),
		uL("# possible values: custom, 256x240, 292x240, 320x240, 354x240, 512x480, 584x480, 640x480," NEWLINE)
		uL("#                  708x480, 768x720, 876x720, 1064x720, 1024x960, 1170x960, 1280x960, 1418x960," NEWLINE)
		uL("#                  1280x720, 1920x1080"),
		NULL,
		{LENGTH(opt_recording_output_resolution), opt_recording_output_resolution}
	},
	{
		uL("recording"), uL("output custom width"), uL("512"),
		uL("# possible values: [256 - 2048]"),
		NULL,
		{0, NULL}
	},
	{
		uL("recording"), uL("output custom height"), uL("480"),
		uL("# possible values: [240 - 2048]"),
		NULL,
		{0, NULL}
	},
	{
		uL("recording"), uL("use emu resolution"), uL("no"),
		uL("# possible values: yes, no"),
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	{
		uL("recording"), uL("follow rotation"), uL("yes"),
		uL("# possible values: yes, no"),
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
	},
#endif
	// opzioni da sola riga di comando
	{
		NULL, NULL, NULL,
		NULL,
		uL("    --hidden-gui          start with hidden GUI"),
		{0, NULL}
	},
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
	{uL("shortcuts"), uL("open"),                          uL("Alt+O,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("quit"),                          uL("Alt+Q,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("turn off"),                      uL("Alt+R,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("hard reset"),                    uL("F11,NULL"),          NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("soft reset"),                    uL("F12,NULL"),          NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("insert coin"),                   uL("8,NULL"),            NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("switch sides"),                  uL("Alt+S,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("eject disk"),                    uL("Alt+E,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("start or stop audio recording"), uL("CTRL+A,NULL"),       NULL, NULL, {0, NULL}},
#if defined (WITH_FFMPEG)
	{uL("shortcuts"), uL("start or stop video recording"), uL("CTRL+V,NULL"),       NULL, NULL, {0, NULL}},
#endif
	{uL("shortcuts"), uL("video fullscreen"),              uL("Alt+Return,NULL"),   NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("save screenshot"),               uL("Alt+X,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("save unalterd nes screen"),      uL("Alt+Z,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("pause"),                         uL("Pause,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("fast forward"),                  uL("Tab,NULL"),          NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("hold fast forward"),             uL("NULL,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("toggle gui in window"),          uL("Alt+G,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("shout into microphone"),         uL("M,NULL"),            NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("mode pal"),                      uL("F6,NULL"),           NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("mode ntsc"),                     uL("F7,NULL"),           NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("mode dendy"),                    uL("F8,NULL"),           NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("mode auto"),                     uL("F9,NULL"),           NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("scale 1x"),                      uL("Alt+1,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("scale 2x"),                      uL("Alt+2,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("scale 3x"),                      uL("Alt+3,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("scale 4x"),                      uL("Alt+4,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("scale 5x"),                      uL("Alt+5,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("scale 6x"),                      uL("Alt+6,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("video interpolation"),           uL("0,NULL"),            NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("integer scaling fullscreen"),    uL("Alt+L,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("stretch fullscreen"),            uL("Alt+P,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("toggle menubar in fullscreen"),  uL("Alt+M,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("toggle capture input"),          uL("ScrollLock,NULL"),   NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("toggle virtual keyboard"),       uL("CTRL+X,NULL"),       NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("audio enable"),                  uL("Alt+A,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("save settings"),                 uL("Alt+W,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("save state"),                    uL("F1,NULL"),           NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("load state"),                    uL("F4,NULL"),           NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("increment state slot"),          uL("F3,NULL"),           NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("decrement state slot"),          uL("F2,NULL"),           NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("start or stop rewind mode"),     uL("Ctrl+Left,NULL"),    NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("rewind step backward"),          uL("Left,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("rewind step forward"),           uL("Right,NULL"),        NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("rewind fast backward"),          uL("Down,NULL"),         NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("rewind fast forward"),           uL("Up,NULL"),           NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("rewind play"),                   uL("Del,NULL"),          NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("rewind pause"),                  uL("PgDown,NULL"),       NULL, NULL, {0, NULL}},
	{uL("shortcuts"), uL("joystick GUID"),                 uL("NULL"),              NULL, NULL, {0, NULL}},
	{
		uL("expansion port"), uL("expansion port"), uL("standard"),
		uL("# possible values: standard, zapper, arkanoid paddle, oeka kids tablet,") NEWLINE
		uL("#                  family basic keyboard, subor keyboard"),
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
	{uL("port 1"), uL("P1J GUID"),    uL("NULL"),        uL("# player 1 joystick"), NULL, {0, NULL}},
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
	{uL("port 1"), uL("P1 TA Delay"), NULL,              uL("# player 1 turbo delays"), NULL, {0, NULL}},
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
	{uL("port 2"), uL("P2J GUID"),    uL("NULL"),        uL("# player 2 joystick"), NULL, {0, NULL}},
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
	{uL("port 2"), uL("P2 TA Delay"), NULL,              uL("# player 2 turbo delays"), NULL, {0, NULL}},
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
	{uL("port 3"), uL("P3J GUID"),    uL("NULL"),        uL("# player 3 joystick"), NULL, {0, NULL}},
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
	{uL("port 3"), uL("P3 TA Delay"), NULL,              uL("# player 3 turbo delays"), NULL, {0, NULL}},
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
	{uL("port 4"), uL("P4J GUID"),    uL("NULL"),        uL("# player 4 joystick"), NULL, {0, NULL}},
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
	{uL("port 4"), uL("P4 TA Delay"), NULL,              uL("# player 4 turbo delays"), NULL, {0, NULL}},
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
	},
	{
		uL("virtual keyboard"), uL("size"), uL("1.0x"),
		uL("# possible values: 1.0x, 1.5x, 2.0x, 2.5x"),
		NULL,
		{LENGTH(opt_vk_size), opt_vk_size}
	},
	// Family Keyboard
	// Row 0 - Column 1
	{uL("family basic keyboard"), uL("FBKB F8"),           uL("NSCODE_F8"),       NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Return"),       uL("NSCODE_Return"),   NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB BracketRight"), uL("NSCODE_Bckslsh"),  NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB BracketLeft"),  uL("NSCODE_BrkRight"), NULL, NULL, {0, NULL}},
	// Row 0 - Column 1
	{uL("family basic keyboard"), uL("FBKB Kana"),         uL("NSCODE_CapsLck"),  NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB RShift"),       uL("NSCODE_RShift"),   NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Yen"),          uL("NSCODE_Home"),     NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Stop"),         uL("NSCODE_End"),      NULL, NULL, {0, NULL}},
	// Row 1 - Column 0
	{uL("family basic keyboard"), uL("FBKB F7"),           uL("NSCODE_F7"),       NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB At"),           uL("NSCODE_BrkLeft"),  NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Colon"),        uL("NSCODE_Apstrph"),  NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Semicolon"),    uL("NSCODE_Semicln"),  NULL, NULL, {0, NULL}},
	// Row 1 - Column 1
	{uL("family basic keyboard"), uL("FBKB OpenBox"),      uL("NSCODE_QtLeft"),   NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Slash"),        uL("NSCODE_Slash"),    NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Minus"),        uL("NSCODE_Minus"),    NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Circumflex"),   uL("NSCODE_Equal"),    NULL, NULL, {0, NULL}},
	// Row 2 - Column 0
	{uL("family basic keyboard"), uL("FBKB F6"),           uL("NSCODE_F6"),       NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB O"),            uL("NSCODE_O"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB L"),            uL("NSCODE_L"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB K"),            uL("NSCODE_K"),        NULL, NULL, {0, NULL}},
	// Row 2 - Column 1
	{uL("family basic keyboard"), uL("FBKB Period"),       uL("NSCODE_Period"),   NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Comma"),        uL("NSCODE_Comma"),    NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB P"),            uL("NSCODE_P"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB 0"),            uL("NSCODE_0"),        NULL, NULL, {0, NULL}},
	// Row 3 - Column 0
	{uL("family basic keyboard"), uL("FBKB F5"),           uL("NSCODE_F5"),       NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB I"),            uL("NSCODE_I"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB U"),            uL("NSCODE_U"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB J"),            uL("NSCODE_J"),        NULL, NULL, {0, NULL}},
	// Row 3 - Column 1
	{uL("family basic keyboard"), uL("FBKB M"),            uL("NSCODE_M"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB N"),            uL("NSCODE_N"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB 9"),            uL("NSCODE_9"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB 8"),            uL("NSCODE_8"),        NULL, NULL, {0, NULL}},
	// Row 4 - Column 0
	{uL("family basic keyboard"), uL("FBKB F4"),           uL("NSCODE_F4"),       NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Y"),            uL("NSCODE_Y"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB G"),            uL("NSCODE_G"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB H"),            uL("NSCODE_H"),        NULL, NULL, {0, NULL}},
	// Row 4 - Column 1
	{uL("family basic keyboard"), uL("FBKB B"),            uL("NSCODE_B"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB V"),            uL("NSCODE_V"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB 7"),            uL("NSCODE_7"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB 6"),            uL("NSCODE_6"),        NULL, NULL, {0, NULL}},
	// Row 5 - Column 0
	{uL("family basic keyboard"), uL("FBKB F3"),           uL("NSCODE_F3"),       NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB T"),            uL("NSCODE_T"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB R"),            uL("NSCODE_R"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB D"),            uL("NSCODE_D"),        NULL, NULL, {0, NULL}},
	// Row 5 - Column 1
	{uL("family basic keyboard"), uL("FBKB F"),            uL("NSCODE_F"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB C"),            uL("NSCODE_C"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB 5"),            uL("NSCODE_5"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB 4"),            uL("NSCODE_4"),        NULL, NULL, {0, NULL}},
	// Row 6 - Column 0
	{uL("family basic keyboard"), uL("FBKB F2"),           uL("NSCODE_F2"),       NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB W"),            uL("NSCODE_W"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB S"),            uL("NSCODE_S"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB A"),            uL("NSCODE_A"),        NULL, NULL, {0, NULL}},
	// Row 6 - Column 1
	{uL("family basic keyboard"), uL("FBKB X"),            uL("NSCODE_X"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Z"),            uL("NSCODE_Z"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB E"),            uL("NSCODE_E"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB 3"),            uL("NSCODE_3"),        NULL, NULL, {0, NULL}},
	// Row 7 - Column 0
	{uL("family basic keyboard"), uL("FBKB F1"),           uL("NSCODE_F1"),       NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Esc"),          uL("NSCODE_Escape"),   NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Q"),            uL("NSCODE_Q"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Ctr"),          uL("NSCODE_LCtrl"),    NULL, NULL, {0, NULL}},
	// Row 7 - Column 1
	{uL("family basic keyboard"), uL("FBKB LShift"),       uL("NSCODE_LShift"),   NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Grph"),         uL("NSCODE_Alt"),      NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB 1"),            uL("NSCODE_1"),        NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB 2"),            uL("NSCODE_2"),        NULL, NULL, {0, NULL}},
	// Row 8 - Column 0
	{uL("family basic keyboard"), uL("FBKB ClrHome"),      uL("NSCODE_Delete"),   NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Up"),           uL("NSCODE_Up"),       NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Right"),        uL("NSCODE_Right"),    NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Left"),         uL("NSCODE_Left"),     NULL, NULL, {0, NULL}},
	// Row 8 - Column 1
	{uL("family basic keyboard"), uL("FBKB Down"),         uL("NSCODE_Down"),     NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Space"),        uL("NSCODE_Space"),    NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Del"),          uL("NSCODE_Bckspc"),   NULL, NULL, {0, NULL}},
	{uL("family basic keyboard"), uL("FBKB Ins"),          uL("NSCODE_Insert"),   NULL, NULL, {0, NULL}},

	{
		uL("subor keyboard"), uL("extended mode"), uL("no"),
		uL("# possible values: yes, no"),
		NULL,
		{LENGTH(opt_no_yes), opt_no_yes}
	},
	// Row 0 - Column 1
	{uL("subor keyboard"), uL("SBKB 4"),            uL("NSCODE_4"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB G"),            uL("NSCODE_G"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB F"),            uL("NSCODE_F"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB C"),            uL("NSCODE_C"),        NULL, NULL, {0, NULL}},
	// Row 0 - Column 1
	{uL("subor keyboard"), uL("SBKB F2"),           uL("NSCODE_F2"),       NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB E"),            uL("NSCODE_E"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB 5"),            uL("NSCODE_5"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB V"),            uL("NSCODE_V"),        NULL, NULL, {0, NULL}},
	// Row 1 - Column 0
	{uL("subor keyboard"), uL("SBKB 2"),            uL("NSCODE_2"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB D"),            uL("NSCODE_D"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB S"),            uL("NSCODE_S"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB End"),          uL("NSCODE_End"),      NULL, NULL, {0, NULL}},
	// Row 1 - Column 1
	{uL("subor keyboard"), uL("SBKB F1"),           uL("NSCODE_F1"),       NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB W"),            uL("NSCODE_W"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB 3"),            uL("NSCODE_3"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB X"),            uL("NSCODE_X"),        NULL, NULL, {0, NULL}},
	// Row 2 - Column 0
	{uL("subor keyboard"), uL("SBKB Insert"),       uL("NSCODE_Insert"),   NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Backspace"),    uL("NSCODE_Bckspc"),   NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB PageDown"),     uL("NSCODE_PgDown"),   NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Right"),        uL("NSCODE_Right"),    NULL, NULL, {0, NULL}},
	// Row 2 - Column 1
	{uL("subor keyboard"), uL("SBKB F8"),           uL("NSCODE_F8"),       NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB PageUp"),       uL("NSCODE_PgUp"),     NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Delete"),       uL("NSCODE_Delete"),   NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Home"),         uL("NSCODE_Home"),     NULL, NULL, {0, NULL}},
	// Row 3 - Column 0
	{uL("subor keyboard"), uL("SBKB 9"),            uL("NSCODE_9"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB I"),            uL("NSCODE_I"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB L"),            uL("NSCODE_L"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Comma"),        uL("NSCODE_Comma"),    NULL, NULL, {0, NULL}},
	// Row 3 - Column 1
	{uL("subor keyboard"), uL("SBKB F5"),           uL("NSCODE_F5"),       NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB O"),            uL("NSCODE_O"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB 0"),            uL("NSCODE_0"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Period"),       uL("NSCODE_Period"),   NULL, NULL, {0, NULL}},
	// Row 4 - Column 0
	{uL("subor keyboard"), uL("SBKB BracketRight"), uL("NSCODE_BrkRight"), NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Return"),       uL("NSCODE_Return"),   NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Enter"),        uL("NSCODE_Enter"),    NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Up"),           uL("NSCODE_Up"),       NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Left"),         uL("NSCODE_Left"),     NULL, NULL, {0, NULL}},
	// Row 4 - Column 1
	{uL("subor keyboard"), uL("SBKB F7"),           uL("NSCODE_F7"),       NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB BracketLeft"),  uL("NSCODE_BrkLeft"),  NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Backslash"),    uL("NSCODE_Bckslsh"),  NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Down"),         uL("NSCODE_Down"),     NULL, NULL, {0, NULL}},
	// Row 5 - Column 0
	{uL("subor keyboard"), uL("SBKB Q"),            uL("NSCODE_Q"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB CapsLock"),     uL("NSCODE_CapsLck"),  NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Z"),            uL("NSCODE_Z"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Tab"),          uL("NSCODE_Tab"),      NULL, NULL, {0, NULL}},
	// Row 5 - Column 1
	{uL("subor keyboard"), uL("SBKB Esc"),          uL("NSCODE_Escape"),   NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB A"),            uL("NSCODE_A"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB 1"),            uL("NSCODE_1"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB LControl"),     uL("NSCODE_LCtrl"),    NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB RControl"),     uL("NSCODE_RCtrl"),    NULL, NULL, {0, NULL}},
	// Row 6 - Column 0
	{uL("subor keyboard"), uL("SBKB 7"),            uL("NSCODE_7"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Y"),            uL("NSCODE_Y"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB K"),            uL("NSCODE_K"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB M"),            uL("NSCODE_M"),        NULL, NULL, {0, NULL}},
	// Row 6 - Column 1
	{uL("subor keyboard"), uL("SBKB F4"),           uL("NSCODE_F4"),       NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB U"),            uL("NSCODE_U"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB 8"),            uL("NSCODE_8"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB J"),            uL("NSCODE_J"),        NULL, NULL, {0, NULL}},
	// Row 7 - Column 0
	{uL("subor keyboard"), uL("SBKB Minus"),        uL("NSCODE_Minus"),    NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Semicolon"),    uL("NSCODE_Semicln"),  NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Apostrophe"),   uL("NSCODE_Apstrph"),  NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Slash"),        uL("NSCODE_Slash"),    NULL, NULL, {0, NULL}},
	// Row 7 - Column 1
	{uL("subor keyboard"), uL("SBKB F6"),           uL("NSCODE_F6"),       NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB P"),            uL("NSCODE_P"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Equal"),        uL("NSCODE_Equal"),    NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB LShift"),       uL("NSCODE_LShift"),   NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB RShift"),       uL("NSCODE_RShift"),   NULL, NULL, {0, NULL}},
	// Row 8 - Column 0
	{uL("subor keyboard"), uL("SBKB T"),            uL("NSCODE_T"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB H"),            uL("NSCODE_H"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB N"),            uL("NSCODE_N"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Space"),        uL("NSCODE_Space"),    NULL, NULL, {0, NULL}},
	// Row 8 - Column 1
	{uL("subor keyboard"), uL("SBKB F3"),           uL("NSCODE_F3"),       NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB R"),            uL("NSCODE_R"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB 6"),            uL("NSCODE_6"),        NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB B"),            uL("NSCODE_B"),        NULL, NULL, {0, NULL}},
	// Row 9 - Column 0
	// Row 9 - Column 1
	// Row 10 - Column 0
	{uL("subor keyboard"), uL("SBKB Pause"),        uL("NULL"),            NULL, NULL, {0, NULL}}, // ???????
	{uL("subor keyboard"), uL("SBKB K4"),           uL("NSCODE_K4"),       NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB K7"),           uL("NSCODE_K7"),       NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB F11"),          uL("NSCODE_F11"),      NULL, NULL, {0, NULL}},
	// Row 10 - Column 1
	{uL("subor keyboard"), uL("SBKB F12"),          uL("NSCODE_F12"),      NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB K1"),           uL("NSCODE_K1"),       NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB K2"),           uL("NSCODE_K2"),       NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB K8"),           uL("NSCODE_K8"),       NULL, NULL, {0, NULL}},
	// Row 11 - Column 0
	{uL("subor keyboard"), uL("SBKB KMinus"),       uL("NSCODE_KMinus"),   NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB KPlus"),        uL("NSCODE_KPlus"),    NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB KAsterisk"),    uL("NSCODE_KAstrsk"),  NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB K9"),           uL("NSCODE_K9"),       NULL, NULL, {0, NULL}},
	// Row 11 - Column 1
	{uL("subor keyboard"), uL("SBKB F10"),          uL("NSCODE_F10"),      NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB K5"),           uL("NSCODE_K5"),       NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Kslash"),       uL("NSCODE_KSlash"),   NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB NumLock"),      uL("NSCODE_NmLock"),   NULL, NULL, {0, NULL}},
	// Row 12 - Column 0
	{uL("subor keyboard"), uL("SBKB QuoteLeft"),    uL("NSCODE_QtLeft"),   NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB K6"),           uL("NSCODE_K6"),       NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Alt"),          uL("NSCODE_Alt"),      NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB AltGr"),        uL("NSCODE_AltGr"),    NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB Break"),        uL("NULL"),            NULL, NULL, {0, NULL}}, // ??????
	// Row 12 - Column 1
	{uL("subor keyboard"), uL("SBKB F9"),           uL("NSCODE_F9"),       NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB K3"),           uL("NSCODE_K3"),       NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB KPeriod"),      uL("NSCODE_KPrd"),     NULL, NULL, {0, NULL}},
	{uL("subor keyboard"), uL("SBKB K0"),           uL("NSCODE_K0"),       NULL, NULL, {0, NULL}}
};

static const _settings jsc_cfg[] = {
	{uL("standard controller"), uL("A"),      NULL, NULL, NULL, {0, NULL}},
	{uL("standard controller"), uL("B"),      NULL, NULL, NULL, {0, NULL}},
	{uL("standard controller"), uL("Select"), NULL, NULL, NULL, {0, NULL}},
	{uL("standard controller"), uL("Start"),  NULL, NULL, NULL, {0, NULL}},
	{uL("standard controller"), uL("Up"),     NULL, NULL, NULL, {0, NULL}},
	{uL("standard controller"), uL("Down"),   NULL, NULL, NULL, {0, NULL}},
	{uL("standard controller"), uL("Left"),   NULL, NULL, NULL, {0, NULL}},
	{uL("standard controller"), uL("Right"),  NULL, NULL, NULL, {0, NULL}},
	{uL("standard controller"), uL("TurboA"), NULL, NULL, NULL, {0, NULL}},
	{uL("standard controller"), uL("TurboB"), NULL, NULL, NULL, {0, NULL}},
	{
		uL("system"), uL("Deadzone"), uL("40"),
		NULL,
		NULL,
		{0, NULL}
	},
	{
		uL("system"), uL("Buttons enabled"), uL("0xFFFFFFFFFFFFFFFF"),
		NULL,
		NULL,
		{0, NULL}
	},
	{
		uL("system"), uL("Axes enabled"), uL("0xFFFFFF"),
		NULL,
		NULL,
		{0, NULL}
	}
};

static const _list_settings list_settings[] = {
	{main_cfg, LENGTH(main_cfg)},
	{pgs_cfg, LENGTH(pgs_cfg)},
	{inp_cfg, LENGTH(inp_cfg)},
	{jsc_cfg, LENGTH(jsc_cfg)},
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
#if defined (FULLSCREEN_RESFREQ)
EXTERNC void settings_resolution_val_to_int(int *w, int *h, const uTCHAR *buffer);
#endif

EXTERNC void *settings_inp_rd_sc(int index, int type);
EXTERNC void settings_inp_wr_sc(void *str, int index, int type);
EXTERNC void settings_inp_all_defaults(_config_input *config_input, _array_pointers_port *array);
EXTERNC void settings_inp_port_defaults(_port *port, int index, int mode);
EXTERNC void settings_inp_port_button_default(int button, _port *port, int index, int mode);
EXTERNC DBWORD settings_inp_nes_keyboard_nscode_default(uTCHAR *name);
EXTERNC DBWORD settings_inp_nes_keyboard_nscode(uTCHAR *name);
EXTERNC void settings_inp_nes_keyboard_set_nscode(uTCHAR *name, DBWORD nscode);
EXTERNC void settings_inp_save(void);

EXTERNC void settings_pgs_parse(void);
EXTERNC void settings_pgs_save(void);

EXTERNC void settings_shp_parse(void);
EXTERNC void settings_shp_save(void);

EXTERNC void settings_jsc_parse(int index);
EXTERNC void settings_jsc_save(void);
EXTERNC int settings_jsc_deadzone_default(void);

#undef EXTERNC

#endif /* SETTINGS_H_ */
