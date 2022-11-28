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

#ifndef INFO_H_
#define INFO_H_

#include "common.h"

enum frame_status_modes {
	FRAME_FINISHED,
	FRAME_STARTED,
	FRAME_INTERRUPTED
};

typedef struct _info_sh1sum {
	struct _info_sha1sum_prg {
		BYTE value[20];
		char string[41];
	} prg;
	struct _info_sha1sum_chr {
		BYTE value[20];
		char string[41];
	} chr;
} _info_sh1sum;
typedef struct _info {
	struct _info_rom {
		uTCHAR file[LENGTH_FILE_NAME_LONG];
		uTCHAR change_rom[LENGTH_FILE_NAME_LONG];
		uTCHAR *from_load_menu;
	} rom;
	BYTE format;
	BYTE machine[2];
	struct _info_mapper {
		WORD id;
		BYTE submapper_nes20;
		BYTE submapper;
		BYTE extend_wr;
		BYTE extend_rd;
		BYTE ram_plus_op_controlled_by_mapper;
		BYTE trainer;
		BYTE misc_roms;
		BYTE force_battery_io;
		BYTE ext_console_type;
		BYTE mirroring;
	} mapper;
	BYTE mirroring_db;
	BYTE portable;
	BYTE id;
	BYTE stop;
	BYTE reset;
	BYTE frame_status;
	BYTE gui;
	BYTE turn_off;
	BYTE no_rom;
	int pause;
	int no_ppu_draw_screen;
	BYTE pause_from_gui;
	BYTE on_cfg;
	BYTE first_illegal_opcode;
	BYTE recording_on_air;
	BYTE recording_is_a_video;
	BYTE cpu_rw_extern;
	BYTE screenshot;
#if defined (WITH_OPENGL)
	BYTE sRGB_FBO_in_use;
#endif
	_info_sh1sum sha1sum;
	struct _info_crc32 {
		uint32_t prg;
		uint32_t chr;
		uint32_t misc;
		uint32_t total;
	} crc32;
	struct _info_chr {
		WORD chips;
		struct _info_chr_rom {
			BYTE is_ram;
			DBWORD banks_8k;
			DBWORD banks_4k;
			DBWORD banks_1k;
			struct _info_chr_rom_max {
				DBWORD banks_8k;
				DBWORD banks_4k;
				DBWORD banks_2k;
				DBWORD banks_1k;
			} max;
		} rom;
		struct _info_chr_ram {
			SWORD banks_8k_plus;
			struct _info_chr_ram_max {
				DBWORD banks_8k;
				DBWORD banks_4k;
				DBWORD banks_2k;
				DBWORD banks_1k;
			} max;
			//struct _info_chr_ram_bat {
			//	BYTE banks;
			//	BYTE start;
			//} bat;
		} ram;
	} chr;
	struct _info_prg {
		WORD chips;
		struct _info_prg_rom {
			DBWORD banks_16k;
			DBWORD banks_8k;
			struct _info_prg_rom_max {
				DBWORD banks_32k;
				DBWORD banks_16k;
				DBWORD banks_8k;
				DBWORD banks_8k_before_last;
				DBWORD banks_4k;
				DBWORD banks_2k;
				DBWORD banks_1k;
			} max;
		} rom;
		struct _info_prg_ram {
			BYTE banks_8k_plus;
			struct _info_prg_ram_bat {
				BYTE banks;
				BYTE start;
			} bat;
		} ram;
	} prg;
	BYTE r4016_dmc_double_read_disabled;
	BYTE r2002_race_condition_disabled;
	BYTE r4014_precise_timing_disabled;
	BYTE r2002_jump_first_vblank;
	WORD default_dipswitches;
	WORD extra_from_db;
	DBWORD bat_ram_frames;
	DBWORD bat_ram_frames_snap;
	BYTE doublebuffer;
	BYTE start_frame_0;
	WORD CPU_PC_before;
	BYTE zapper_is_present;
	BYTE disable_tick_hw;
	BYTE start_with_hidden_gui;
#if defined (FULLSCREEN_RESFREQ)
	BYTE old_machine_type;
#endif
	BYTE decimal_mode;

#if !defined (RELEASE)
	BYTE snd_info;
#endif
} _info;

extern _info info;

#endif /* INFO_H_ */
