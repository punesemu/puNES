/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

#include <stddef.h>
#include "common.h"

enum frame_status_modes {
	FRAME_FINISHED,
	FRAME_STARTED,
	FRAME_INTERRUPTED
};

typedef struct _info_lag_frame {
	uint8_t next;
	uint8_t actual;
	uint32_t totals;

	// da salvare
	uint32_t consecutive;
} _info_lag_frame;
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
	struct _info_mapper {
		WORD id;
		BYTE submapper_nes20;
		BYTE submapper;
		size_t prgrom_size;
		DBWORD prgrom_banks_16k;
		size_t chrrom_size;
		DBWORD chrrom_banks_8k;
		BYTE extend_wr;
		BYTE extend_rd;
		BYTE battery;
		BYTE force_battery_io;
		BYTE ext_console_type;
		BYTE mirroring;
		BYTE supported;
		struct _info_mapper_nes20db {
			BYTE in_use;
		} nes20db;
	} mapper;
	struct _info_header {
		BYTE format;
		WORD mapper;
		BYTE submapper;
		size_t prgrom_size;
		DBWORD prgrom;
		size_t chrrom_size;
		DBWORD chrrom;
		DBWORD prgram;
		DBWORD prgnvram;
		DBWORD chrram;
		DBWORD chrnvram;
		BYTE trainer;
		BYTE misc_roms;
		BYTE battery;
		BYTE ext_console_type;
		BYTE mirroring;
		BYTE cpu_timing;
		BYTE vs_hardware;
		BYTE vs_ppu;
	} header;
	struct _info_crc32 {
		uint32_t prg;
		uint32_t chr;
		uint32_t trainer;
		uint32_t misc;
		uint32_t total;
	} crc32;

	BYTE format;
	BYTE machine[2];

	BYTE prg_truncated;
	BYTE chr_truncated;
	BYTE misc_truncated;

	BYTE mirroring_db;
	BYTE portable;
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
	_info_lag_frame lag_frame;
	BYTE r4016_dmc_double_read_disabled;
	BYTE r2002_race_condition_disabled;
	BYTE r4014_precise_timing_disabled;
	BYTE r2002_jump_first_vblank;
	WORD extra_from_db;
	DBWORD bat_ram_frames;
	DBWORD bat_ram_frames_snap;
	BYTE doublebuffer;
	BYTE start_frame_0;
	WORD CPU_PC_before;
	BYTE zapper_is_present;
	BYTE disable_tick_hw;
	BYTE start_with_hidden_gui;
	BYTE block_recent_roms_update;
#if defined (FULLSCREEN_RESFREQ)
	BYTE old_machine_type;
#endif
	BYTE decimal_mode;
	BYTE number_of_nes;
	union _exec_nes_op {
		BYTE b[2];
		WORD w;
	} exec_cpu_op;

#if !defined (RELEASE)
	BYTE snd_info;
#endif
} _info;

extern _info info;

#endif /* INFO_H_ */
