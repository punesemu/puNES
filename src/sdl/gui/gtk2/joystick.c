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

void js_init(void) {
	memset(&js1, 0, sizeof(js1));
	memset(&js2, 0, sizeof(js2));

	sprintf(js1.dev, "%s%d", JS_DEV_PATH, port1.joy_id);
	js1.input_port = input_port1;

	sprintf(js2.dev, "%s%d", JS_DEV_PATH, port2.joy_id);
	js2.input_port = input_port2;

	js_open(&js1);
	js_open(&js2);
}
void js_open(_js *joy) {
	joy->fd = 0;
	if (joy->dev[0] && strcmp(joy->dev, "NULL")) {
		joy->fd = open(joy->dev, O_RDONLY | O_NONBLOCK);
		if (joy->fd < 0) {
			joy->fd = 0;
		}
	}
}
void js_control(_js *joy, _port *port) {
	_js_event jse;
	DBWORD value = 0;
	BYTE mode = 0;

	if (!joy->fd) {
		if (++joy->open_try == 300) {
			joy->open_try = 0;
			js_open(joy);
		}
		return;
	}

	if (!js_read_event(&jse, joy)) {
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
				//fprintf(stderr, "released : %s\n\n", jsv_to_name(value));
			} else if (jse.value == 1) {
				mode = PRESSED;
				//fprintf(stderr, "pressed  : %s\n\n", jsv_to_name(value));
			} else {
				value = 0;
			}
		}

		if (value && joy->input_port) {
			joy->input_port(mode, value, JOYSTICK, port);
		}
	}
}
void js_close(_js *joy) {
	if (joy->fd) {
		close(joy->fd);
	}
	joy->fd = 0;
}
void js_quit(void) {
	js_close(&js1);
	js_close(&js2);
}
BYTE js_read_event(_js_event *event, _js *joy) {
	SWORD bytes;
	BYTE size = sizeof(*event);

	if ((bytes = read(joy->fd, event, size)) == size) {
		return (EXIT_OK);
	}

	if (errno == ENODEV) {
		js_close(joy);
	}

	return (EXIT_ERROR);
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
