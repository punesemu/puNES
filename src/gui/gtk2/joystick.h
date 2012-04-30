/*
 * joystick.h
 *
 *  Created on: 03/nov/2011
 *      Author: fhorse
 */

#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include "common.h"
#include "input.h"

#define JSDEVPATH "/dev/input/js"
#define nameToJsv(name) jsFromName(name, jsvlist, LENGTH(jsvlist))
#define nameToJsn(name) jsFromName(name, jsnlist, LENGTH(jsnlist))
#define jsvToName(jsvl) jsToName(jsvl, jsvlist, LENGTH(jsvlist))
#define jsnToName(jsvl) jsToName(jsvl, jsnlist, LENGTH(jsnlist))

enum {
	/* button pressed/released */
	JS_EVENT_BUTTON = 0x01,
	/* joystick moved */
	JS_EVENT_AXIS = 0x02,
	/* initial state of device */
	JS_EVENT_INIT = 0x80
};

typedef struct {
	char dev[30];
	SDBWORD fd;
	WORD openTry;
	SWORD last[16];
	BYTE (*inputPort)(BYTE mode, DBWORD event, BYTE type,
			_port *port);
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
} _jsevent;
typedef struct {
	DBWORD value;
	char name[20];
} _jselement;

_js js1, js2;

static const _jselement jsvlist[] = {
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
static const _jselement jsnlist[] = {
	{ 0x0FF,  "NULL"        },
	{ 0x000,  "JOYSTICKID1" },
	{ 0x001,  "JOYSTICKID2" },
	{ 0x002,  "JOYSTICKID3" },
	{ 0x003,  "JOYSTICKID4" },
};

void jsInit(void);
void jsOpen(_js *joy);
void jsControl(_js *joy, _port *port);
void jsClose(_js *joy);
void jsQuit(void);
BYTE jsReadEvent(_jsevent *jse, _js *joy);
char *jsToName(const DBWORD jsval, const _jselement *jslist, const DBWORD length);
DBWORD jsFromName(const char *jsname, const _jselement *jslist, const DBWORD lenght);

#endif /* JOYSTICK_H_ */
