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
#include "draw_on_screen.h"
#include "input.h"

enum nsf_mode {
	NSF_NTSC_MODE,
	NSF_PAL_MODE,
	NSF_DENDY_MODE
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
enum nsf_routine_addresses {
	NSF_ROUTINE_SIZE = 0x50,
	NSF_ROUTINE_START = 0x2500,
	NSF_ROUTINE_END = NSF_ROUTINE_START + (NSF_ROUTINE_SIZE - 1),
	NSF_ROUTINE_CLEAR_ALL = 0x2518,
	NSF_ROUTINE_LOOP = 0x254D,
	NSF_ROUTINE_NMI = 0x2500,
	NSF_ROUTINE_NMI_RTI = 0x2514,
	NSF_ROUTINE_NORMAL = 0x2536,
	NSF_ROUTINE_NSF_INIT = 0x2525,
	NSF_ROUTINE_RESET = 0x2515,
	NSF_ROUTINE_YES_PLAY = 0x2521,

	NSF_DATA_SIZE = 0x0B,
	NSF_DATA_START = 0x2600,
	NSF_DATA_END = NSF_DATA_START + (NSF_DATA_SIZE - 1),
	NSF_DATA_INIT_LO = NSF_DATA_START + 0x00,
	NSF_DATA_INIT_HI = NSF_DATA_START + 0x01,
	NSF_DATA_PLAY_LO = NSF_DATA_START + 0x02,
	NSF_DATA_PLAY_HI = NSF_DATA_START + 0x03,
	NSF_DATA_IRQ_SUPPORT = NSF_DATA_START + 0x04,
	NSF_DATA_NON_RETURNING_INIT = NSF_DATA_START + 0x05,
	NSF_DATA_SUPPRESS_PLAY = NSF_DATA_START + 0x06,
	NSF_DATA_CURRENT_SONG = NSF_DATA_START + 0x07,
	NSF_DATA_TYPE = NSF_DATA_START + 0x08,
	NSF_DATA_LOOP_LO = NSF_DATA_START + 0x09,
	NSF_DATA_LOOP_HI = NSF_DATA_START + 0x0A,
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
	BYTE use_timer;
	int32_t time;
	int32_t fade;
	uTCHAR *track_label;
	uTCHAR *author;
} _nsf_info_song;
typedef struct _nsf_effect_coords {
	int x1, x2;
	int y1, y2;
	int w, h;
	int y_center;
} _nsf_effect_coords;
typedef struct _nsf {
	BYTE enabled;
	BYTE version;
	BYTE type;
	BYTE state;
	BYTE authors_note;
	BYTE repeat_song;
	const BYTE *routine;

	struct _nsf_region {
		BYTE supported;
		BYTE preferred;
	} region;
	struct _nsf_chunk {
		uint32_t length;
		char id[4];
	} chunk;
	struct _nsf_rate {
		DBWORD count;
		DBWORD reload;
	} rate;
	struct _nsf_nmi {
		BYTE in_use;
		DBWORD count;
		DBWORD reload;
	} nmi;
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
		WORD loop;
	} adr;
	struct _nsf_info {
		uTCHAR *name;
		uTCHAR *artist;
		uTCHAR *copyright;
		uTCHAR *ripper;
		uTCHAR *text;
	} info;
	struct _nsf_play_speed {
		WORD ntsc;
		WORD pal;
		WORD dendy;
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
	struct _nsf_options {
		BYTE visual_duration;
	} options;

	_dos_text_scroll scroll_info_nsf;
	_dos_text_scroll scroll_title_song;
	_dos_text_curtain curtain_info;
	_dos_text_curtain curtain_title_song;
	_nsf_info_song *info_song;
	_nsf_info_song current_song;
	_nsf_effect_coords effect_coords;
	_nsf_effect_coords effect_bars_coords;
} _nsf;
typedef struct _nsf2 {
	uint32_t prg_size;
	struct _nsf2_features {
		BYTE irq_support;
		BYTE non_returning_init;
		BYTE suppressed_PLAY;
		BYTE metadata;
	} features;
	struct _nsf2_irq {
		BYTE enable;
		WORD reload;
		WORD counter;
		BYTE vector[2];
	} irq;
} _nsf2;

#if defined (_NSF_STATIC_)
static char nsf_default_label[] = "<?>";
static const BYTE nsf_routine[NSF_ROUTINE_SIZE] = {
/*
;$2600-$2601 - INIT address
;$2602-$2603 - PLAY address
;$2604       - IRQ support
;$2605       - non-returning init
;$2606       - suppress PLAY
;$2607       - current song
;$2608       - NTSC, PAL or DANDY
;$2609-$260A - loop address

.org $2500
NMI:        PHA
            LDA $2605
            BEQ NMI_END
NMI_NRET:   TXA
            PHA
            TYA
            PHA
            SEI
            JSR _PLAY
            CLI
            PLA
            TAY
            PLA
            TAX
NMI_END:    PLA
NMI_RTI:    RTI
RESET:      JMP LOOP
CLEAR_ALL:  RTS
_INIT:      JMP ($2600)
_PLAY:      LDA $2606
            BNE _NO_PLAY
_YES_PLAY   JMP ($2602)
_NO_PLAY:   RTS
NSF_INIT:   JSR CLEAR_ALL
            LDA $2607
            LDX $2608
            LDY $2605
            BNE INIT_NRET
            JSR _INIT
NORMAL:     JSR _PLAY
            JMP LOOP
INIT_NRET:  LDY #$80
            JSR _INIT
            CLI
            LDA $2607
            LDX $2608
            LDY #$81
            JSR _INIT
LOOP:       JMP ($2609)
.end

CLEAR_ALL         $2518
INIT_NRET         $253C
LOOP              $254D
NMI               $2500
NMI_END           $2513
NMI_NRET          $2506
NMI_RTI           $2514
NORMAL            $2536
NSF_INIT          $2525
RESET             $2515
_INIT             $2519
_NO_PLAY          $2524
_PLAY             $251C
_YES_PLAY         $2521
*/
	0x48, 0xAD, 0x05, 0x26, 0xF0, 0x0D, 0x8A, 0x48,
	0x98, 0x48, 0x78, 0x20, 0x1C, 0x25, 0x58, 0x68,
	0xA8, 0x68, 0xAA, 0x68, 0x40, 0x4C, 0x4D, 0x25,
	0x60, 0x6C, 0x00, 0x26, 0xAD, 0x06, 0x26, 0xD0,
	0x03, 0x6C, 0x02, 0x26, 0x60, 0x20, 0x18, 0x25,
	0xAD, 0x07, 0x26, 0xAE, 0x08, 0x26, 0xAC, 0x05,
	0x26, 0xD0, 0x09, 0x20, 0x19, 0x25, 0x20, 0x1C,
	0x25, 0x4C, 0x4D, 0x25, 0xA0, 0x80, 0x20, 0x19,
	0x25, 0x58, 0xAD, 0x07, 0x26, 0xAE, 0x08, 0x26,
	0xA0, 0x81, 0x20, 0x19, 0x25, 0x6C, 0x09, 0x26
};
#endif

extern _nsf nsf;
extern _nsf2 nsf2;

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
