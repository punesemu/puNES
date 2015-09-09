/*
 * jstick.c
 *
 *  Created on: 03/nov/2011
 *      Author: fhorse
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "jstick.h"
#include "input.h"
#include "gui.h"

#define _js_start(jid, op)\
	if (jid == name_to_jsn("NULL")) {\
		op;\
	}\
	if (!joy->fd) {\
		if (++joy->open_try == 300) {\
			joy->open_try = 0;\
			js_open(joy);\
		}\
		op;\
	}
#define _js_control()\
	if ((jse.type & JS_EVENT_INIT) == JS_EVENT_INIT) {\
		continue;\
	}\
	if (jse.type == JS_EVENT_AXIS) {\
		BYTE axis = jse.number & 0x0F;\
		if ((jse.value < (CENTER + sensibility)) && (jse.value > (CENTER - sensibility))) {\
			jse.value = CENTER;\
		}\
		if ((jse.value < (joy->last_value[axis] + sensibility))\
				&& (jse.value > (joy->last_value[axis] - sensibility))) {\
			continue;\
		}\
		if (jse.value) {\
			mode = PRESSED;\
			value = (jse.number << 1) + 1;\
			if (jse.value > 0) {\
				value++;\
			}\
			joy->last[axis] = value;\
		} else {\
			mode = RELEASED;\
			value = joy->last[axis];\
			joy->last[axis] = 0;\
		}\
		joy->last_value[axis] = jse.value;\
	} else if (jse.type == JS_EVENT_BUTTON) {\
		value = jse.number | 0x400;\
		if (jse.value == 0) {\
			mode = RELEASED;\
		} else if (jse.value == 1) {\
			mode = PRESSED;\
		} else {\
			value = 0;\
		}\
	}

void js_init(void) {
	BYTE i;

	for (i = PORT1; i < PORT_MAX; i++) {
		memset(&js[i], 0x00, sizeof(_js));

		if (port[i].joy_id == name_to_jsn("NULL")) {
			continue;
		}

		sprintf(js[i].dev, "%s%d", JS_DEV_PATH, port[i].joy_id);
		js[i].input_decode_event = input_decode_event[i];
		js_open(&js[i]);
	}
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
	static const SWORD sensibility = (PLUS / 100) * 20;
	_js_event jse;
	DBWORD value = 0;
	BYTE mode = 0;

	_js_start(port->joy_id, return)

	while (!js_read_event(&jse, joy)) {
		_js_control()

		if (value && joy->input_decode_event) {
			joy->input_decode_event(mode, value, JOYSTICK, port);
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
	BYTE i;

	for (i = PORT1; i < PORT_MAX; i++) {
		js_close(&js[i]);
	}
}
BYTE js_is_connected(int dev) {
	char path_dev[30];
	int fd;

	snprintf(path_dev, sizeof(path_dev), "%s%d", JS_DEV_PATH, dev);

	if ((fd = open(path_dev, O_RDONLY | O_NONBLOCK)) < 0) {
		return (EXIT_ERROR);
	}

	close(fd);
	return (EXIT_OK);
}
char *js_name_device(int dev) {
	static char name[128];
	char path_dev[30];
	int fd;

	memset(name, 0x00, sizeof(name));

	snprintf(path_dev, sizeof(path_dev), "%s%d", JS_DEV_PATH, dev);
	fd = open(path_dev, O_RDONLY | O_NONBLOCK);

	if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) < 0) {
		strncpy(name, "Not connected", sizeof(name));
	}

	close(fd);

	return(name);
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
DBWORD js_read_in_dialog(int dev, int fd) {
	static const WORD sensibility = (PLUS / 100) * 75;
	_js_event jse;
	ssize_t size = sizeof(jse);
	DBWORD value = 0;

	memset(&jse, 0x00, size);

	while(read(fd, &jse, size) == size) {
		if (jse.value == CENTER) {
			return (0);
		}
		jse.type &= ~JS_EVENT_INIT;

		if ((jse.type == JS_EVENT_AXIS) && jse.value) {
			if (((jse.value < CENTER) ? (jse.value * -1) : jse.value) > sensibility) {
				value = (jse.number << 1) + 1;
				if (jse.value > CENTER) {
					value++;
				}
			} else {
				continue;
			}
		} else if ((jse.type == JS_EVENT_BUTTON) && jse.value) {
			value = jse.number | 0x400;
		} else {
			continue;
		}
		break;
	}

	return (value);
}
BYTE js_shcut_read(_js_sch *js_sch, _js *joy, int id) {
	static const SWORD sensibility = (PLUS / 100) * 50;
	DBWORD value = 0;
	BYTE mode = 0;
	_js_event jse;

	_js_start(id, return (EXIT_ERROR))

	while (!js_read_event(&jse, joy)) {
		_js_control()

		if (value) {
			js_sch->value = value;
			js_sch->mode = mode;
			return (EXIT_OK);
		}

		break;
	}

	return (EXIT_ERROR);
}
