/*
 * cfg_file.c
 *
 *  Created on: 31/lug/2011
 *      Author: fhorse
 */



#include "cfg_file.h"
#define __GUI_BASE__
#define __GUI_SND__
#include "gui.h"
#undef __GUI_SND__
#undef __GUI_BASE__
#include "gfx.h"
#include "snd.h"
#include "clock.h"
#include "fps.h"
#include "input.h"
#include "audio_quality.h"






#define INIFILE NAME  ".cfg"
#define INPUTFILE     "input.cfg"
#define MAXLEN         512
#define cfg_evaluate(src, dst, chr)\
{\
	char *buf = 0;\
	buf = strtok(src, chr);\
	if (!buf) {\
		continue;\
	}\
	strcpy(dst, buf);\
}
#define cfg_search(structp, prm, start, desc, cmd)\
{\
	char buf[MAXLEN];\
	memset(buf, 0x00, MAXLEN);\
	strcpy(buf,  structp[prm].lname);\
	trim_space(buf);\
	if (strcmp(key, buf) == 0) {\
		param_search(start, value, desc, cmd);\
		continue;\
	}\
}
#define cfg_input_search(param, port, type)\
{\
	BYTE index, found = FALSE;\
	for(index = 0; index < LENGTH(param); index++) {\
		char buf[MAXLEN];\
		memset(buf, 0x00, MAXLEN);\
		strcpy(buf, param[index].lname);\
		trim_space(buf);\
		if (strcmp(key, buf) == 0) {\
			if (type == JOYSTICK) {\
				if (index == (LENGTH(param) - 1)) {\
					port.joy_id = name_to_jsn(value);\
				} else {\
					port.input[JOYSTICK][index] = name_to_jsv(value);\
				}\
			} else {\
				port.input[KEYBOARD][index] = keyval_from_name(value);\
			}\
			found = TRUE;\
			break;\
		}\
	}\
	if (found) {\
		continue;\
	}\
}

void set_default(void);

