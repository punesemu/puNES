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

#ifndef JSTICK_H_
#define JSTICK_H_

#include "gui.h"
#include "input.h"

#define name_to_jsv(name) js_from_name(name, jsv_list, LENGTH(jsv_list))
#define name_to_jsn(name) js_from_name(name, jsn_list, LENGTH(jsn_list))
#define jsv_to_name(jsvl) js_to_name(jsvl, jsv_list, LENGTH(jsv_list))
#define jsn_to_name(jsvl) js_to_name(jsvl, jsn_list, LENGTH(jsn_list))

enum { X, Y, Z, R, U, V, POV, MAXAXIS };
enum {
	JOYSTICKID3 = JOYSTICKID2 + 1,
	JOYSTICKID4,
	JOYSTICKID5,
	JOYSTICKID6,
	JOYSTICKID7,
	JOYSTICKID8,
	JOYSTICKID9,
	JOYSTICKID10,
	JOYSTICKID11,
	JOYSTICKID12,
	JOYSTICKID13,
	JOYSTICKID14,
	JOYSTICKID15
};
enum {
	MINUS,
	CENTER = 0x7FFF,
	PLUS = 0xFFFF
};
enum { MAX_BUTTONS = 32 };

typedef struct {
	char dev[30];
	DBWORD id;
	BYTE present;
	BYTE open_try;
	BYTE clock;
	DWORD last_axis[MAXAXIS];
	DWORD last_buttons;
	JOYINFOEX joy_info;
	JOYCAPS joy_caps;
	BYTE (*input_decode_event)(BYTE mode, DBWORD event, BYTE type, _port *port);
} _js;
typedef struct {
	DBWORD value;
	char name[20];
} _js_element;
typedef struct {
	DBWORD value;
	BYTE mode;
} _js_sch;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

static const _js_element jsn_list[] = {
	{ 0xFF,         "NULL"         },
	{ JOYSTICKID1,  "JOYSTICKID1"  },
	{ JOYSTICKID2,  "JOYSTICKID2"  },
	{ JOYSTICKID3,  "JOYSTICKID3"  },
	{ JOYSTICKID4,  "JOYSTICKID4"  },
	{ JOYSTICKID5,  "JOYSTICKID5"  },
	{ JOYSTICKID6,  "JOYSTICKID6"  },
	{ JOYSTICKID7,  "JOYSTICKID7"  },
	{ JOYSTICKID8,  "JOYSTICKID8"  },
	{ JOYSTICKID9,  "JOYSTICKID9"  },
	{ JOYSTICKID10, "JOYSTICKID10" },
	{ JOYSTICKID11, "JOYSTICKID11" },
	{ JOYSTICKID12, "JOYSTICKID12" },
	{ JOYSTICKID13, "JOYSTICKID13" },
	{ JOYSTICKID14, "JOYSTICKID14" },
	{ JOYSTICKID15, "JOYSTICKID15" }
};
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
	{ 0x100, "JPOVF"  }, { 0x101, "JPOVR"  },
	{ 0x102, "JPOVB"  }, { 0x103, "JPOVL"  },
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
	{ 0x418, "JB24"   }, { 0x419, "JB25"   },
	{ 0x41A, "JB26"   }, { 0x41B, "JB27"   },
	{ 0x41C, "JB28"   }, { 0x41D, "JB29"   },
	{ 0x41E, "JB30"   }, { 0x41F, "JB31"   }
};

EXTERNC _js js[PORT_MAX];

EXTERNC void js_init(void);
EXTERNC void js_open(_js *joy);
EXTERNC void js_control(_js *joy, _port *port);
EXTERNC void js_close(_js *joy);
EXTERNC void js_quit(void);
EXTERNC BYTE js_is_connected(int dev);
EXTERNC char *js_name_device(int index);
EXTERNC char *js_to_name(const DBWORD val, const _js_element *list, const DBWORD length);
EXTERNC DBWORD js_from_name(const char *name, const _js_element *list, const DBWORD lenght);
EXTERNC DBWORD js_read_in_dialog(int dev, int fd);
EXTERNC BYTE js_shcut_read(_js_sch *js_sch, _js *joy, int id);

#undef EXTERNC

#endif /* JSTICK_H_ */
