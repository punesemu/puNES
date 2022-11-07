/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#include "common.h"
#include "input.h"
#include "thread_def.h"
#include "jstick_db.h"

enum _js_misc {
	JS_MS_UPDATE_DETECT_DEVICE = 1000,
	JS_MS_UPDATE_DETECT_DEVICE_DLG = 1500,
	JS_MAX_AXES = 16,
	JS_MAX_HATS = 4,
	JS_MAX_BUTTONS = 64,
	JS_AXIS_MIN = -32767,
	JS_AXIS_MAX = 32767,
	JS_NO_JOYSTICK = 0xFF,
	JS_ABS_FIRST_HAT = ABS_HAT0X,
	JS_ABS_LAST_HAT = ABS_HAT3Y
};
enum _js_button_enum {
	JS_BUTTON_A,
	JS_BUTTON_B,
	JS_BUTTON_X,
	JS_BUTTON_Y,
	JS_BUTTON_LEFT_SHOULDER,
	JS_BUTTON_RIGHT_SHOULDER,
	JS_BUTTON_BACK,
	JS_BUTTON_START,
	JS_BUTTON_GUIDE,
	JS_BUTTON_LEFT_THUMB,
	JS_BUTTON_RIGHT_THUMB,
	JS_BUTTON_11,
	JS_BUTTON_DPAD_UP,
	JS_BUTTON_DPAD_DOWN,
	JS_BUTTON_DPAD_LEFT,
	JS_BUTTON_DPAD_RIGHT
};
enum _js_axis_enum {
	JS_AXIS_X,
	JS_AXIS_Y,
	JS_AXIS_Z,
	JS_AXIS_RX,
	JS_AXIS_RY,
	JS_AXIS_RZ,
	JS_AXIS_SLIDER,
	JS_AXIS_WHEEL,
	JS_HAT0_A,
	JS_HAT0_B,
	JS_HAT1_A,
	JS_HAT1_B,
	JS_HAT2_A,
	JS_HAT2_B,
	JS_HAT3_A,
	JS_HAT3_B,
};

typedef struct _js_axs_joyval {
	DBWORD value;
	DBWORD offset;
	uTCHAR desc[3][30];
} _js_axs_joyval;
typedef struct _js_btn_joyval {
	DBWORD value;
	DBWORD offset;
	uTCHAR desc[30];
} _js_btn_joyval;
typedef struct _js_sch {
	BYTE mode;
	DBWORD value;
} _js_sch;
typedef struct _js_info {
	int axes;
	int buttons;
	int hats;
#if defined (_WIN32)
	int sliders;
#endif
} _js_info;
typedef struct _js_axis {
	BYTE used;
	BYTE enabled;
	float min;
	float max;
	float scale;
	float center;
	SWORD value;
	DBWORD offset;
	BYTE is_hat;
#if defined (_WIN32)
	DBWORD offset_di8;
#endif
	// thx to the developer of SDL for this
	struct _js_axs_validate {
		BYTE sent_initial_value;
		BYTE has_initial_value;
		BYTE has_second_value;
		float initial_value;
		float second_value;
	} validate;
} _js_axis;
typedef struct _js_button {
	BYTE used;
	BYTE enabled;
	BYTE value;
	DBWORD offset;
#if defined (_WIN32)
	DBWORD offset_di8;
#endif
} _js_button;
typedef struct _js_data {
	_js_axis axis[JS_MAX_AXES];
	_js_axis hat[JS_MAX_HATS * 2];
	_js_button button[JS_MAX_BUTTONS];
} _js_data;
typedef struct _js_last_states {
	float axis[JS_MAX_AXES];
	float hats[JS_MAX_HATS * 2];
	BYTE button[JS_MAX_BUTTONS];
} _js_last_states;
typedef struct _js_device {
	// dipendenti dall'os
#if defined (_WIN32)
	GUID product_guid;
	// xinput
	unsigned int xinput_player_index;
	// dinput
	void *di8device;
	BYTE buffered;
#else
	int fd;
	uTCHAR dev[30];
#if defined (__OpenBSD__) || defined (__FreeBSD__)
	SDBWORD hug_d_pad_state;
	struct report_desc *repdesc;
	struct _js_report {
		int id;
#if defined (__FreeBSD__)
		void *buf;
#else
		struct usb_ctl_report *buf;
#endif
		int size;
	} report;
#endif
#endif
	// comuni
	enum _js_gamepad_type type;
	uTCHAR desc[128];
	_js_info info;
	_js_data data;
	_input_guid guid;
	struct _js_usb_info {
		WORD bustype;
		WORD vendor_id;
		WORD product_id;
		WORD version;
	} usb;
	struct _js_device_port {
		_js_last_states last_state;
		_js_last_states last_value;
	} port;
	struct _js_device_shcut {
		_js_last_states last_state;
		_js_last_states last_value;
	} shcut;

	BYTE is_xinput;

	// non devono essere inizializzate nel js_os_jdev_init()
	BYTE present;
	BYTE index;
	thread_mutex_t lock;
	thread_t thread;
	int deadzone;
	// standard controller
	DBWORD stdctrl[MAX_STD_PAD_BUTTONS];
} _js_device;
typedef struct _js {
	BYTE inited;
	thread_mutex_t lock;
	_input_guid guid;
	void *jdev;
	BYTE (*input_decode_event)(BYTE mode, BYTE autorepeat, DBWORD event, BYTE type, _port *prt);
} _js;
typedef struct _jstick {
	thread_t thread;
	double last_control;
	struct _js_devices_detected {
		int count;
		_js_device devices[MAX_JOYSTICK];
	} jdd;
} _jstick;