void cfg_file_init(void) {
	cfg = &cfg_from_file;
}
void cfg_file_parse(void) {
	/* attivo la modalita' configurazione */
	info.on_cfg = TRUE;

	/* default */
	set_default();

	return;
}
void cfg_file_save(void) {
	return;
}
void cfg_file_pgs_parse(void) {
	return;
}
void cfg_file_pgs_save(void) {
	return;
}
void cfg_file_input_parse(void) {
	return;
}
void cfg_file_input_save(void) {
	return;
}
void cfg_file_set_all_input_default(_config_input *config_input, _array_pointers_port *array) {
	BYTE i;

	config_input->permit_updown_leftright = FALSE;
	config_input->controller_mode = CTRL_MODE_NES;

	for (i = PORT1; i < PORT_MAX; i++) {
		_port *port = array->port[i];

		switch (i) {
			case PORT1:
				port->type = CTRL_STANDARD;
				port->joy_id = name_to_jsn("JOYSTICKID1");
				break;
			case PORT2:
				port->type = FALSE;
				port->joy_id = name_to_jsn("JOYSTICKID2");
				break;
			default:
				port->type = FALSE;
				port->joy_id = name_to_jsn("NULL");
				break;
		}

		port->turbo[TURBOA].frequency = TURBO_BUTTON_DELAY_DEFAULT;
		port->turbo[TURBOB].frequency = TURBO_BUTTON_DELAY_DEFAULT;
		cfg_file_set_kbd_joy_default(port, i, KEYBOARD);
		cfg_file_set_kbd_joy_default(port, i, JOYSTICK);
	}
}
void cfg_file_set_kbd_joy_default(_port *port, int index, int mode) {
	BYTE i;

	for (i = BUT_A; i < MAX_STD_PAD_BUTTONS; i++) {
		if (mode == KEYBOARD) {
			port->input[KEYBOARD][i] = keyval_from_name(
			        cfg_file_set_kbd_joy_button_default(index, KEYBOARD, i));
		} else {
			port->input[JOYSTICK][i] = name_to_jsv(
			        cfg_file_set_kbd_joy_button_default(index, JOYSTICK, i));
		}
	}
}
char *cfg_file_set_kbd_joy_button_default(int index, int mode, int button) {
	static char default_value_port[PORT_MAX][2][MAX_STD_PAD_BUTTONS][15] = {
		{
			{
				"S",  "A",    "Z",    "X",
				"Up", "Down", "Left", "Right",
				"W",  "Q"
			},
			{
				"JB1",    "JB0",    "JB8",    "JB9",
				"JA1MIN", "JA1PLS", "JA0MIN", "JA0PLS",
				"JB2",    "JB3"
			}
		},
		{
			{
#ifdef GTK
				"Page_Down", "End",     "Insert",  "Delete",
				"KP_Up",     "KP_Down", "KP_Left", "KP_Right",
				"Home",      "Page_Up"
#else
				"PgDown",    "End",     "Insert",  "Delete",
				"NumPad8",   "NumPad2", "NumPad4", "NumPad6",
				"Home",      "PgUp"
#endif
			},
			{
				"JB1",    "JB0",    "JB8",    "JB9",
				"JA1MIN", "JA1PLS", "JA0MIN", "JA0PLS",
				"JB2",    "JB3"
			}
		},
		{
			{
				"NULL", "NULL", "NULL", "NULL",
				"NULL", "NULL", "NULL", "NULL",
				"NULL", "NULL"
			},
			{
				"JB1",    "JB0",    "JB8",    "JB9",
				"JA1MIN", "JA1PLS", "JA0MIN", "JA0PLS",
				"JB2",    "JB3"
			}
		},
		{
			{
				"NULL", "NULL", "NULL", "NULL",
				"NULL", "NULL", "NULL", "NULL",
				"NULL", "NULL"
			},
			{
				"JB1",    "JB0",    "JB8",    "JB9",
				"JA1MIN", "JA1PLS", "JA0MIN", "JA0PLS",
				"JB2",    "JB3"
			}
		}
	};

	return(default_value_port[index][mode][button]);
}

void set_default(void) {
	cfg_from_file.mode = AUTO;
	machine = machinedb[NTSC - 1];

	cfg_from_file.fps = FPS_DEFAULT;
	cfg_from_file.frameskip = 0;
	cfg_from_file.scale = gfx.scale_before_fscreen = X2;
	cfg_from_file.oscan_default = OSCAN_OFF;
	cfg_from_file.filter = NTSC_FILTER;
	cfg_from_file.ntsc_format = COMPOSITE;
	cfg_from_file.palette = PALETTE_NTSC;

	cfg_from_file.render = RENDER_HLSL;
	gfx_set_render(cfg_from_file.render);

	cfg_from_file.vsync = TRUE;
	//cfg_from_file.fullscreen = NO_FULLSCR;
	cfg_from_file.aspect_ratio = FALSE;
	//cfg_from_file.save_on_exit = FALSE;

	{
		BYTE i;

		for (i = APU_S1; i <= APU_MASTER; i++) {
			cfg_from_file.apu.channel[i] = TRUE;
			cfg_from_file.apu.volume[i] = 1.0f;
		}
	}
	cfg_from_file.samplerate = S44100;
	cfg_from_file.channels = STEREO;
	cfg_from_file.stereo_delay = STEREO_DELAY_DEFAULT;
	cfg_from_file.audio_quality = AQ_HIGH;
	cfg_from_file.swap_duty = 0;

	cfg_from_file.gamegenie = FALSE;

	{
		_array_pointers_port array;
		BYTE i;

		for (i = PORT1; i < PORT_MAX; i++) {
			array.port[i] = &port[i];
		}

		cfg_file_set_all_input_default(&cfg_from_file.input, &array);
	}
}
