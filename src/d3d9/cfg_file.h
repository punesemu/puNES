/*
 * cfg_file.h
 *
 *  Created on: 31/lug/2011
 *      Author: fhorse
 */

#ifndef CFG_FILE_H_
#define CFG_FILE_H_

#include "common.h"
#include "apu.h"
#include "input.h"

typedef struct {
	BYTE save_on_exit;
	BYTE mode;
	BYTE samplerate;
	BYTE channels;
	double stereo_delay;
	BYTE audio_quality;
	BYTE swap_duty;
	BYTE fps;
	BYTE frameskip;

	BYTE render;
	BYTE scale;
	BYTE fullscreen;
	BYTE filter;
	BYTE ntsc_format;
	BYTE palette;
	BYTE vsync;
	BYTE aspect_ratio;
	BYTE oscan;
	BYTE oscan_default;
	BYTE gamegenie;

	_config_input input;
	_config_apu apu;
} _config;

_config cfg_from_file, *cfg;

void cfg_file_init(void);
void cfg_file_parse(void);
void cfg_file_save(void);
void cfg_file_pgs_parse(void);
void cfg_file_pgs_save(void);
void cfg_file_input_parse(void);
void cfg_file_input_save(void);
void cfg_file_set_all_input_default(_config_input *config_input, _array_pointers_port *array);
void cfg_file_set_kbd_joy_default(_port *port, int index, int mode);
char *cfg_file_set_kbd_joy_button_default(int index, int mode, int button);


#endif /* CFG_FILE_H_ */
