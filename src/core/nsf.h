/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#ifndef NSF_H_
#define NSF_H_

#include "common.h"
#include "input.h"

enum nsf_mode {
	NSF_NTSC_MODE,
	NSF_PAL_MODE,
	NSF_DUAL_MODE
};
enum nsf_states {
	NSF_STOP = 0x01,
	NSF_PLAY = 0x02,
	NSF_PAUSE = 0x04,
	NSF_CHANGE_SONG = 0x10,
	NSF_NEXT = 0x100,
	NSF_PREV = 0x200,
	NSF_RESTART_SONG = 0x400,
};
enum nsf_routine_bytes {
	NSF_R_SONG = 1,
	NSF_R_TYPE = 3,
	NSF_R_INIT_LO = 5,
	NSF_R_INIT_HI = 6,
	NSF_R_PLAY_LO = 12,
	NSF_R_PLAY_HI = 13,
	NSF_R_JMP_PLAY = 15,
	NSF_R_PLAY = 0x0B,
	NSF_R_LOOP = 0x0E,
	NSF_R_PLAY_INST = 0x250B,
	NSF_R_START = 0x2500,
	NSF_R_END = 0x2510,
	NSF_R_MASK = 0x001F,
};
enum nsf_effect_types {
	NSF_EFFECT_BARS,
	NSF_EFFECT_BARS_MIXED,
	NSF_EFFECT_RAW,
	NSF_EFFECT_RAW_FULL,
	NSF_EFFECT_HANNING,
	NSF_EFFECT_HANNING_FULL,
	NSF_EFFECTS
};

typedef struct _nsf_info_song {
	int32_t time;
	int32_t fade;
	char *track_label;
} _nsf_info_song;
typedef struct _nsf_text_scroll {
	int x, y;
	int rows;
	char buffer[2048];
	char string[1024];
	double timer;
	double reload;
	int velocity;
	int pixel_len;
	int pixel;
} _nsf_text_scroll;
typedef struct _nsf_text_curtain_line {
	char *text;
	int length;
} _nsf_text_curtain_line;
typedef struct _nsf_text_curtain {
	int x, y;
	int count;
	int index;
	int rows;
	BYTE pause;
	double timer;
	_nsf_text_curtain_line *line;
	struct _nsf_text_curtain_borders {
		int left;
		int right;
		int bottom;
		int top;
	} borders;
	struct _nsf_text_curtain_reload {
		double r1;
		double r2;
	} reload;
	struct _nsf_redraw {
		BYTE all;
		int left;
		int right;
		int bottom;
		int top;
	} redraw;
} _nsf_text_curtain;
typedef struct _nsf_effect_coords {
	int x1, x2;
	int y1, y2;
	int w, h;
	int y_center;
} _nsf_effect_coords;
typedef struct _nsf {
	BYTE enabled;
	BYTE version;
	BYTE draw_mask_frames;
	BYTE type;
	BYTE state;
	BYTE made_tick;
	DBWORD frames;

	struct _nsf_rate {
		DBWORD count;
		DBWORD reload;
	} rate;
	struct _nsf_songs {
		BYTE total;
		BYTE starting;
		BYTE current;
		BYTE started;
	} songs;
	struct _nsf_address {
		WORD load;
		WORD init;
		WORD play;
	} adr;
	struct _nsf_info {
		char *track_label;
		char *auth;
		char *name;
		char *artist;
		char *copyright;
		char *ripper;
	} info;
	struct _nsf_play_speed {
		WORD ntsc;
		WORD pal;
	} play_speed;
	struct _nsf_bankswitch {
		BYTE enabled;
		BYTE banks[8];
	} bankswitch;
	struct _nsf_extra_sound_chips {
		BYTE vrc6;
		BYTE vrc7;
		BYTE fds;
		BYTE mmc5;
		BYTE namco163;
		BYTE sunsoft5b;
	} sound_chips;
	struct _nsf_routine {
		BYTE prg[17];
		BYTE INT_NMI;
		BYTE INT_RESET;
	} routine;
	struct _nsf_timers {
		BYTE update_only_diff;
		double button[INPUT_DECODE_COUNTS];
		double total_rom;
		double song;
		double fadeout;
		double silence;
		double last_tick;
		double diff;
		double effect;
	} timers;
	struct _nsf_playlist {
		BYTE *data;
		BYTE index;
		BYTE starting;
		uint32_t count;
	} playlist;
	struct _nsf_text {
		BYTE *data;
		BYTE index;
		uint32_t count;
	} text;
	struct _nsf_options {
		BYTE visual_duration;
	} options;

	_nsf_info_song *info_song;
	_nsf_info_song current_song;
	_nsf_effect_coords effect_coords;
	_nsf_effect_coords effect_bars_coords;
	_nsf_text_scroll scroll_info_nsf;
	_nsf_text_scroll scroll_title_song;
	_nsf_text_curtain curtain_title_song;
	_nsf_text_curtain curtain_info;
} _nsf;

#if defined (_NSF_STATIC_)
static char nsf_default_label[] = {"<?>"};
static const BYTE nsf_routine[17] = {
//	0     1
	0xA9, 0x00,       // 0x2500 : LDA [current song]
//	2     3
	0xA2, 0x00,       // 0x2502 : LDX [PAL or NTSC]
//	4     5     6
	0x20, 0x00, 0x00, // 0x2504 : JSR [address init routine]
//	7
	0x78,             // 0x2507 : SEI
//	8     9     10
	0x4C, 0x0E, 0x25, // 0x2508 : JMP 0x250E
//	11    12    13
	0x20, 0x00, 0x00, // 0x250B : JSR [address play routine]
//	14    15    16
	0x4C, 0x00, 0x25  // 0x250E : JMP [0x250B / 0x250E]
};
#endif

extern _nsf nsf;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void nsf_init(void);
EXTERNC void nsf_quit(void);
EXTERNC void nsf_reset(void);
EXTERNC void nsf_info(void);
EXTERNC BYTE nsf_load_rom(void);
EXTERNC void nsf_after_load_rom(void);
EXTERNC void nsf_init_tune(void);
EXTERNC void nsf_tick(void);
EXTERNC void extcl_audio_samples_mod_nsf(SWORD *samples, int count);

EXTERNC void nsf_reset_prg(void);
EXTERNC void nsf_reset_timers(void);
EXTERNC void nsf_reset_song_title(void);

EXTERNC void nsf_main_screen(void);
EXTERNC void nsf_main_screen_event(void);

EXTERNC void nsf_controls_mouse_in_gui(int x_mouse, int y_mouse);

EXTERNC void nsf_effect(void);

#undef EXTERNC

#endif /* NSF_H_ */