static const unsigned int js_axs_type[] = { JS_MAX_AXES, JS_MAX_HATS };
static const _js_axs_joyval js_axs_joyval[] = {
	{ 0x0FF, 0xFFFFFF,            { uL("NULL"),         uL("NULL"),         uL("NULL")           } },
	{ 0x002, ABS_X,               { uL("AXS X MIN"),    uL("AXS X PLS"),    uL("ABS_X")          } },
	{ 0x004, ABS_Y,               { uL("AXS Y MIN"),    uL("AXS Y PLS"),    uL("ABS_Y")          } },
	{ 0x006, ABS_Z,               { uL("AXS Z MIN"),    uL("AXS Z PLS"),    uL("ABS_Z")          } },
	{ 0x008, ABS_RX,              { uL("AXS RX MIN"),   uL("AXS RX PLS"),   uL("ABS_RX")         } },
	{ 0x00A, ABS_RY,              { uL("AXS RY MIN"),   uL("AXS RY PLS"),   uL("ABS_RY")         } },
	{ 0x00C, ABS_RZ,              { uL("AXS RZ MIN"),   uL("AXS RZ PLS"),   uL("ABS_RZ")         } },
	{ 0x00E, ABS_THROTTLE,        { uL("THROTTLE MIN"), uL("THROTTLE PLS"), uL("ABS_THROTTLE")   } },
	{ 0x010, ABS_WHEEL,           { uL("WHEEL MIN"),    uL("WHEEL PLS"),    uL("ABS_WHEEL")      } },
	{ 0x012, ABS_HAT0X,           { uL("HAT0X MIN"),    uL("HAT0X PLS"),    uL("ABS_HAT0X")      } },
	{ 0x014, ABS_HAT0Y,           { uL("HAT0Y MIN"),    uL("HAT0Y PLS"),    uL("ABS_HAT0Y")      } },
	{ 0x016, ABS_HAT1X,           { uL("HAT1X MIN"),    uL("HAT1X PLS"),    uL("ABS_HAT1X")      } },
	{ 0x018, ABS_HAT1Y,           { uL("HAT1Y MIN"),    uL("HAT1Y PLS"),    uL("ABS_HAT1Y")      } },
	{ 0x01A, ABS_HAT2X,           { uL("HAT2X MIN"),    uL("HAT2X PLS"),    uL("ABS_HAT2X")      } },
	{ 0x01C, ABS_HAT2Y,           { uL("HAT2Y MIN"),    uL("HAT2Y PLS"),    uL("ABS_HAT2Y")      } },
	{ 0x01E, ABS_HAT3X,           { uL("HAT3X MIN"),    uL("HAT3X PLS"),    uL("ABS_HAT3X")      } },
	{ 0x020, ABS_HAT3Y,           { uL("HAT3Y MIN"),    uL("HAT3Y PLS"),    uL("ABS_HAT3Y")      } },
	{ 0x022, ABS_RUDDER,          { uL("RUDDER MIN"),   uL("RUDDER PLS"),   uL("ABS_RUDDER")     } },
	{ 0x024, ABS_GAS,             { uL("GAS MIN"),      uL("GAS PLS"),      uL("ABS_GAS")        } },
	{ 0x026, ABS_BRAKE,           { uL("BRAKE MIN"),    uL("BRAKE PLS"),    uL("ABS_BRAKE")      } },
	{ 0x028, ABS_PRESSURE,        { uL("PRESSURE MIN"), uL("PRESSURE PLS"), uL("ABS_PRESSURE")   } },
	{ 0x02A, ABS_DISTANCE,        { uL("DISTANCE MIN"), uL("DISTANCE PLS"), uL("ABS_DISTANCE")   } },
	{ 0x02C, ABS_TILT_X,          { uL("TILT X MIN"),   uL("TILT X PLS"),   uL("ABS_TILT_X")     } },
	{ 0x02E, ABS_TILT_Y,          { uL("TILT Y MIN"),   uL("TILT Y PLS"),   uL("ABS_TILT_Y")     } },
	{ 0x030, ABS_TOOL_WIDTH,      { uL("TWIDTH MIN"),   uL("TWIDTH PLS"),   uL("ABS_TOOL_WIDTH") } }
};
static const _js_btn_joyval js_btn_joyval[] = {
	{ 0x000, 0x000,               uL("NULL")  },
	{ 0x400, BTN_A,               uL("BTN01") },
	{ 0x401, BTN_B,               uL("BTN02") },
	{ 0x402, BTN_C,               uL("BTN03") },
	{ 0x403, BTN_X,               uL("BTN04") },
	{ 0x404, BTN_Y,               uL("BTN05") },
	{ 0x405, BTN_Z,               uL("BTN06") },
	{ 0x406, BTN_TL,              uL("BTN07") },
	{ 0x407, BTN_TR,              uL("BTN08") },
	{ 0x408, BTN_TL2,             uL("BTN09") },
	{ 0x409, BTN_TR2,             uL("BTN10") },
	{ 0x40A, BTN_SELECT,          uL("BTN11") },
	{ 0x40B, BTN_START,           uL("BTN12") },
	{ 0x40C, BTN_MODE,            uL("BTN13") },
	{ 0x40D, BTN_THUMBL,          uL("BTN14") },
	{ 0x40E, BTN_THUMBR,          uL("BTN15") },
	{ 0x40F, BTN_DPAD_UP,         uL("BTN16") },
	{ 0x410, BTN_DPAD_DOWN,       uL("BTN17") },
	{ 0x411, BTN_DPAD_LEFT,       uL("BTN18") },
	{ 0x412, BTN_DPAD_RIGHT,      uL("BTN19") },
	{ 0x413, BTN_TRIGGER,         uL("BTN20") },
	{ 0x414, BTN_THUMB,           uL("BTN21") },
	{ 0x415, BTN_THUMB2,          uL("BTN22") },
	{ 0x416, BTN_TOP,             uL("BTN23") },
	{ 0x417, BTN_TOP2,            uL("BTN24") },
	{ 0x418, BTN_PINKIE,          uL("BTN25") },
	{ 0x419, BTN_BASE,            uL("BTN26") },
	{ 0x41A, BTN_BASE2,           uL("BTN27") },
	{ 0x41B, BTN_BASE3,           uL("BTN28") },
	{ 0x41C, BTN_BASE4,           uL("BTN29") },
	{ 0x41D, BTN_BASE5,           uL("BTN30") },
	{ 0x41E, BTN_BASE6,           uL("BTN31") },
	{ 0x41F, BTN_DEAD,            uL("BTN32") },
	{ 0x420, BTN_GEAR_DOWN,       uL("BTN33") },
	{ 0x421, BTN_GEAR_UP,         uL("BTN34") },
	{ 0x422, BTN_TRIGGER_HAPPY,   uL("BTN35") },
	{ 0x423, BTN_TRIGGER_HAPPY1,  uL("BTN36") },
	{ 0x424, BTN_TRIGGER_HAPPY2,  uL("BTN37") },
	{ 0x425, BTN_TRIGGER_HAPPY3,  uL("BTN38") },
	{ 0x426, BTN_TRIGGER_HAPPY4,  uL("BTN39") },
	{ 0x427, BTN_TRIGGER_HAPPY5,  uL("BTN40") },
	{ 0x428, BTN_TRIGGER_HAPPY6,  uL("BTN41") },
	{ 0x429, BTN_TRIGGER_HAPPY7,  uL("BTN42") },
	{ 0x42A, BTN_TRIGGER_HAPPY8,  uL("BTN43") },
	{ 0x42B, BTN_TRIGGER_HAPPY9,  uL("BTN44") },
	{ 0x42C, BTN_TRIGGER_HAPPY10, uL("BTN45") },
	{ 0x42D, BTN_TRIGGER_HAPPY11, uL("BTN46") },
	{ 0x42E, BTN_TRIGGER_HAPPY12, uL("BTN47") },
	{ 0x42F, BTN_TRIGGER_HAPPY13, uL("BTN48") },
	{ 0x430, BTN_TRIGGER_HAPPY14, uL("BTN49") },
	{ 0x431, BTN_TRIGGER_HAPPY15, uL("BTN50") },
	{ 0x432, BTN_TRIGGER_HAPPY16, uL("BTN51") },
	{ 0x433, BTN_TRIGGER_HAPPY17, uL("BTN52") },
	{ 0x434, BTN_TRIGGER_HAPPY18, uL("BTN53") },
	{ 0x435, BTN_TRIGGER_HAPPY19, uL("BTN54") },
	{ 0x436, BTN_TRIGGER_HAPPY20, uL("BTN55") },
	{ 0x437, BTN_TRIGGER_HAPPY21, uL("BTN56") },
	{ 0x438, BTN_TRIGGER_HAPPY22, uL("BTN57") },
	{ 0x439, BTN_TRIGGER_HAPPY23, uL("BTN58") },
	{ 0x43A, BTN_TRIGGER_HAPPY24, uL("BTN59") },
	{ 0x43B, BTN_TRIGGER_HAPPY25, uL("BTN60") },
	{ 0x43C, BTN_TRIGGER_HAPPY26, uL("BTN61") },
	{ 0x43D, BTN_TRIGGER_HAPPY27, uL("BTN62") },
	{ 0x43E, BTN_TRIGGER_HAPPY28, uL("BTN63") },
	{ 0x43F, BTN_TRIGGER_HAPPY29, uL("BTN64") },
	{ 0x440, BTN_TRIGGER_HAPPY30, uL("BTN65") },
	{ 0x441, BTN_TRIGGER_HAPPY31, uL("BTN66") },
	{ 0x442, BTN_TRIGGER_HAPPY32, uL("BTN67") },
	{ 0x443, BTN_TRIGGER_HAPPY33, uL("BTN68") },
	{ 0x444, BTN_TRIGGER_HAPPY34, uL("BTN69") },
	{ 0x445, BTN_TRIGGER_HAPPY35, uL("BTN70") },
	{ 0x446, BTN_TRIGGER_HAPPY36, uL("BTN71") },
	{ 0x447, BTN_TRIGGER_HAPPY37, uL("BTN72") },
	{ 0x448, BTN_TRIGGER_HAPPY38, uL("BTN73") },
	{ 0x449, BTN_TRIGGER_HAPPY39, uL("BTN74") },
	{ 0x44A, BTN_TRIGGER_HAPPY40, uL("BTN75") }
};

