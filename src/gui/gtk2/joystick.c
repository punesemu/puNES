/*
 * joystick.c
 *
 *  Created on: 03/nov/2011
 *      Author: fhorse
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "joystick.h"
#include "input.h"

void jsInit(void) {
	memset(&js1, 0, sizeof(js1));
	memset(&js2, 0, sizeof(js2));

	sprintf(js1.dev, "%s%d", JSDEVPATH, port1.joyID);
	js1.inputPort = inputPort1;

	sprintf(js2.dev, "%s%d", JSDEVPATH, port2.joyID);
	js2.inputPort = inputPort2;

	jsOpen(&js1);
	jsOpen(&js2);
}
void jsOpen(_js *joy) {
	joy->fd = 0;
	if (joy->dev[0] && strcmp(joy->dev, "NULL")) {
		joy->fd = open(joy->dev, O_RDONLY | O_NONBLOCK);
		if (joy->fd < 0) {
			joy->fd = 0;
		}
	}
}
void jsControl(_js *joy, _port *port) {
	_jsevent jse;
	DBWORD value = 0;
	BYTE mode = 0;

	if (!joy->fd) {
		if (++joy->openTry == 300) {
			joy->openTry = 0;
			jsOpen(joy);
		}
		return;
	}

	if (!jsReadEvent(&jse, joy)) {
		//fprintf(stderr, "%04X %04X %d\n", jse.type, jse.number, (SWORD) jse.value);
		jse.type &= ~JS_EVENT_INIT;

		if (jse.type == JS_EVENT_AXIS) {
			BYTE axis = jse.number & 0x000F;

			if (jse.value) {
				mode = PRESSED;
				value = (jse.number << 1) + 1;
				if (jse.value > 0) {
					value++;
				}
				//fprintf(stderr, "pressed  : %s (%d) (%d)\n\n", jsvalToName(value), axis, value);
				joy->last[axis] = value;
			} else {
				mode = RELEASED;
				value = joy->last[axis];
				//fprintf(stderr, "released : %s (%d) (%d)\n\n", jsvalToName(value), axis, value);
				joy->last[axis] = 0;
			}
		} else if (jse.type == JS_EVENT_BUTTON) {
			value = jse.number | 0x400;
			if (jse.value == 0) {
				mode = RELEASED;
				//fprintf(stderr, "released : %s\n\n", jsvToName(value));
			} else if (jse.value == 1) {
				mode = PRESSED;
				//fprintf(stderr, "pressed  : %s\n\n", jsvToName(value));
			} else {
				value = 0;
			}
		}

		if (value && joy->inputPort) {
			joy->inputPort(mode, value, JOYSTICK, port);
		}
	}
}
void jsClose(_js *joy) {
	if (joy->fd) {
		close(joy->fd);
	}
	joy->fd = 0;
}
void jsQuit(void) {
	jsClose(&js1);
	jsClose(&js2);
}
BYTE jsReadEvent(_jsevent *jse, _js *joy) {
	SWORD bytes;
	BYTE size = sizeof(*jse);

	if ((bytes = read(joy->fd, jse, size)) == size) {
		return (EXIT_OK);
	}

	if (errno == ENODEV) {
		jsClose(joy);
	}

	return (EXIT_ERROR);
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
