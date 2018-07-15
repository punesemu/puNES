/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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
#include "conf.h"

#define _js_start(jid, op)\
	if (jid == name_to_jsn(uL("NULL"))) {\
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

static void js_open(_js *joy);
static void js_close(_js *joy);

void js_init(BYTE first_time) {
	BYTE i;

	for (i = PORT1; i < PORT_MAX; i++) {
		memset(&js[i], 0x00, sizeof(_js));

		if (port[i].joy_id == name_to_jsn(uL("NULL"))) {
			continue;
		}

		usnprintf(js[i].dev, usizeof(js[i].dev), uL("" JS_DEV_PATH "%d"), port[i].joy_id);
		js[i].input_decode_event = port_funct[i].input_decode_event;
		js_open(&js[i]);
	}
}
void js_quit(BYTE last_time) {
	BYTE i;

	for (i = PORT1; i < PORT_MAX; i++) {
		js_close(&js[i]);
	}
}
void js_update_detected_devices(void) {}
void js_control(_js *joy, _port *port) {
	static const SWORD sensibility = (PLUS / 100) * 20;
	_js_event jse;
	DBWORD value = 0;
	BYTE mode = 0;

	_js_start(port->joy_id, return)

	while (!js_read_event(&jse, joy)) {
		_js_control()

		if (value && joy->input_decode_event) {
			joy->input_decode_event(mode, FALSE, value, JOYSTICK, port);
		}
	}
}

BYTE js_is_connected(int dev) {
	uTCHAR path_dev[30];
	int fd;

	usnprintf(path_dev, usizeof(path_dev), uL("" JS_DEV_PATH "%d"), dev);

	if ((fd = uopen(path_dev, O_RDONLY | O_NONBLOCK)) < 0) {
		return (EXIT_ERROR);
	}

	close(fd);
	return (EXIT_OK);
}
BYTE js_is_this(BYTE dev, BYTE *id) {
	return (dev == (*id));
}
BYTE js_is_null(BYTE *id) {
	return ((*id) == name_to_jsn(uL("NULL")));
}
void js_set_id(BYTE *id, int dev) {
	(*id) = dev;
}
uTCHAR *js_name_device(int dev) {
	static uTCHAR name[128];
	uTCHAR path_dev[30];
	int fd;

	umemset(name, 0x00, usizeof(name));

	usnprintf(path_dev, usizeof(path_dev), uL("" JS_DEV_PATH "%d"), dev);
	fd = uopen(path_dev, O_RDONLY | O_NONBLOCK);

	if (uioctl(fd, JSIOCGNAME(sizeof(name)), name) < 0) {
		ustrncpy(name, uL("Not connected"), usizeof(name));
	}

	close(fd);

	return((uTCHAR *) name);
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
uTCHAR *js_to_name(const DBWORD val, const _js_element *list, const DBWORD length) {
	BYTE index;
	static uTCHAR str[20];

	umemset(str, 0x00, usizeof(str));

	for (index = 0; index < length; index++) {
		if (val == list[index].value) {
			ustrncpy(str, list[index].name, usizeof(str));
			return ((uTCHAR *) str);
		}
	}
	return ((uTCHAR *) list[0].name);
}
DBWORD js_from_name(const uTCHAR *name, const _js_element *list, const DBWORD length) {
	DBWORD js = 0;
	BYTE index;

	for (index = 0; index < length; index++) {
		if (!ustrcmp(name, list[index].name)) {
			return (list[index].value);
		}
	}
	return (js);
}
DBWORD js_read_in_dialog(BYTE *id, int fd) {
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

void js_shcut_init(void) {
	memset(&js_shcut, 0x00, sizeof(_js));
	usnprintf(js_shcut.dev, usizeof(js_shcut.dev), uL("" JS_DEV_PATH "%d"), cfg->input.shcjoy_id);
	if ((js_shcut.fd = open(js_shcut.dev, O_RDONLY | O_NONBLOCK)) == -1) {
		js_shcut.fd = 0;
	}
}
void js_shcut_stop(void) {
	if (js_shcut.fd) {
		close(js_shcut.fd);
		js_shcut.fd = 0;
	}
}
BYTE js_shcut_read(_js_sch *js_sch) {
	static const SWORD sensibility = (PLUS / 100) * 50;
	DBWORD value = 0;
	BYTE mode = 0;
	_js_event jse;
	_js *joy = &js_shcut;

	_js_start(cfg->input.shcjoy_id, return (EXIT_ERROR))

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

static void js_open(_js *joy) {
	joy->fd = 0;
	if (joy->dev[0] && ustrcmp(joy->dev, uL("NULL"))) {
		joy->fd = uopen(joy->dev, O_RDONLY | O_NONBLOCK);
		if (joy->fd < 0) {
			joy->fd = 0;
		}
	}
}
static void js_close(_js *joy) {
	if (joy->fd) {
		close(joy->fd);
	}
	joy->fd = 0;
}