extern _js js[PORT_MAX], js_shcut;
extern _jstick jstick;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void js_init(BYTE first_time);
EXTERNC void js_quit(BYTE last_time);
EXTERNC void js_reset(_js *js);

EXTERNC void js_init_shcut(void);

EXTERNC BYTE js_is_connected(int index);
EXTERNC BYTE js_is_this(int index, _input_guid *guid);
EXTERNC BYTE js_is_null(_input_guid *guid);

#if !defined (_WIN32)
EXTERNC void js_guid_create(_js_device *jdev);
#endif
EXTERNC void js_guid_set(int index, _input_guid *guid);
EXTERNC void js_guid_unset(_input_guid *guid);
EXTERNC BYTE js_guid_cmp(_input_guid *guid1, _input_guid *guid2);
EXTERNC uTCHAR *js_guid_to_string(_input_guid *guid);
EXTERNC void js_guid_from_string(_input_guid *guid, uTCHAR *string);

EXTERNC uTCHAR *js_axs_joyoffset_to_name(DBWORD offset);
EXTERNC uTCHAR *js_axs_joyval_to_name(DBWORD value);
EXTERNC DBWORD js_axs_joyval_from_name(const uTCHAR *name);
EXTERNC DBWORD js_axs_joyval_to_joyoffset(DBWORD value);
EXTERNC DBWORD js_axs_joyval_from_joyoffset(DBWORD offset);
EXTERNC void js_axs_validate(_js_axis *jsx, SDBWORD value);

