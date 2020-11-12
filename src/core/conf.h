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

#ifndef CONF_H_
#define CONF_H_

#include "apu.h"
#include "input.h"

typedef struct _last_geometry {
	int x, y;
	int w, h;
} _last_geometry;
typedef struct _toolbar {
	BYTE area;
	BYTE hidden;
} _toolbar;
typedef struct _config {
	BYTE save_on_exit;
	BYTE mode;
	BYTE samplerate;
	BYTE channels_mode;
	double stereo_delay;
	BYTE audio_buffer_factor;
	BYTE reverse_bits_dpcm;
	BYTE swap_duty;
	BYTE rewind_minutes;
	BYTE ff_velocity;
	BYTE hide_sprites;
	BYTE hide_background;
	BYTE unlimited_sprites;
	BYTE scale;
	BYTE fullscreen;
	BYTE fullscreen_in_window;
	DBWORD filter;
	DBWORD shader;
	BYTE ntsc_format;
	BYTE palette;
	BYTE disable_swap_emphasis_pal;
	BYTE vsync;
	BYTE integer_scaling;
	BYTE stretch;
	BYTE oscan;
	BYTE oscan_black_borders;
	BYTE oscan_black_borders_fscr;
	BYTE oscan_default;
	BYTE pixel_aspect_ratio;
	BYTE PAR_soft_stretch;
	BYTE interpolation;
	BYTE cheat_mode;
	BYTE txt_on_screen;
	BYTE screen_rotation;
	BYTE text_rotation;
	BYTE show_fps;
	BYTE input_display;
	BYTE disable_tv_noise;
	BYTE disable_sepia_color;
#if defined (WITH_OPENGL)
	BYTE disable_srgb_fbo;
#endif
	BYTE bck_pause;
	WORD language;
	WORD dipswitch;
	BYTE ppu_overclock;
	BYTE ppu_overclock_dmc_control_disabled;
	WORD extra_vb_scanlines;
	WORD extra_pr_scanlines;
	BYTE save_battery_ram_file;
	BYTE nsf_player_effect;
	BYTE nsf_player_nsfe_playlist;
	BYTE nsf_player_nsfe_fadeout;

	_config_input input;
	_config_apu apu;
#if defined (WITH_FFMPEG)
	struct _config_recording {
		BYTE audio_format;
		BYTE video_format;
		BYTE quality;
		BYTE use_emu_resolution;
		BYTE follow_rotation;
		BYTE output_resolution;
		int output_custom_w;
		int output_custom_h;
	} recording;
#endif

	uTCHAR shader_file[LENGTH_FILE_NAME_LONG];
	uTCHAR palette_file[LENGTH_FILE_NAME_LONG];
	uTCHAR save_file[LENGTH_FILE_NAME_LONG];
	uTCHAR gg_rom_file[LENGTH_FILE_NAME_LONG];
	uTCHAR fds_bios_file[LENGTH_FILE_NAME_LONG];
	uTCHAR last_import_cheat_path[LENGTH_FILE_NAME_LONG];
#if defined (WITH_FFMPEG)
	uTCHAR last_rec_audio_path[LENGTH_FILE_NAME_LONG];
	uTCHAR last_rec_video_path[LENGTH_FILE_NAME_LONG];
#endif

	uTCHAR audio_output[100];
	uTCHAR audio_input[100];

	_last_geometry lg;
	_last_geometry lg_settings;
	_toolbar toolbar;
} _config;

extern _config *cfg;
extern _config cfg_from_file;

#endif /* CONF_H_ */
