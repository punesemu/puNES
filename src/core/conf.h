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

#ifndef CONF_H_
#define CONF_H_

#include "apu.h"
#include "input.h"
#include "overscan.h"

typedef struct _config {
	BYTE save_on_exit;
	BYTE mode;
	BYTE samplerate;
	BYTE channels_mode;
	double stereo_delay;
	BYTE audio_quality;
	BYTE audio_buffer_factor;
	BYTE swap_duty;
	BYTE fps;
	BYTE frameskip;
	BYTE ff_velocity;
	BYTE hide_sprites;
	BYTE hide_background;
	BYTE unlimited_sprites;
#if defined (WITH_OPENGL)
	BYTE render;
#endif
	BYTE scale;
	BYTE fullscreen;
	DBWORD filter;
	BYTE ntsc_format;
	BYTE palette;
	BYTE disable_swap_emphasis_pal;
	BYTE vsync;
	BYTE stretch;
	BYTE oscan;
	BYTE oscan_default;
	BYTE pixel_aspect_ratio;
	BYTE PAR_soft_stretch;
	BYTE interpolation;
	BYTE cheat_mode;
	BYTE txt_on_screen;
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

	_config_input input;
	_config_apu apu;

	char shader_file[LENGTH_FILE_NAME_LONG];
	char palette_file[LENGTH_FILE_NAME_LONG];
	char save_file[LENGTH_FILE_NAME_LONG];

	struct _last_pos {
		int x;
		int y;
	} last_pos;
} _config;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC _config cfg_from_file, *cfg;

#undef EXTERNC

#endif /* CONF_H_ */
