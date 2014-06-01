/*
 * joystick.c
 *
 *  Created on: 03/nov/2011
 *      Author: fhorse
 */

#include "win.h"

enum joy_misc {
	JOY_MAX_FRAME_COUNT = 3,
	JOY_MAX_FRAME_COUNT_NO_PRESENT = 300 / JOY_MAX_FRAME_COUNT,
	JOY_POV_UP = 0,
	JOY_POV_RIGHT,
	JOY_POV_DOWN,
	JOY_POV_LEFT,
	JOY_POV_UP_RIGHT,
	JOY_POV_RIGHT_DOWN,
	JOY_POV_DOWN_LEFT,
	JOY_POV_LEFT_UP
};

#define js_elaborate_axis(axis, info)\
	if ((joy->joy_info.info > (CENTER - sensibility))\
	        && (joy->joy_info.info < (CENTER + sensibility))) {\
		joy->joy_info.info = CENTER;\
	} else {\
		DWORD diff = ((joy->joy_info.info < joy->last_axis[axis]) ?\
				(joy->last_axis[axis] - joy->joy_info.info) :\
				(joy->joy_info.info - joy->last_axis[axis]));\
		if (diff < sensibility) {\
			joy->joy_info.info = joy->last_axis[axis];\
		}\
	}\
	value = (axis << 1) + 1;\
	if (joy->joy_info.info == CENTER) {\
		mode = RELEASED;\
		if (joy->last_axis[axis] > CENTER) {\
			value++;\
		}\
		joy->last_axis[axis] = CENTER;\
	} else  {\
		mode = PRESSED;\
		if (joy->joy_info.info > CENTER) {\
			value++;\
		}\
		joy->last_axis[axis] = joy->joy_info.info;\
	}\
	if (value && joy->input_decode_event) {\
		joy->input_decode_event(mode, value, JOYSTICK, port);\
	}
#define js_elaborate_pov(md, pov)\
	mode = md;\
	value = 0;\
	if (pov == JOY_POVCENTERED) {\
		;\
	} else if (pov == POV_table[JOY_POV_UP]) {\
		value = 0x100;\
	} else if (pov == POV_table[JOY_POV_RIGHT]) {\
		value = 0x101;\
	} else if (pov == POV_table[JOY_POV_DOWN]) {\
		value = 0x102;\
	} else if (pov == POV_table[JOY_POV_LEFT]) {\
		value = 0x103;\
	} else if (pov == POV_table[JOY_POV_UP_RIGHT]) {\
		value = 0x100;\
		if (joy->input_decode_event) {\
			joy->input_decode_event(mode, value, JOYSTICK, port);\
		}\
		value = 0x101;\
	} else if (pov == POV_table[JOY_POV_RIGHT_DOWN]) {\
		value = 0x101;\
		if (joy->input_decode_event) {\
			joy->input_decode_event(mode, value, JOYSTICK, port);\
		}\
		value = 0x102;\
	} else if (pov == POV_table[JOY_POV_DOWN_LEFT]) {\
		value = 0x102;\
		if (joy->input_decode_event) {\
			joy->input_decode_event(mode, value, JOYSTICK, port);\
		}\
		value = 0x103;\
	} else if (pov == POV_table[JOY_POV_LEFT_UP]) {\
		value = 0x103;\
		if (joy->input_decode_event) {\
			joy->input_decode_event(mode, value, JOYSTICK, port);\
		}\
		value = 0x100;\
	}\
	if (value && joy->input_decode_event) {\
		joy->input_decode_event(mode, value, JOYSTICK, port);\
	}

static const WORD POV_table[8] = {
	JOY_POVFORWARD                      , /* Up           */
	JOY_POVRIGHT                        , /* Right        */
	JOY_POVBACKWARD                     , /* Down         */
	JOY_POVLEFT                         , /* Left         */
	JOY_POVFORWARD  + (JOY_POVRIGHT / 2), /* Up + Right   */
	JOY_POVRIGHT    + (JOY_POVRIGHT / 2), /* Right + Down */
	JOY_POVBACKWARD + (JOY_POVRIGHT / 2), /* Down + Left  */
	JOY_POVLEFT     + (JOY_POVRIGHT / 2), /* Left + Up    */
};

static const DWORD sensibility = (PLUS / 100) * 5;

