/*
 * conf.h
 *
 *  Created on: 06/dic/2014
 *      Author: fhorse
 */

#ifndef CONF_H_
#define CONF_H_

#include "apu.h"
#include "input.h"
#include "overscan.h"

typedef struct {
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

	BYTE render;
	BYTE scale;
	BYTE fullscreen;
	BYTE filter;
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
	BYTE bck_pause;
	WORD language;

	_config_input input;
	_config_apu apu;

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
