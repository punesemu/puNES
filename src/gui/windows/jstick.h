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

typedef struct _js_special_case {
	struct _axis_with_CENTER_equal_to_0 {
		BYTE X;
		BYTE Y;
		BYTE Z;
		BYTE R;
		BYTE U;
		BYTE V;
	} axis_with_CENTER_equal_to_0;
} _js_special_case;
typedef struct _js {
	uTCHAR dev[30];
	DBWORD id;
	BYTE present;

	BYTE open_try;
	BYTE clock;

	DWORD last_axis[MAXAXIS];
	DWORD last_buttons;

	JOYINFOEX joy_info;
	JOYCAPS joy_caps;

	BYTE (*input_decode_event)(BYTE mode, DBWORD event, BYTE type, _port *port);

	_js_special_case jsc;
} _js;
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
	{ 0xFF,         uL("NULL")         },
	{ JOYSTICKID1,  uL("JOYSTICKID1")  },
	{ JOYSTICKID2,  uL("JOYSTICKID2")  },
	{ JOYSTICKID3,  uL("JOYSTICKID3")  },
	{ JOYSTICKID4,  uL("JOYSTICKID4")  },
	{ JOYSTICKID5,  uL("JOYSTICKID5")  },
	{ JOYSTICKID6,  uL("JOYSTICKID6")  },
	{ JOYSTICKID7,  uL("JOYSTICKID7")  },
	{ JOYSTICKID8,  uL("JOYSTICKID8")  },
	{ JOYSTICKID9,  uL("JOYSTICKID9")  },
	{ JOYSTICKID10, uL("JOYSTICKID10") },
	{ JOYSTICKID11, uL("JOYSTICKID11") },
	{ JOYSTICKID12, uL("JOYSTICKID12") },
	{ JOYSTICKID13, uL("JOYSTICKID13") },
	{ JOYSTICKID14, uL("JOYSTICKID14") },
	{ JOYSTICKID15, uL("JOYSTICKID15") }
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
	{ 0x100, uL("JPOVF")  }, { 0x101, uL("JPOVR")  },
	{ 0x102, uL("JPOVB")  }, { 0x103, uL("JPOVL")  },
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
	{ 0x418, uL("JB24")   }, { 0x419, uL("JB25")   },
	{ 0x41A, uL("JB26")   }, { 0x41B, uL("JB27")   },
	{ 0x41C, uL("JB28")   }, { 0x41D, uL("JB29")   },
	{ 0x41E, uL("JB30")   }, { 0x41F, uL("JB31")   }
};

EXTERNC _js js[PORT_MAX];

EXTERNC void js_init(void);
EXTERNC void js_open(_js *joy);
EXTERNC void js_control(_js *joy, _port *port);
EXTERNC void js_close(_js *joy);
EXTERNC void js_quit(void);
EXTERNC BYTE js_is_connected(int dev);
EXTERNC uTCHAR *js_name_device(int dev);
EXTERNC uTCHAR *js_to_name(const DBWORD val, const _js_element *list, const DBWORD length);
EXTERNC DBWORD js_from_name(const uTCHAR *name, const _js_element *list, const DBWORD lenght);
EXTERNC DBWORD js_read_in_dialog(int dev, int fd);
EXTERNC BYTE js_shcut_read(_js_sch *js_sch, _js *joy, int id);

#undef EXTERNC

#endif /* JSTICK_H_ */
