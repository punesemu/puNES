/*
 * joystick.c
 *
 *  Created on: 03/nov/2011
 *      Author: fhorse
 */

#include "win.h"

#define js_elaborate_axis(axis, info)\
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
	}
#define middle (JOY_POVRIGHT / 2)

static const WORD POV_table[8] = {
	/* Up           (JOY_POVFORWARD)           */
	JOY_POVFORWARD,
	/* Right        (JOY_POVRIGHT)             */
	JOY_POVRIGHT,
	/* Down         (JOY_POVBACKWARD)          */
	JOY_POVBACKWARD,
	/* Left         (JOY_POVLEFT)              */
	JOY_POVLEFT,
	/* Up + Right   (JOY_POVFORWARD + middle)  */
	JOY_POVFORWARD + middle,
	/* Right + Down (JOY_POVRIGHT + middle)    */
	JOY_POVRIGHT + middle,
	/* Down + Left  (JOY_POVBACKWARD + middle) */
	JOY_POVBACKWARD + middle,
	/* Left + Up    (JOY_POVLEFT + middle)     */
	JOY_POVLEFT + middle,
};

void js_init(void) {
	memset(&js1, 0, sizeof(js1));
	memset(&js2, 0, sizeof(js2));

	sprintf(js1.dev, "%s", jsn_to_name(port1.joy_id));
	js1.open_try = (BYTE) (rand() % 110);
	js1.clock = 0;
	js1.input_port = input_port1;

	sprintf(js2.dev, "%s", jsn_to_name(port2.joy_id));
	js2.open_try = (BYTE) (rand() % 110);
	js2.clock = 1;
	js2.input_port = input_port2;

	js_open(&js1);
	js_open(&js2);
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
			joy->joy_info.dwFlags = JOY_RETURNALL | JOY_RETURNCENTERED | JOY_USEDEADZONE;
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

	if (++joy->clock < 3) {
		return;
	}

	joy->clock = 0;

	if (joy->present == FALSE) {
		if (++joy->open_try == 100) {
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
				mode = after | 0x40;
				value = i | 0x400;
				if (after) {
					/* PRESSED */
					joy->last_buttons |= mask;
				} else {
					/* RELEASED */
					joy->last_buttons &= ~mask;
				}
				/* elaboro l'evento */
				if (joy->input_port) {
					joy->input_port(mode, value, JOYSTICK, port);
				}
				return;
			}
			mask <<= 1;
			buttons >>= 1;
			last_buttons >>= 1;
		}
	}

	if (joy->last_axis[POV] != joy->joy_info.dwPOV) {
		if (joy->last_axis[POV] != JOY_POVCENTERED) {
			mode = RELEASED;

			if (joy->last_axis[POV] == POV_table[0]) {
				/* up */
				value = 0x100;
			} else if (joy->last_axis[POV] == POV_table[1]) {
				/* right */
				value = 0x101;
			} else if (joy->last_axis[POV] == POV_table[2]) {
				/* down */
				value = 0x102;
			} else if (joy->last_axis[POV] == POV_table[3]) {
				/* left */
				value = 0x103;
			} else if (joy->last_axis[POV] == POV_table[4]) {
				/* up + right */
				value = 0x100;
				if (joy->input_port) {
					joy->input_port(mode, value, JOYSTICK, port);
				}
				value = 0x101;
			} else if (joy->last_axis[POV] == POV_table[5]) {
				/* right + down */
				value = 0x101;
				if (joy->input_port) {
					joy->input_port(mode, value, JOYSTICK, port);
				}
				value = 0x102;
			} else if (joy->last_axis[POV] == POV_table[6]) {
				/* down + left */
				value = 0x102;
				if (joy->input_port) {
					joy->input_port(mode, value, JOYSTICK, port);
				}
				value = 0x103;
			} else if (joy->last_axis[POV] == POV_table[7]) {
				/* left + up */
				value = 0x103;
				if (joy->input_port) {
					joy->input_port(mode, value, JOYSTICK, port);
				}
				value = 0x100;
			}
			joy->last_axis[POV] = JOY_POVCENTERED;
		} else {
			mode = PRESSED;

			if (joy->joy_info.dwPOV == JOY_POVCENTERED) {
			} else if (joy->joy_info.dwPOV == POV_table[0]) {
				/* up */
				value = 0x100;
			} else if (joy->joy_info.dwPOV == POV_table[1]) {
				/* right */
				value = 0x101;
			} else if (joy->joy_info.dwPOV == POV_table[2]) {
				/* down */
				value = 0x102;
			} else if (joy->joy_info.dwPOV == POV_table[3]) {
				/* left */
				value = 0x103;
			} else if (joy->joy_info.dwPOV == POV_table[4]) {
				/* up + right */
				value = 0x100;
				if (joy->input_port) {
					joy->input_port(mode, value, JOYSTICK, port);
				}
				value = 0x101;
			} else if (joy->joy_info.dwPOV == POV_table[5]) {
				/* right + down */
				value = 0x101;
				if (joy->input_port) {
					joy->input_port(mode, value, JOYSTICK, port);
				}
				value = 0x102;
			} else if (joy->joy_info.dwPOV == POV_table[6]) {
				/* down + left */
				value = 0x102;
				if (joy->input_port) {
					joy->input_port(mode, value, JOYSTICK, port);
				}
				value = 0x103;
			} else if (joy->joy_info.dwPOV == POV_table[7]) {
				/* left + up */
				value = 0x103;
				if (joy->input_port) {
					joy->input_port(mode, value, JOYSTICK, port);
				}
				value = 0x100;
			}
			joy->last_axis[POV] = joy->joy_info.dwPOV;
		}
	} else if (joy->last_axis[X] != joy->joy_info.dwXpos) {
		js_elaborate_axis(X, dwXpos);
	} else if (joy->last_axis[Y] != joy->joy_info.dwYpos) {
		js_elaborate_axis(Y, dwYpos);
	} else if (joy->last_axis[Z] != joy->joy_info.dwZpos) {
		js_elaborate_axis(Z, dwZpos);
	} else if (joy->last_axis[R] != joy->joy_info.dwRpos) {
		js_elaborate_axis(R, dwRpos);
	} else if (joy->last_axis[U] != joy->joy_info.dwUpos) {
		js_elaborate_axis(U, dwUpos);
	} else if (joy->last_axis[V] != joy->joy_info.dwVpos) {
		js_elaborate_axis(V, dwVpos);
	}

	if (value && joy->input_port) {
		joy->input_port(mode, value, JOYSTICK, port);
	}
}
void js_close(_js *joy) {
	return;
}
void js_quit(void) {
	js_close(&js1);
	js_close(&js2);
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