EXTERNC uTCHAR *js_btn_joyoffset_to_name(DBWORD offset);
EXTERNC uTCHAR *js_btn_joyval_to_name(DBWORD value);
EXTERNC DBWORD js_btn_joyval_from_name(const uTCHAR *name);
EXTERNC DBWORD js_btn_joyval_to_joyoffset(DBWORD value);
EXTERNC DBWORD js_btn_joyval_from_joyoffset(DBWORD offset);

EXTERNC uTCHAR *js_joyval_to_name(DBWORD value);
EXTERNC DBWORD js_joyval_from_name(const uTCHAR *name);
EXTERNC DBWORD js_joyval_to_joyoffset(DBWORD value);

EXTERNC DBWORD js_joyval_default(int index, int button);
EXTERNC void js_joyval_icon_and_desc(int index, DBWORD input, uTCHAR **icon, uTCHAR **desc);

EXTERNC void js_jdev_init(_js_device *jdev);
EXTERNC void js_jdev_type(_js_device *jdev);
EXTERNC void js_jdev_open_common(_js_device *jdev);
EXTERNC BYTE js_jdev_is_xinput(_js_device *jdev);
EXTERNC void js_jdev_scan(void);
EXTERNC void js_jdev_update(_js *js, BYTE enable_decode, BYTE port_index);
EXTERNC BYTE js_jdev_update_axs(_js_device *jdev, BYTE type, BYTE shcut, int index, DBWORD *value, BYTE *mode, float deadzone);
EXTERNC BYTE js_jdev_update_btn(_js_device *jdev, BYTE shcut, int index, DBWORD *value, BYTE *mode);
EXTERNC uTCHAR *js_jdev_desc(int dev);
EXTERNC size_t js_jdev_sizeof_stdctrl(void);

EXTERNC void js_jdev_read_port(_js *js, _port *port);
EXTERNC BYTE js_jdev_read_shcut(_js_sch *js_sch);
EXTERNC DBWORD js_jdev_read_in_dialog(_input_guid *guid);

#if defined (DEBUG)
EXTERNC void js_info_jdev(_js_device *jdev);
#endif

EXTERNC void js_os_init(BYTE first_time);
EXTERNC void js_os_quit(BYTE last_time);
EXTERNC void js_os_jdev_init(_js_device *jdev);
EXTERNC void js_os_jdev_open(_js_device *jdev, void *arg);
EXTERNC void js_os_jdev_close(_js_device *jdev);
EXTERNC void js_os_jdev_scan(void);
EXTERNC void js_os_jdev_read_events_loop(_js_device *jdev);

EXTERNC void js_scan_thread_init(void);
EXTERNC void js_scan_thread_quit(void);

#undef EXTERNC

#endif /* JSTICK_H_ */
