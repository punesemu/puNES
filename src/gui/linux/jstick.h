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

#ifndef JSTICK_H_
#define JSTICK_H_

#include <linux/joystick.h>
#include "common.h"
#include "input.h"

#define JS_DEV_PATH "/dev/input/js"
#define name_to_jsv(name) js_from_name(name, jsv_list, LENGTH(jsv_list))
#define name_to_jsn(name) js_from_name(name, jsn_list, LENGTH(jsn_list))
#define jsv_to_name(jsvl) js_to_name(jsvl, jsv_list, LENGTH(jsv_list))
#define jsn_to_name(jsvl) js_to_name(jsvl, jsn_list, LENGTH(jsn_list))

enum {
	CENTER,
	PLUS = 0x7FFF
};

typedef struct _js {
	uTCHAR dev[30];
	SDBWORD fd;
	WORD open_try;
	SWORD last[16];
	SWORD last_value[16];
	BYTE (*input_decode_event)(BYTE mode, DBWORD event, BYTE type, _port *port);
} _js;
typedef struct _js_event {
	/* event timestamp in milliseconds */
	DBWORD time;
	/* value */
	SWORD value;
	/* event type */
	BYTE type;
	/* axis/button number */
	BYTE number;
} _js_event;
typedef struct _js_element {
	DBWORD value;
	uTCHAR name[20];
} _js_element;
typedef struct _js_sch {
	DBWORD value;
	BYTE mode;
} _js_sch;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

static const _js_element jsn_list[] = {
	{ 0x0FF,  uL("NULL")        },
	{ 0x000,  uL("JOYSTICKID1") },
	{ 0x001,  uL("JOYSTICKID2") },
	{ 0x002,  uL("JOYSTICKID3") },
	{ 0x003,  uL("JOYSTICKID4") },
};
static const _js_element jsv_list[] = {
	{ 0x000, uL("NULL")   },
	{ 0x001, uL("JA0MIN") }, { 0x002, uL("JA0PLS") },
	{ 0x003, uL("JA1MIN") }, { 0x004, uL("JA1PLS") },
	{ 0x005, uL("JA2MIN") }, { 0x006, uL("JA2PLS") },
	{ 0x007, uL("JA3MIN") }, { 0x008, uL("JA3PLS") },
	{ 0x009, uL("JA4MIN") }, { 0x00A, uL("JA4PLS") },
	{ 0x00B, uL("JA5MIN") }, { 0x00C, uL("JA5PLS") },
	{ 0x00D, uL("JA6MIN") }, { 0x00E, uL("JA6PLS") },
	{ 0x00F, uL("JA7MIN") }, { 0x010, uL("JA7PLS") },
	{ 0x011, uL("JA8MIN") }, { 0x012, uL("JA8PLS") },
	{ 0x013, uL("JA9MIN") }, { 0x014, uL("JA9PLS") },
	{ 0x400, uL("JB0")    }, { 0x401, uL("JB1")    },
	{ 0x402, uL("JB2")    }, { 0x403, uL("JB3")    },
	{ 0x404, uL("JB4")    }, { 0x405, uL("JB5")    },
	{ 0x406, uL("JB6")    }, { 0x407, uL("JB7")    },
	{ 0x408, uL("JB8")    }, { 0x409, uL("JB9")    },
	{ 0x40A, uL("JB10")   }, { 0x40B, uL("JB11")   },
	{ 0x40C, uL("JB12")   }, { 0x40D, uL("JB13")   },
	{ 0x40E, uL("JB14")   }, { 0x40F, uL("JB15")   },
	{ 0x410, uL("JB16")   }, { 0x411, uL("JB17")   },
	{ 0x412, uL("JB18")   }, { 0x413, uL("JB19")   },
	{ 0x414, uL("JB20")   }, { 0x415, uL("JB21")   },
	{ 0x416, uL("JB22")   }, { 0x417, uL("JB23")   },
};

EXTERNC _js js[PORT_MAX], js_shcut;;

EXTERNC void js_init(BYTE first_time);
EXTERNC void js_quit(BYTE last_time);
EXTERNC void js_update_detected_devices(void);
EXTERNC void js_control(_js *joy, _port *port);

EXTERNC BYTE js_is_connected(int dev);
EXTERNC BYTE js_is_this(BYTE dev, BYTE *id);
EXTERNC BYTE js_is_null(BYTE *id);
EXTERNC void js_set_id(BYTE *id, int dev);
EXTERNC uTCHAR *js_name_device(int dev);
EXTERNC BYTE js_read_event(_js_event *event, _js *joy);
EXTERNC uTCHAR *js_to_name(const DBWORD val, const _js_element *list, const DBWORD length);
EXTERNC DBWORD js_from_name(const uTCHAR *name, const _js_element *list, const DBWORD lenght);
EXTERNC DBWORD js_read_in_dialog(BYTE *id, int fd);

EXTERNC void js_shcut_init(void);
EXTERNC void js_shcut_stop(void);
EXTERNC BYTE js_shcut_read(_js_sch *js_sch);

#undef EXTERNC

#endif /* JSTICK_H_ */
