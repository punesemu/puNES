/*
 * cfg_file.c
 *
 *  Created on: 31/lug/2011
 *      Author: fhorse
 */

#include "cfg_file.h"
#include "gui.h"
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

void set_default(void) {

#define _port_kb_default(port, button, name)\
	port.input[KEYBOARD][button] = keyval_from_name(name);
#define _port_js_default(port, button, name)\
	port.input[JOYSTICK][button] = name_to_jsv(name)
#define port_kb_default(port, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10)\
	_port_kb_default(port, BUT_A,  s1);\
	_port_kb_default(port, BUT_B,  s2);\
	_port_kb_default(port, SELECT, s3);\
	_port_kb_default(port, START,  s4);\
	_port_kb_default(port, UP,     s5);\
	_port_kb_default(port, DOWN,   s6);\
	_port_kb_default(port, LEFT,   s7);\
	_port_kb_default(port, RIGHT,  s8);\
	_port_kb_default(port, TRB_A,  s9);\
	_port_kb_default(port, TRB_B,  s10)
#define port_js_default(port, id, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10)\
	port.joy_id = name_to_jsn(id);\
	_port_js_default(port, BUT_A,  s1);\
	_port_js_default(port, BUT_B,  s2);\
	_port_js_default(port, SELECT, s3);\
	_port_js_default(port, START,  s4);\
	_port_js_default(port, UP,     s5);\
	_port_js_default(port, DOWN,   s6);\
	_port_js_default(port, LEFT,   s7);\
	_port_js_default(port, RIGHT,  s8);\
	_port_js_default(port, TRB_A,  s9);\
	_port_js_default(port, TRB_B,  s10)

	/* default */
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
		int index;

		for (index = 0; index <= APU_MASTER; index++) {
			cfg_from_file.apu.channel[index] = TRUE;
			cfg_from_file.apu.volume[index] = 1.0f;
		}
	}
	cfg_from_file.samplerate = S44100;
	cfg_from_file.channels = STEREO;
	cfg_from_file.stereo_delay = STEREO_DELAY_DEFAULT;
	cfg_from_file.audio_quality = AQ_HIGH;
	cfg_from_file.swap_duty = 0;
	cfg_from_file.gamegenie = FALSE;

	port1.type = CTRL_STANDARD;
	port_kb_default(port1, "S", "A", "Z", "X", "Up", "Down", "Left", "Right", "W", "Q");
	//port_js_default(port1, "JOYSTICKID1", "JB1", "JB0", "JB8", "JB9", "JA1MIN", "JA1PLS", "JA0MIN",
	//		"JA0PLS", "JB2", "JB3");
	port1.turbo[TURBOA].frequency = TURBO_BUTTON_DELAY_DEFAULT;
	port1.turbo[TURBOB].frequency = TURBO_BUTTON_DELAY_DEFAULT;

	//port2.type = FALSE;
	//port2.joy_id = name_to_jsn("JOYSTICKID2");
	port2.turbo[TURBOA].frequency = TURBO_BUTTON_DELAY_DEFAULT;
	port2.turbo[TURBOB].frequency = TURBO_BUTTON_DELAY_DEFAULT;
}
