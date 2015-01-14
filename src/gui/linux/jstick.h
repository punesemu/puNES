/*
 * jstick.h
 *
 *  Created on: 03/nov/2011
 *      Author: fhorse
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

typedef struct {
	char dev[30];
	SDBWORD fd;
	WORD open_try;
	SWORD last[16];
	SWORD last_value[16];
	BYTE (*input_decode_event)(BYTE mode, DBWORD event, BYTE type, _port *port);
} _js;
typedef struct {
	/* event timestamp in milliseconds */
	DBWORD time;
	/* value */
	SWORD value;
	/* event type */
	BYTE type;
	/* axis/button number */
	BYTE number;
} _js_event;
typedef struct {
	DBWORD value;
	char name[20];
} _js_element;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

static const _js_element jsv_list[] = {
	{ 0x000, "NULL"   },
	{ 0x001, "JA0MIN" }, { 0x002, "JA0PLS" },
	{ 0x003, "JA1MIN" }, { 0x004, "JA1PLS" },
	{ 0x005, "JA2MIN" }, { 0x006, "JA2PLS" },
	{ 0x007, "JA3MIN" }, { 0x008, "JA3PLS" },
	{ 0x009, "JA4MIN" }, { 0x00A, "JA4PLS" },
	{ 0x00B, "JA5MIN" }, { 0x00C, "JA5PLS" },
	{ 0x00D, "JA6MIN" }, { 0x00E, "JA6PLS" },
	{ 0x00F, "JA7MIN" }, { 0x010, "JA7PLS" },
	{ 0x011, "JA8MIN" }, { 0x012, "JA8PLS" },
	{ 0x013, "JA9MIN" }, { 0x014, "JA9PLS" },
	{ 0x400, "JB0"    }, { 0x401, "JB1"    },
	{ 0x402, "JB2"    }, { 0x403, "JB3"    },
	{ 0x404, "JB4"    }, { 0x405, "JB5"    },
	{ 0x406, "JB6"    }, { 0x407, "JB7"    },
	{ 0x408, "JB8"    }, { 0x409, "JB9"    },
	{ 0x40A, "JB10"   }, { 0x40B, "JB11"   },
	{ 0x40C, "JB12"   }, { 0x40D, "JB13"   },
	{ 0x40E, "JB14"   }, { 0x40F, "JB15"   },
	{ 0x410, "JB16"   }, { 0x411, "JB17"   },
	{ 0x412, "JB18"   }, { 0x413, "JB19"   },
	{ 0x414, "JB20"   }, { 0x415, "JB21"   },
	{ 0x416, "JB22"   }, { 0x417, "JB23"   },
};
static const _js_element jsn_list[] = {
	{ 0x0FF,  "NULL"        },
	{ 0x000,  "JOYSTICKID1" },
	{ 0x001,  "JOYSTICKID2" },
	{ 0x002,  "JOYSTICKID3" },
	{ 0x003,  "JOYSTICKID4" },
};

EXTERNC _js js[PORT_MAX];

EXTERNC void js_init(void);
EXTERNC void js_open(_js *joy);
EXTERNC void js_control(_js *joy, _port *port);
EXTERNC void js_close(_js *joy);
EXTERNC void js_quit(void);
EXTERNC BYTE js_is_connected(int dev);
EXTERNC char *js_name_device(int dev);
EXTERNC BYTE js_read_event(_js_event *event, _js *joy);
EXTERNC char *js_to_name(const DBWORD val, const _js_element *list, const DBWORD length);
EXTERNC DBWORD js_from_name(const char *name, const _js_element *list, const DBWORD lenght);
EXTERNC int js_read_in_dialog(int dev, int *dt, DBWORD *value, int max_joystick);

#undef EXTERNC

#endif /* JSTICK_H_ */