void js_init(void) {
	BYTE i;

	for (i = PORT1; i < PORT_MAX; i++) {
		memset(&js[i], 0x00, sizeof(_js));

		if (port[i].joy_id == name_to_jsn("NULL")) {
			continue;
		}

		sprintf(js[i].dev, "%s", jsn_to_name(port[i].joy_id));
		js[i].open_try = (BYTE) (rand() % 110);
		/* TODO : controllare con quattro gamepad come si comporta */
		js[i].clock = i & 0x01;
		js[i].input_decode_event = input_decode_event[i];

		js_open(&js[i]);
	}
}
void js_open(_js *joy) {
	BYTE index;

	joy->present = FALSE;

	if (!joy->dev[0] || !strcmp(joy->dev, "NULL")) {
		return;
	}

	for (index = 0; index < LENGTH(jsn_list); index++) {
		if (!strcmp(joy->dev, jsn_list[index].name)) {
			joy->id = jsn_list[index].value;
			joy->joy_info.dwFlags = JOY_RETURNALL | JOY_RETURNCENTERED | JOY_RETURNPOV
			        | JOY_USEDEADZONE;
			joy->joy_info.dwSize = sizeof(joy->joy_info);

			if (joyGetPosEx(joy->id, &joy->joy_info) == JOYERR_NOERROR) {
				joyGetDevCaps(joy->id, &joy->joy_caps, sizeof(joy->joy_caps));
				joy->present = TRUE;
			}
		}
	}
}
void js_control(_js *joy, _port *port) {
	WORD value = 0;
	BYTE mode = 0;

	if (++joy->clock < JOY_MAX_FRAME_COUNT) {
		return;
	}

	joy->clock = 0;

	if (joy->present == FALSE) {
		if (++joy->open_try == JOY_MAX_FRAME_COUNT_NO_PRESENT) {
			joy->open_try = 0;
			js_open(joy);
		}
		return;
	}

	if (joyGetPosEx(joy->id, &joy->joy_info) != JOYERR_NOERROR) {
		joy->present = FALSE;
		return;
	}

	/* esamino i pulsanti */
	if (joy->last_buttons != joy->joy_info.dwButtons) {
		DWORD buttons = joy->joy_info.dwButtons;
		DWORD last_buttons = joy->last_buttons;
		DWORD mask = JOY_BUTTON1;
		BYTE i;

		for (i = 0; i < MAX_BUTTONS; i++) {
			BYTE after = buttons & 0x1;
			BYTE before = last_buttons & 0x1;

			if (after != before) {
				mode = after;
				value = i | 0x400;
				if (after) {
					/* PRESSED */
					joy->last_buttons |= mask;
				} else {
					/* RELEASED */
					joy->last_buttons &= ~mask;
				}
				/* elaboro l'evento */
				if (joy->input_decode_event) {
					joy->input_decode_event(mode, value, JOYSTICK, port);
				}
			}
			mask <<= 1;
			buttons >>= 1;
			last_buttons >>= 1;
		}
	}
	/*esamino i POV */
	if ((joy->joy_caps.wCaps & JOYCAPS_HASPOV) && (joy->last_axis[POV] != joy->joy_info.dwPOV)) {
		js_elaborate_pov(RELEASED, joy->last_axis[POV])
		js_elaborate_pov(PRESSED, joy->joy_info.dwPOV)
		joy->last_axis[POV] = joy->joy_info.dwPOV;
	}
	/*esamino gli assi */
	if (joy->last_axis[X] != joy->joy_info.dwXpos) {
		js_elaborate_axis(X, dwXpos)
	}
	if (joy->last_axis[Y] != joy->joy_info.dwYpos) {
		js_elaborate_axis(Y, dwYpos)
	}
	if ((joy->joy_caps.wCaps & JOYCAPS_HASZ) && (joy->last_axis[Z] != joy->joy_info.dwZpos)) {
		js_elaborate_axis(Z, dwZpos)
	}
	if ((joy->joy_caps.wCaps & JOYCAPS_HASR) && (joy->last_axis[R] != joy->joy_info.dwRpos)) {
		js_elaborate_axis(R, dwRpos)
	}
	if ((joy->joy_caps.wCaps & JOYCAPS_HASU) && (joy->last_axis[U] != joy->joy_info.dwUpos)) {
		//js_elaborate_axis(U, dwUpos)
	}
	if ((joy->joy_caps.wCaps & JOYCAPS_HASV) && (joy->last_axis[V] != joy->joy_info.dwVpos)) {
		//printf("v : %ld %ld %ld\n", joy->joy_info.dwPOV, joy->last_axis[V], joy->joy_info.dwVpos);
		//js_elaborate_axis(V, dwVpos)
	}
}
void js_close(_js *joy) {
	return;
}
void js_quit(void) {
	BYTE i;

	for (i = PORT1; i < PORT_MAX; i++) {
		js_close(&js[i]);
	}
}
BYTE js_is_connected(int dev) {
	JOYCAPS caps;

	if (joyGetDevCaps(dev, &caps, sizeof(caps)) != JOYERR_NOERROR) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}

char *js_to_name(const DBWORD val, const _js_element *list, const DBWORD length) {
	BYTE index;
	static char str[20];

	memset(str, 0, 20);

	for (index = 0; index < length; index++) {
		if (val == list[index].value) {
			strcpy(str, list[index].name);
			return (str);
		}
	}
	return ((char *) list[0].name);
}
DBWORD js_from_name(const char *name, const _js_element *list, const DBWORD length) {
	DBWORD js = 0;
	BYTE index;

	for (index = 0; index < length; index++) {
		if (!strcmp(name, list[index].name)) {
			return (list[index].value);
		}
	}
	return (js);
}
