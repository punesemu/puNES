/*
 * joystick.c
 *
 *  Created on: 03/nov/2011
 *      Author: fhorse
 */

#include "win.h"

#define jsElaborateAxis(axis, info)\
	value = (axis << 1) + 1;\
	if (joy->joyInfo.info == CENTER) {\
		mode = RELEASED;\
		if (joy->lastAxis[axis] > CENTER) {\
			value++;\
		}\
		joy->lastAxis[axis] = CENTER;\
	} else  {\
		mode = PRESSED;\
		if (joy->joyInfo.info > CENTER) {\
			value++;\
		}\
		joy->lastAxis[axis] = joy->joyInfo.info;\
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

void jsInit(void) {
	memset(&js1, 0, sizeof(js1));
	memset(&js2, 0, sizeof(js2));

	sprintf(js1.dev, "%s", jsnToName(port1.joy_id));
	js1.openTry = (BYTE) (rand() % 110);
	js1.clock = 0;
	js1.inputPort = input_port1;

	sprintf(js2.dev, "%s", jsnToName(port2.joy_id));
	js2.openTry = (BYTE) (rand() % 110);
	js2.clock = 1;
	js2.inputPort = input_port2;

	jsOpen(&js1);
	jsOpen(&js2);
}
void jsOpen(_js *joy) {
	BYTE index;

	joy->present = FALSE;

	if (!joy->dev[0] || !strcmp(joy->dev, "NULL")) {
		return;
	}

	for (index = 0; index < LENGTH(jsnlist); index++) {
		if (!strcmp(joy->dev, jsnlist[index].name)) {
			joy->id = jsnlist[index].value;
			joy->joyInfo.dwFlags = JOY_RETURNALL | JOY_RETURNCENTERED | JOY_USEDEADZONE;
			joy->joyInfo.dwSize = sizeof(joy->joyInfo);

			if (joyGetPosEx(joy->id, &joy->joyInfo) == JOYERR_NOERROR) {
				joyGetDevCaps(joy->id, &joy->joyCaps, sizeof(joy->joyCaps));
				joy->present = TRUE;
			}
		}
	}
}
void jsControl(_js *joy, _port *port) {
	WORD value = 0;
	BYTE mode = 0;

	if (++joy->clock < 3) {
		return;
	}

	joy->clock = 0;

	if (joy->present == FALSE) {
		if (++joy->openTry == 100) {
			joy->openTry = 0;
			jsOpen(joy);
		}
		return;
	}

	if (joyGetPosEx(joy->id, &joy->joyInfo) != JOYERR_NOERROR) {
		joy->present = FALSE;
		return;
	}

	/* esamino i pulsanti */
	if (joy->lastButtons != joy->joyInfo.dwButtons) {
		DWORD buttons = joy->joyInfo.dwButtons;
		DWORD lastButtons = joy->lastButtons;
		DWORD mask = JOY_BUTTON1;
		BYTE i;

		for (i = 0; i < MAXBUTTONS; i++) {
			BYTE after = buttons & 0x1;
			BYTE before = lastButtons & 0x1;

			if (after != before) {
				mode = after | 0x40;
				value = i | 0x400;
				if (after) {
					/* PRESSED */
					joy->lastButtons |= mask;
				} else {
					/* RELEASED */
					joy->lastButtons &= ~mask;
				}
				/* elaboro l'evento */
				if (joy->inputPort) {
					joy->inputPort(mode, value, JOYSTICK, port);
				}
				return;
			}
			mask <<= 1;
			buttons >>= 1;
			lastButtons >>= 1;
		}
	}

	if (joy->lastAxis[POV] != joy->joyInfo.dwPOV) {
		if (joy->lastAxis[POV] != JOY_POVCENTERED) {
			mode = RELEASED;

			if (joy->lastAxis[POV] == POV_table[0]) {
				/* up */
				value = 0x100;
			} else if (joy->lastAxis[POV] == POV_table[1]) {
				/* right */
				value = 0x101;
			} else if (joy->lastAxis[POV] == POV_table[2]) {
				/* down */
				value = 0x102;
			} else if (joy->lastAxis[POV] == POV_table[3]) {
				/* left */
				value = 0x103;
			} else if (joy->lastAxis[POV] == POV_table[4]) {
				/* up + right */
				value = 0x100;
				if (joy->inputPort) {
					joy->inputPort(mode, value, JOYSTICK, port);
				}
				value = 0x101;
			} else if (joy->lastAxis[POV] == POV_table[5]) {
				/* right + down */
				value = 0x101;
				if (joy->inputPort) {
					joy->inputPort(mode, value, JOYSTICK, port);
				}
				value = 0x102;
			} else if (joy->lastAxis[POV] == POV_table[6]) {
				/* down + left */
				value = 0x102;
				if (joy->inputPort) {
					joy->inputPort(mode, value, JOYSTICK, port);
				}
				value = 0x103;
			} else if (joy->lastAxis[POV] == POV_table[7]) {
				/* left + up */
				value = 0x103;
				if (joy->inputPort) {
					joy->inputPort(mode, value, JOYSTICK, port);
				}
				value = 0x100;
			}
			joy->lastAxis[POV] = JOY_POVCENTERED;
		} else {
			mode = PRESSED;

			if (joy->joyInfo.dwPOV == JOY_POVCENTERED) {
			} else if (joy->joyInfo.dwPOV == POV_table[0]) {
				/* up */
				value = 0x100;
			} else if (joy->joyInfo.dwPOV == POV_table[1]) {
				/* right */
				value = 0x101;
			} else if (joy->joyInfo.dwPOV == POV_table[2]) {
				/* down */
				value = 0x102;
			} else if (joy->joyInfo.dwPOV == POV_table[3]) {
				/* left */
				value = 0x103;
			} else if (joy->joyInfo.dwPOV == POV_table[4]) {
				/* up + right */
				value = 0x100;
				if (joy->inputPort) {
					joy->inputPort(mode, value, JOYSTICK, port);
				}
				value = 0x101;
			} else if (joy->joyInfo.dwPOV == POV_table[5]) {
				/* right + down */
				value = 0x101;
				if (joy->inputPort) {
					joy->inputPort(mode, value, JOYSTICK, port);
				}
				value = 0x102;
			} else if (joy->joyInfo.dwPOV == POV_table[6]) {
				/* down + left */
				value = 0x102;
				if (joy->inputPort) {
					joy->inputPort(mode, value, JOYSTICK, port);
				}
				value = 0x103;
			} else if (joy->joyInfo.dwPOV == POV_table[7]) {
				/* left + up */
				value = 0x103;
				if (joy->inputPort) {
					joy->inputPort(mode, value, JOYSTICK, port);
				}
				value = 0x100;
			}
			joy->lastAxis[POV] = joy->joyInfo.dwPOV;
		}
	} else if (joy->lastAxis[X] != joy->joyInfo.dwXpos) {
		jsElaborateAxis(X, dwXpos);
	} else if (joy->lastAxis[Y] != joy->joyInfo.dwYpos) {
		jsElaborateAxis(Y, dwYpos);
	} else if (joy->lastAxis[Z] != joy->joyInfo.dwZpos) {
		jsElaborateAxis(Z, dwZpos);
	} else if (joy->lastAxis[R] != joy->joyInfo.dwRpos) {
		jsElaborateAxis(R, dwRpos);
	} else if (joy->lastAxis[U] != joy->joyInfo.dwUpos) {
		jsElaborateAxis(U, dwUpos);
	} else if (joy->lastAxis[V] != joy->joyInfo.dwVpos) {
		jsElaborateAxis(V, dwVpos);
	}

	if (value && joy->inputPort) {
		joy->inputPort(mode, value, JOYSTICK, port);
	}
}
void jsClose(_js *joy) {
	return;
}
void jsQuit(void) {
	jsClose(&js1);
	jsClose(&js2);
}
char *jsToName(const DBWORD jsval, const _jselement *jslist, const DBWORD length) {
	BYTE index;
	static char str[20];

	memset(str, 0, 20);

	for (index = 0; index < length; index++) {
		if (jsval == jslist[index].value) {
			strcpy(str, jslist[index].name);
			return (str);
		}
	}
	return ((char *) jslist[0].name);
}
DBWORD jsFromName(const char *jsname, const _jselement *jslist, const DBWORD length) {
	DBWORD js = 0;
	BYTE index;

	for (index = 0; index < length; index++) {
		if (!strcmp(jsname, jslist[index].name)) {
			return (jslist[index].value);
		}
	}
	return (js);
}
