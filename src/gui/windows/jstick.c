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

#include "gui.h"
#include <regstr.h>

static INLINE void js_special_case(int dev, _js_special_case *jsc);

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

void js_init(void) {
	BYTE i;

	for (i = PORT1; i < PORT_MAX; i++) {
		memset(&js[i], 0x00, sizeof(_js));

		if (port[i].joy_id == name_to_jsn("NULL")) {
			continue;
		}

		sprintf(js[i].dev, "%s", jsn_to_name(port[i].joy_id));
		js[i].open_try = (BYTE) (rand() % 110);
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
				js_special_case(joy->id, &joy->jsc);
			}
		}
	}
}
void js_control(_js *joy, _port *port) {
	//static const DWORD sensibility = (PLUS / 100) * 5;
	static const DWORD sensibility = (PLUS / 100) * 13;
	WORD value = 0;
	BYTE mode = 0;

#define js_elaborate_axis(axs, info)\
	if (joy->jsc.axis_with_CENTER_equal_to_0.axs == TRUE) {\
		if (joy->joy_info.info > 0) {\
			joy->joy_info.info = CENTER;\
		}\
		if ((joy->last_axis[axs] != joy->joy_info.info)) {\
			value = (axs << 1) + 1;\
			if (joy->joy_info.info == 0) {\
				mode = RELEASED;\
			} else {\
				mode = PRESSED;\
			}\
			if (joy->input_decode_event) {\
				joy->input_decode_event(mode, value, JOYSTICK, port);\
			}\
		}\
		joy->last_axis[axs] = joy->joy_info.info;\
	} else {\
		if ((joy->joy_info.info > (CENTER - sensibility))\
				&& (joy->joy_info.info < (CENTER + sensibility))) {\
			joy->joy_info.info = CENTER;\
		} else {\
			DWORD diff = ((joy->joy_info.info < joy->last_axis[axs]) ?\
					(joy->last_axis[axs] - joy->joy_info.info) :\
					(joy->joy_info.info - joy->last_axis[axs]));\
			if (diff < sensibility) {\
				joy->joy_info.info = joy->last_axis[axs];\
			}\
		}\
		value = (axs << 1) + 1;\
		if (joy->joy_info.info == CENTER) {\
			mode = RELEASED;\
			if (joy->last_axis[axs] > CENTER) {\
				value++;\
			}\
			joy->last_axis[axs] = CENTER;\
		} else  {\
			mode = PRESSED;\
			if (joy->joy_info.info > CENTER) {\
				value++;\
			}\
			joy->last_axis[axs] = joy->joy_info.info;\
		}\
		if (value && joy->input_decode_event) {\
			joy->input_decode_event(mode, value, JOYSTICK, port);\
		}\
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
		js_elaborate_axis(U, dwUpos)
	}
	if ((joy->joy_caps.wCaps & JOYCAPS_HASV) && (joy->last_axis[V] != joy->joy_info.dwVpos)) {
		js_elaborate_axis(V, dwVpos)
	}

#undef js_elaborate_axis
#undef js_elaborate_pov
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
	JOYINFOEX info;

	info.dwFlags = JOY_RETURNALL;
	info.dwSize = sizeof(info);

	if (joyGetPosEx(dev, &info) != JOYERR_NOERROR) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
char *js_name_device(int dev) {
	static char name[512];
	char rkey[256], rvalue[256], rname[256];
	char *reg_key;
	JOYCAPS caps;
	HKEY topkey, key;
	DWORD size;
	LONG result;

	joyGetDevCaps(dev, &caps, sizeof(caps));

	reg_key = (CHAR *) &caps.szRegKey;

	snprintf(rkey, sizeof(rkey), "%s\\%s\\%s", REGSTR_PATH_JOYCONFIG, reg_key, REGSTR_KEY_JOYCURR);

	{
		topkey = HKEY_LOCAL_MACHINE;
		result = RegOpenKeyExA(topkey, rkey, 0, KEY_READ, &key);

		if (result != ERROR_SUCCESS) {
			topkey = HKEY_CURRENT_USER;
			result = RegOpenKeyExA(topkey, rkey, 0, KEY_READ, &key);
		}

		if (result != ERROR_SUCCESS) {
			return (NULL);
		}
	}

	/* trovo la chiave di registro delle proprieta' del joystick */
	{
		size = sizeof(rname);
		snprintf(rvalue, sizeof(rvalue), "Joystick%d%s", dev + 1, REGSTR_VAL_JOYOEMNAME);
		result = RegQueryValueExA(key, rvalue, 0, 0, (LPBYTE) rname, &size);
		RegCloseKey(key);

		if (result != ERROR_SUCCESS) {
			return (NULL);
		}
	}

	/* apro la chiave di registro */
	{
		snprintf(rkey, sizeof(rkey), "%s\\%s", REGSTR_PATH_JOYOEM, rname);
		result = RegOpenKeyExA(topkey, rkey, 0, KEY_READ, &key);

		if (result != ERROR_SUCCESS) {
			return (NULL);
		}
	}

	/* trovo la dimensione della stringa del nome OEM */
	{
		size = sizeof(rvalue);
		result = RegQueryValueExA(key, REGSTR_VAL_JOYOEMNAME, 0, 0, NULL, &size);

		if (result == ERROR_SUCCESS) {
			memset(&name[0], 0x00, sizeof(name));

			if (size >= sizeof(name)) {
				size = sizeof(name) - 1;
			}
			/* leggo il nome OEM */
			result = RegQueryValueExA(key, REGSTR_VAL_JOYOEMNAME, 0, 0, (LPBYTE) & name[0], &size);
		}
	}

	RegCloseKey(key);

	return ((char *) name);
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
DBWORD js_read_in_dialog(int dev, int fd) {
	static const DWORD sensibility = (PLUS / 100) * 35;
	static _js_special_case jsc;
	JOYINFOEX joy_info;
	JOYCAPS joy_caps;
	DBWORD value = 0;

#define adjust_axis_joy(axs, info)\
	if (jsc.axis_with_CENTER_equal_to_0.axs == TRUE) {\
		if ((joy_info.info == 0) || (joy_info.info == CENTER)) {\
			joy_info.info = CENTER;\
		} else {\
			joy_info.info = MINUS;\
		}\
	} else {\
		if (joy_info.info != CENTER) {\
			if (joy_info.info < (CENTER - sensibility)) {\
				joy_info.info = MINUS;\
			} else if (joy_info.info > (CENTER + sensibility)) {\
				joy_info.info = PLUS;\
			} else {\
				joy_info.info = CENTER;\
			}\
		}\
	}
#define read_axis_joy(axs, info)\
	value = (axs << 1) + 1;\
	if (joy_info.info > CENTER) {\
		value++;\
	}

	joyGetDevCaps(dev, &joy_caps, sizeof(joy_caps));

	joy_info.dwFlags = JOY_RETURNALL | JOY_RETURNCENTERED | JOY_RETURNPOV | JOY_USEDEADZONE;
	joy_info.dwSize = sizeof(joy_info);

	if (joyGetPosEx(dev, &joy_info) != JOYERR_NOERROR) {
		return (value);
	}

	js_special_case(dev, &jsc);

	adjust_axis_joy(X, dwXpos)
	adjust_axis_joy(Y, dwYpos)
	adjust_axis_joy(Z, dwZpos)
	adjust_axis_joy(R, dwRpos)
	adjust_axis_joy(U, dwUpos)
	adjust_axis_joy(V, dwVpos)

	/* esamino i pulsanti */
	if (joy_info.dwButtons) {
		int i;

		for (i = BUT_A; i < MAX_BUTTONS; i++) {
			BYTE button = joy_info.dwButtons & 0x1;

			if (button) {
				value = i | 0x400;
				return (value);
			}
			joy_info.dwButtons >>= 1;
		}
	}

	/* esamino gli assi */
	if ((joy_caps.wCaps & JOYCAPS_HASPOV) && (joy_info.dwPOV != JOY_POVCENTERED)) {
		if (joy_info.dwPOV == JOY_POVFORWARD) {
			value = 0x100;
		} else if (joy_info.dwPOV == JOY_POVRIGHT) {
			value = 0x101;
		} else if (joy_info.dwPOV == JOY_POVBACKWARD) {
			value = 0x102;
		} else if (joy_info.dwPOV == JOY_POVLEFT) {
			value = 0x103;
		}
	} else if (joy_info.dwXpos != CENTER) {
		read_axis_joy(X, dwXpos);
	} else if (joy_info.dwYpos != CENTER) {
		read_axis_joy(Y, dwYpos);
	} else if ((joy_caps.wCaps & JOYCAPS_HASZ) && (joy_info.dwZpos != CENTER)) {
		read_axis_joy(Z, dwZpos);
	} else if ((joy_caps.wCaps & JOYCAPS_HASR) && (joy_info.dwRpos != CENTER)) {
		read_axis_joy(R, dwRpos);
	} else if ((joy_caps.wCaps & JOYCAPS_HASU) && (joy_info.dwUpos != CENTER)) {
		read_axis_joy(U, dwUpos);
	} else if ((joy_caps.wCaps & JOYCAPS_HASV) && (joy_info.dwVpos != CENTER)) {
		read_axis_joy(V, dwVpos);
	}

	return (value);

#undef adjust_axis_joy
#undef read_axis_joy
}
BYTE js_shcut_read(_js_sch *js_sch, _js *joy, int id) {
	static const DWORD sensibility = (PLUS / 100) * 25;
	WORD value = 0;
	BYTE mode = 0;

	js_sch->value = 0;
	js_sch->mode = 255;

#define js_elaborate_axis(axs, info)\
	if (joy->jsc.axis_with_CENTER_equal_to_0.axs == TRUE) {\
		if (joy->joy_info.info > 0) {\
			joy->joy_info.info = CENTER;\
		}\
		if ((joy->last_axis[axs] != joy->joy_info.info)) {\
			value = (axs << 1) + 1;\
			if (joy->joy_info.info == 0) {\
				mode = RELEASED;\
			} else {\
				mode = PRESSED;\
			}\
			if (joy->input_decode_event) {\
				joy->input_decode_event(mode, value, JOYSTICK, port);\
			}\
		}\
		joy->last_axis[axs] = joy->joy_info.info;\
	} else {\
		if ((joy->joy_info.info > (CENTER - sensibility))\
			&& (joy->joy_info.info < (CENTER + sensibility))) {\
			joy->joy_info.info = CENTER;\
		} else {\
			DWORD diff = ((joy->joy_info.info < joy->last_axis[axs]) ?\
				(joy->last_axis[axs] - joy->joy_info.info) :\
				(joy->joy_info.info - joy->last_axis[axs]));\
				if (diff < sensibility) {\
					joy->joy_info.info = joy->last_axis[axs];\
				}\
		}\
		if (joy->joy_info.info != joy->last_axis[axs]) {\
			value = (axs << 1) + 1;\
			if (joy->joy_info.info == CENTER) {\
				mode = RELEASED;\
				if (joy->last_axis[axs] > CENTER) {\
					value++;\
				}\
				joy->last_axis[axs] = CENTER;\
			} else  {\
				mode = PRESSED;\
				if (joy->joy_info.info > CENTER) {\
					value++;\
				}\
				joy->last_axis[axs] = joy->joy_info.info;\
			}\
		}\
	}\
	if (value) {\
		js_sch->value = value;\
		js_sch->mode = mode;\
		return (EXIT_OK);\
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
	}

	if (++joy->clock < JOY_MAX_FRAME_COUNT) {
		return (EXIT_ERROR);
	}

	joy->clock = 0;

	if (joy->present == FALSE) {
		if (++joy->open_try == JOY_MAX_FRAME_COUNT_NO_PRESENT / 2) {
			joy->open_try = 0;
			js_open(joy);
		}
		return (EXIT_ERROR);
	}

	if (joyGetPosEx(joy->id, &joy->joy_info) != JOYERR_NOERROR) {
		joy->present = FALSE;
		return (EXIT_ERROR);
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
				if (value) {
					js_sch->value = value;
					js_sch->mode = mode;
					return (EXIT_OK);
				}
			}
			mask <<= 1;
			buttons >>= 1;
			last_buttons >>= 1;
		}
	}
	/* esamino i POV */
	if ((joy->joy_caps.wCaps & JOYCAPS_HASPOV) && (joy->last_axis[POV] != joy->joy_info.dwPOV)) {
		js_elaborate_pov(RELEASED, joy->last_axis[POV])
		if (value) {
			js_sch->value = value;
			js_sch->mode = mode;
		}
		js_elaborate_pov(PRESSED, joy->joy_info.dwPOV)
		joy->last_axis[POV] = joy->joy_info.dwPOV;
		if (value && !js_sch->value) {
			js_sch->value = value;
			js_sch->mode = mode;
		}
		if (js_sch->value) {
			return (EXIT_OK);
		}
	}
	/* esamino gli assi */
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
		js_elaborate_axis(U, dwUpos)
	}
	if ((joy->joy_caps.wCaps & JOYCAPS_HASV) && (joy->last_axis[V] != joy->joy_info.dwVpos)) {
		js_elaborate_axis(V, dwVpos)
	}

	return (EXIT_ERROR);

#undef js_elaborate_axis
#undef js_elaborate_pov
}

static INLINE void js_special_case(int dev, _js_special_case *jsc) {
	memset (jsc, 0x00, sizeof(_js_special_case));

	if ((gui.version_os == WIN_TEN) &&
			(strcmp(js_name_device(dev), "Controller (Xbox One For Windows)") == 0)) {
		jsc->axis_with_CENTER_equal_to_0.Z = TRUE;
		jsc->axis_with_CENTER_equal_to_0.R = TRUE;
	}
}
