/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#if !defined (_WIN32)
#include <string.h>
#endif
#include <stdlib.h>
#include <math.h>
#include "gui.h"
#include "conf.h"
#include "info.h"
#include "settings.h"

#define JSJDEV ((_js_device *)js->jdev)

thread_funct(js_jdev_read_events_loop, void *arg);
thread_funct(js_detection_loop, void *arg);

const _js_db_device *js_search_in_db(int index);

_js jsp[PORT_MAX], js_shcut;
_jstick jstick;

void js_init(BYTE first_time) {
	int i;

	js_os_init(first_time);

	jstick.last_control = gui_get_ms();

	for (i = PORT1; i < PORT_MAX; i++) {
		js_reset(&jsp[i]);
		memcpy(&jsp[i].guid, &port[i].jguid, sizeof(_input_guid));
	}

	if (first_time) {
		jstick.last_control = 0;
		memset(&jstick.jdd, 0x00, sizeof(jstick.jdd));

		for (i = 0; i < MAX_JOYSTICK; i++) {
			_js_device *jdev = &jstick.jdd.devices[i];

			js_os_jdev_init(jdev);
			jdev->index = i;
			thread_mutex_init(jdev->lock);
			thread_create(jdev->thread, js_jdev_read_events_loop, jdev);
		}

		for (i = PORT1; i < PORT_MAX; i++) {
			thread_mutex_init(jsp[i].lock);
		}

		thread_mutex_init(js_shcut.lock);
	}
}
void js_quit(BYTE last_time) {
	int i;

	if (last_time) {
		for (i = 0; i < MAX_JOYSTICK; i++) {
			_js_device *jdev = &jstick.jdd.devices[i];

			thread_mutex_lock(jdev->lock);
			js_os_jdev_close(jdev);
			thread_mutex_unlock(jdev->lock);
			if (jdev->thread) {
				thread_join(jdev->thread);
				thread_free(jdev->thread);
			}
			thread_mutex_destroy(jdev->lock);
		}
		jstick.jdd.count = 0;

		for (i = PORT1; i < PORT_MAX; i++) {
			thread_mutex_destroy(jsp[i].lock);
		}
		thread_mutex_destroy(js_shcut.lock);
	}

	for (i = PORT1; i < PORT_MAX; i++) {
		jsp[i].jdev = NULL;
	}

	js_os_quit(last_time);
}
void js_reset(_js *js) {
	js_guid_unset(&js->guid);
	js->jdev = NULL;
	js->input_decode_event = NULL;
	js->inited = FALSE;
}

void js_init_shcut(void) {
	_js *js = &js_shcut;

	thread_mutex_lock(js->lock);
	js_reset(js);
	memcpy(&js->guid, &cfg->input.jguid_sch, sizeof(_input_guid));
	thread_mutex_unlock(js->lock);
}

BYTE js_is_connected(int index) {
	if (index >= MAX_JOYSTICK) {
		return (FALSE);
	}
	return (jstick.jdd.devices[index].present ? TRUE : FALSE);
}
BYTE js_is_this(int index, _input_guid *guid) {
	if (index >= MAX_JOYSTICK) {
		return (FALSE);
	}
	return (js_guid_cmp(guid, &jstick.jdd.devices[index].guid));
}
BYTE js_is_null(_input_guid *guid) {
	BYTE *byte = (BYTE *)guid;
	unsigned int i;

	for (i = 0; i < sizeof(_input_guid); i++) {
		if ((*byte) != 0) {
			return (FALSE);
		}
		++byte;
	}
	return (TRUE);
}

#if !defined (_WIN32)
void js_guid_create(_js_device *jdev) {
	WORD *word = (WORD *)&jdev->guid.data;

	js_guid_unset(&jdev->guid);

	(*(word + 0)) = jdev->usb.bustype - 500;
	(*(word + 1)) = 0 - 100;
	if (jdev->usb.vendor_id && jdev->usb.product_id) {
		(*(word + 2)) = jdev->usb.vendor_id;
		(*(word + 3)) = jdev->usb.vendor_id - 200;
		(*(word + 4)) = jdev->usb.product_id;
		(*(word + 5)) = jdev->usb.product_id - 300;
		(*(word + 6)) = jdev->usb.version;
		(*(word + 7)) = jdev->usb.version - 400;
#if defined (_linux__)
		if (ustrlen(jdev->uniq)) {
			BYTE *byte = (BYTE *)&word[2];
			int idx = 0;

			for (const char *s = jdev->uniq; *s; ++s) {
				byte[idx++] ^= *s;
				if (idx > 11) {
					idx = 0;
				}
			}
		}
#endif
	} else {
		word += 2;
		memcpy((char *)word, (char *)jdev->desc, sizeof(jdev->guid.data) - 4);
	}
}
#endif
void js_guid_unset(_input_guid *guid) {
	memset(guid, 0x00, sizeof(_input_guid));
}
void js_guid_set(int index, _input_guid *guid) {
	if (index >= MAX_JOYSTICK) {
		js_guid_unset(guid);
		return;
	}
	memcpy(guid, &jstick.jdd.devices[index].guid, sizeof(_input_guid));
}
BYTE js_guid_cmp(_input_guid *guid1, _input_guid *guid2) {
	if ((memcmp(guid1, guid2, sizeof(_input_guid))) == 0) {
		return (TRUE);
	}
	return (FALSE);
}
uTCHAR *js_guid_to_string(_input_guid *guid) {
	static uTCHAR string[40];

	umemset(string, 0x00, usizeof(string));

#if defined (_WIN32)
	usnprintf(string, sizeof(string), uL("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
		guid->Data1, guid->Data2, guid->Data3,
		guid->Data4[0], guid->Data4[1], guid->Data4[2],
		guid->Data4[3], guid->Data4[4], guid->Data4[5],
		guid->Data4[6], guid->Data4[7]);
#else
	{
		WORD *word = (WORD *)guid;

		usnprintf(string, usizeof(string), "{%04X%04X-%04X-%04X-%04X-%04X%04X%04X}",
			(*(word + 0)), (*(word + 1)), (*(word + 2)), (*(word + 3)),
			(*(word + 4)), (*(word + 5)), (*(word + 6)), (*(word + 7)));
	}
#endif
	return (string);
}
void js_guid_from_string(_input_guid *guid, uTCHAR *string) {
	int ret;
#if defined (_WIN32)
	unsigned long data1;
	unsigned short data2, data3, data4, data5, data6, data7, data8, data9, data10, data11;

	ret = usscanf(string, uL("{%8X-%4hX-%4hX-%2hX%2hX-%2hX%2hX%2hX%2hX%2hX%2hX}"),
		&data1, &data2, &data3, &data4,
		&data5, &data6, &data7, &data8,
		&data9, &data10, &data11);
	if (ret != 11) {
		js_guid_unset(guid);
	} else {
		guid->Data1 = data1;
		guid->Data2 = data2;
		guid->Data3 = data3;
		guid->Data4[0] = data4;
		guid->Data4[1] = data5;
		guid->Data4[2] = data6;
		guid->Data4[3] = data7;
		guid->Data4[4] = data8;
		guid->Data4[5] = data9;
		guid->Data4[6] = data10;
		guid->Data4[7] = data11;
	}
#else
	WORD *word = (WORD *)guid;

	ret = usscanf(string, uL("{%4hX%4hX-%4hX-%4hX-%4hX-%4hX%4hX%4hX}"),
		word + 0, word + 1, word + 2,
		word + 3, word + 4, word + 5,
		word + 6, word + 7);
	if (ret != 8) {
		js_guid_unset(guid);
	}
#endif
}

uTCHAR *js_axs_joyoffset_to_name(DBWORD offset) {
	static uTCHAR str[30];
	unsigned int i;

	for (i = 0; i < LENGTH(js_axs_joyval); i++) {
		if (offset == js_axs_joyval[i].offset) {
			ustrncpy(str, js_axs_joyval[i].desc[2], usizeof(str) - 1);
			return ((uTCHAR *)str);
		}
	}
	return ((uTCHAR *)js_axs_joyval[0].desc[0]);
}
uTCHAR *js_axs_joyval_to_name(DBWORD value) {
	static uTCHAR str[30];
	unsigned int a, b;

	umemset(str, 0x00, usizeof(str));

	for (a = 0; a < LENGTH(js_axs_joyval); a++) {
		for (b = 0; b < 2; b++) {
			if (value == (js_axs_joyval[a].value | b)) {
				ustrncpy(str, js_axs_joyval[a].desc[b], usizeof(str) - 1);
				return ((uTCHAR *)str);
			}
		}
	}
	return ((uTCHAR *)js_axs_joyval[0].desc[0]);
}
DBWORD js_axs_joyval_from_name(const uTCHAR *name) {
	unsigned int a, b;

	for (a = 0; a < LENGTH(js_axs_joyval); a++) {
		for (b = 0; b < 2; b++) {
			if (!ustrcmp(name, js_axs_joyval[a].desc[b])) {
				return (js_axs_joyval[a].value | b);
			}
		}
	}
	return (0);
}
DBWORD js_axs_joyval_to_joyoffset(DBWORD value) {
	unsigned int i;

	for (i = 0; i < LENGTH(js_axs_joyval); i++) {
		if ((value & 0xFFFE) == js_axs_joyval[i].value) {
			return (js_axs_joyval[i].offset);
		}
	}
	return (0x0FF);
}
DBWORD js_axs_joyval_from_joyoffset(DBWORD offset) {
	unsigned int a;

	for (a = 0; a < LENGTH(js_axs_joyval); a++) {
		if (offset == js_axs_joyval[a].offset) {
			return (js_axs_joyval[a].value);
		}
	}
	return (0);
}
void js_axs_validate(_js_axis *jsx, SDBWORD value) {
	value = (SDBWORD)floorf(((float)value - jsx->min) * jsx->scale + JS_AXIS_MIN + 0.5f);

	// thx to the developer of SDL for this routine
	if (!jsx->validate.has_initial_value ||
		(!jsx->validate.has_second_value &&
			((jsx->validate.initial_value <= JS_AXIS_MIN) || (jsx->validate.initial_value == JS_AXIS_MAX)) &&
			(abs(value) < (JS_AXIS_MAX / 4)))) {
		jsx->validate.initial_value = (float)value;
		jsx->validate.has_initial_value = TRUE;
		jsx->value = (SWORD)value;
		if ((jsx->validate.initial_value <= JS_AXIS_MIN) || (jsx->validate.initial_value == JS_AXIS_MAX)) {
			jsx->center = jsx->validate.initial_value;
		} else {
			jsx->center = 0;
		}
	} else if (value == jsx->value) {
		return;
	} else {
		jsx->validate.has_second_value = TRUE;
	}
	if (!jsx->validate.sent_initial_value) {
		const int MAX_ALLOWED_JITTER = JS_AXIS_MAX / 80;

		if (abs(value - jsx->value) <= MAX_ALLOWED_JITTER) {
			return;
		}
		jsx->value = (SWORD)(~value);
		jsx->validate.sent_initial_value = TRUE;
		js_axs_validate(jsx, (SDBWORD)jsx->validate.initial_value);
	}
	jsx->value = (SWORD)value;
}

uTCHAR *js_btn_joyoffset_to_name(DBWORD offset) {
	static uTCHAR str[30];
	unsigned int i;

	for (i = 0; i < LENGTH(js_btn_joyval); i++) {
		if (offset == js_btn_joyval[i].offset) {
			ustrncpy(str, js_btn_joyval[i].desc, usizeof(str) - 1);
			return ((uTCHAR *)str);
		}
	}
	return ((uTCHAR *)js_axs_joyval[0].desc[0]);
}
uTCHAR *js_btn_joyval_to_name(DBWORD value) {
	static uTCHAR str[30];
	unsigned int i;

	umemset(str, 0x00, usizeof(str));

	for (i = 0; i < LENGTH(js_btn_joyval); i++) {
		if (value == js_btn_joyval[i].value) {
			ustrncpy(str, js_btn_joyval[i].desc, usizeof(str) - 1);
			return ((uTCHAR *)str);
		}
	}
	return ((uTCHAR *)js_btn_joyval[0].desc);
}
DBWORD js_btn_joyval_from_name(const uTCHAR *name) {
	unsigned int i;

	for (i = 0; i < LENGTH(js_btn_joyval); i++) {
		if (!ustrcmp(name, js_btn_joyval[i].desc)) {
			return (js_btn_joyval[i].value);
		}
	}
	return (0);
}
DBWORD js_btn_joyval_to_joyoffset(DBWORD value) {
	unsigned int i;

	for (i = 0; i < LENGTH(js_btn_joyval); i++) {
		if (value == js_btn_joyval[i].value) {
			return (js_btn_joyval[i].offset);
		}
	}
	return (0);
}
DBWORD js_btn_joyval_from_joyoffset(DBWORD offset) {
	unsigned int i;

	for (i = 0; i < LENGTH(js_btn_joyval); i++) {
		if (offset == js_btn_joyval[i].offset) {
			return (js_btn_joyval[i].value);
		}
	}
	return (0);
}

uTCHAR *js_joyval_to_name(DBWORD value) {
	if (value & 0x400) {
		return (js_btn_joyval_to_name(value));
	}
	return (js_axs_joyval_to_name(value));
}
DBWORD js_joyval_from_name(const uTCHAR *name) {
	DBWORD value = js_axs_joyval_from_name(name);

	if (value) {
		return (value);
	}
	return (js_btn_joyval_from_name(name));
}
DBWORD js_joyval_to_joyoffset(DBWORD value) {
	if (value & 0x400) {
		return (js_btn_joyval_to_joyoffset(value));
	}
	return (js_axs_joyval_to_joyoffset(value));
}

DBWORD js_joyval_default(int index, int button) {
	const _js_db_device *jdb = js_search_in_db(index);
	const DBWORD *defaults = NULL;
	DBWORD offset;
	unsigned int i;

	defaults = &jdb->std_pad_default[0];
	offset = (*(defaults + button));

	if (JS_IS_BTN_DEF(offset)) {
		offset = JS_BTNABS_UNDEF(offset);

		for (i = 0; i < LENGTH(js_btn_joyval); i++) {
			if (offset == js_btn_joyval[i].offset) {
				return (js_btn_joyval[i].value);
			}
		}
		return (js_btn_joyval[0].offset);
	} else {
		DBWORD min = (offset & JS_ABS_DEF_BIT(1)) != 0;

		offset = JS_BTNABS_UNDEF(offset);
		for (i = 0; i < LENGTH(js_axs_joyval); i++) {
			if (offset == js_axs_joyval[i].offset) {
				return (js_axs_joyval[i].value | min);
			}
		}
		return (js_axs_joyval[0].offset);
	}
}
void js_joyval_icon_and_desc(int index, DBWORD input, uTCHAR **icon, uTCHAR **desc) {
	(*icon) = NULL;
	(*desc) = NULL;

	if ((index < MAX_JOYSTICK) && (jstick.jdd.devices[index].type != JS_SC_UNKNOWN)) {
		const _js_db_device *jdb = js_search_in_db(index);
		const _js_db_device_icon_desc *btn = &jdb->btn[0], *axs = &jdb->axs[0], *tid = NULL;
		unsigned int i, len_btn = LENGTH(jdb->btn), len_axs = LENGTH(jdb->axs);
		DBWORD offset;

		if (input & 0x400) {
			offset = js_joyval_to_joyoffset(input);
			for (i = 0; i < len_btn; i++) {
				tid = btn + i;
				if (offset == tid->offset) {
					break;
				}
			}
		} else {
			offset = js_joyval_to_joyoffset(input & ~0x01);
			for (i = 0; i < len_axs; i += 2) {
				tid = axs + i;
				if (offset == tid->offset) {
					tid = axs + (i + (input & 0x01));
					break;
				}
			}
		}
		if (tid) {
			(*icon) = (uTCHAR *)&tid->icon[0];
			(*desc) = (uTCHAR *)&tid->desc[0];
		}
	} else {
		(*desc) = js_joyval_to_name(input);
	}
}

void js_jdev_init(_js_device *jdev) {
	jdev->type = JS_SC_UNKNOWN;
	jdev->is_xinput = FALSE;

	umemset(jdev->desc, 0x00, usizeof(jdev->desc));

	memset(&jdev->info, 0x00, sizeof(_js_info));
	memset(&jdev->data, 0x00, sizeof(_js_data));
	memset(&jdev->usb, 0x00, sizeof(jdev->usb));
	memset(&jdev->port, 0x00, sizeof(jdev->port));
	memset(&jdev->shcut, 0x00, sizeof(jdev->shcut));

	js_guid_unset(&jdev->guid);

	js_os_jdev_init(jdev);
}
void js_jdev_type(_js_device *jdev) {
	unsigned int i;

	if (!(jdev->usb.vendor_id | jdev->usb.product_id)) {
		jdev->type = JS_SC_UNKNOWN;
		return;
	}
	for (i = 0; i < LENGTH(js_gamepads_list); i++) {
		if ((jdev->usb.vendor_id == js_gamepads_list[i].vendor_id) &&
			(jdev->usb.product_id == js_gamepads_list[i].product_id)) {
			jdev->type = js_gamepads_list[i].type;
			return;
		}
	}
}
void js_jdev_open_common(_js_device *jdev) {
	unsigned int i;

	for (i = 0; i < LENGTH(js_gamepads_list); i++) {
		if ((jdev->usb.vendor_id == js_gamepads_list[i].vendor_id) &&
			(jdev->usb.product_id == js_gamepads_list[i].product_id)) {
			if (js_gamepads_list[i].desc) {
				umemset(jdev->desc, 0x00, usizeof(jdev->desc));
				ustrncpy(jdev->desc, js_gamepads_list[i].desc, usizeof(jdev->desc) - 1);
			}
			break;
		}
	}
	settings_jsc_parse(jdev->index);
}
BYTE js_jdev_is_xinput(_js_device *jdev) {
	if ((jdev->type == JS_SC_MS_XBOX_360_GAMEPAD) || (jdev->type == JS_SC_MS_XBOX_ONE_GAMEPAD) ||
		(((jdev->usb.vendor_id == JS_USB_VENDOR_ID_VALVE) && (jdev->usb.product_id == JS_USB_PID_STEAM_VIRTUAL_GAMEPAD)))) {
		return (TRUE);
	}
	return (FALSE);
}
void js_jdev_scan(void) {
	double this_control = gui_get_ms();

	if ((this_control - jstick.last_control) >= JS_MS_UPDATE_DETECT_DEVICE) {
		int i;

		js_os_jdev_scan();

		for (i = PORT1; i < PORT_MAX; i++) {
			js_jdev_update(&jsp[i], TRUE, i);
		}
		js_jdev_update(&js_shcut, FALSE, 0);

		jstick.last_control = gui_get_ms();
	}
}
void js_jdev_update(_js *js, BYTE enable_decode, BYTE port_index) {
	_port *p = &port[port_index];
	_port_funct *pf = &port_funct[port_index];

	thread_mutex_lock(js->lock);

	if (!js->jdev) {
		js->jdev = NULL;
		js->inited = FALSE;
		js->input_decode_event = NULL;

		if (!js_is_null(&js->guid)) {
			int i;

			for (i = 0; i < MAX_JOYSTICK; i++) {
				if (js_guid_cmp(&js->guid, &jstick.jdd.devices[i].guid)) {
					js->jdev = &jstick.jdd.devices[i];
					js->inited = TRUE;
					if (enable_decode) {
						for (size_t btn = 0; btn < MAX_STD_PAD_BUTTONS ; btn++) {
							if (js->stdctrl_from_cmdline[btn]) {
								JSJDEV->stdctrl[btn] = js->stdctrl_from_cmdline[btn];
								js->stdctrl_from_cmdline[btn] = 0;
							}
							p->input[JOYSTICK][btn] = JSJDEV->stdctrl[btn];
						}
						js->input_decode_event = pf->input_decode_event;
					}
					break;
				}
			}
		}
	} else if (!JSJDEV->present) {
		js->jdev = NULL;
		js->inited = FALSE;
		js->input_decode_event = NULL;
		memset(p->input[JOYSTICK], 0x00, js_jdev_sizeof_stdctrl());
	}

	thread_mutex_unlock(js->lock);
}
BYTE js_jdev_update_axs(_js_device *jdev, BYTE type, BYTE shcut, int index, DBWORD *value, BYTE *mode, float deadzone) {
	_js_axis *jsx = !type ? &jdev->data.axis[index] : &jdev->data.hat[index];
	float *last_state = !type
		? shcut ? &jdev->shcut.last_state.axis[index] : &jdev->port.last_state.axis[index]
		: shcut ? &jdev->shcut.last_state.hats[index] : &jdev->port.last_state.hats[index];
	float *last_value = !type
		? shcut ? &jdev->shcut.last_value.axis[index] : &jdev->port.last_value.axis[index]
		: shcut ? &jdev->shcut.last_value.hats[index] : &jdev->port.last_value.hats[index];
	float center = jsx->center;
	float state = jsx->value;

	if ((state < (center + deadzone)) && (state > (center - deadzone))) {
		state = center;
	}
	if ((state < ((*last_value) - deadzone)) || (state > ((*last_value) + deadzone))) {
		if (state != center) {
			(*mode) = PRESSED;
			(*value) = js_axs_joyval_from_joyoffset(jsx->offset);
			if (state > center) {
				(*value) |= 1;
			}
			(*last_state) = (float)(*value);
		} else {
			(*mode) = RELEASED;
			(*value) = (DBWORD)(*last_state);
			(*last_state) = 0;
		}

		(*last_value) = state;

		return (EXIT_OK);
	}
	return (EXIT_ERROR);
}
BYTE js_jdev_update_btn(_js_device *jdev, BYTE shcut, int index, DBWORD *value, BYTE *mode) {
	BYTE *last_state = shcut ? &jdev->shcut.last_state.button[index] : &jdev->port.last_state.button[index];
	BYTE *last_value = shcut ? &jdev->shcut.last_value.button[index] : &jdev->port.last_value.button[index];
	_js_button *jsx = &jdev->data.button[index];
	BYTE state = jsx->value;

	if (state != (*last_state)) {
		(*value) = js_btn_joyval_from_joyoffset(jsx->offset);

		if (state == 0) {
			(*mode) = RELEASED;
		} else if (state == 1) {
			(*mode) = PRESSED;
		} else {
			(*value) = 0;
		}

		(*last_state) = state;
		(*last_value) = state;

		return (EXIT_OK);
	}

	return (EXIT_ERROR);
}
uTCHAR *js_jdev_desc(int dev) {
	if (dev >= MAX_JOYSTICK) {
		return (NULL);
	}
	return (jstick.jdd.devices[dev].desc);
}
size_t js_jdev_sizeof_stdctrl(void) {
	return (sizeof(jstick.jdd.devices[0].stdctrl));
}

void js_jdev_read_port(_js *js, _port *prt) {
	_js_device *jdev;

	thread_mutex_lock(js->lock);

	jdev = js->jdev;

	if (js->inited && jdev) {
		thread_mutex_lock(jdev->lock);

		if (jdev->present) {
			float deadzone = (JS_AXIS_MAX / 100.0f) * (float)jdev->deadzone;
			unsigned int i, a;
			DBWORD value = 0;
			BYTE mode = 0;

			for (i = 0; i < LENGTH(js_axs_type); i++) {
				for (a = 0; a < js_axs_type[i]; a++) {
					_js_axis *jsx = !i ? &jdev->data.axis[a] : &jdev->data.hat[a];

					if (jsx->used & jsx->enabled)  {
						if (js_jdev_update_axs(jdev, i, FALSE, (int)a, &value, &mode, deadzone) == EXIT_OK) {
							if (value && js->input_decode_event) {
								js->input_decode_event(mode, FALSE, value, JOYSTICK, prt);
							}
						}
					}
				}
			}
			for (i = 0; i < JS_MAX_BUTTONS; i++) {
				_js_button *jsx = &jdev->data.button[i];

				if (jsx->used & jsx->enabled) {
					if (js_jdev_update_btn(jdev, FALSE, (int)i, &value, &mode) == EXIT_OK) {
						if (value && js->input_decode_event) {
							js->input_decode_event(mode, FALSE, value, JOYSTICK, prt);
						}
					}
				}
			}
		}

		thread_mutex_unlock(jdev->lock);
	}

	thread_mutex_unlock(js->lock);
}
BYTE js_jdev_read_shcut(_js_sch *js_sch) {
	_js *js = &js_shcut;
	_js_device *jdev;
	DBWORD value = 0;

	thread_mutex_lock(js->lock);

	jdev = js->jdev;

	if (js->inited && jdev) {
		thread_mutex_lock(jdev->lock);

		if (jdev->present) {
			static float deadzone = (JS_AXIS_MAX / 100.0f) * 45.0f;
			BYTE mode = 0;
			unsigned int i, a;

			for (i = 0; i < LENGTH(js_axs_type); i++) {
				for (a = 0; a < js_axs_type[i]; a++) {
					_js_axis *jsx = !i ? &jdev->data.axis[a] : &jdev->data.hat[a];

					if (jsx->used & jsx->enabled) {
						if (js_jdev_update_axs(jdev, i, TRUE, (int)a, &value, &mode, deadzone) == EXIT_OK) {
							if (value) {
								js_sch->value = value;
								js_sch->mode = mode;
								break;
							}
						}
					}
				}
			}
			if (value == 0) {
				for (i = 0; i < JS_MAX_BUTTONS; i++) {
					_js_button *jsx = &jdev->data.button[i];

					if (jsx->used & jsx->enabled) {
						if (js_jdev_update_btn(jdev, TRUE, (int)i, &value, &mode) == EXIT_OK) {
							if (value) {
								js_sch->value = value;
								js_sch->mode = mode;
								break;
							}
						}
					}
				}
			}
		}

		thread_mutex_unlock(jdev->lock);
	}

	thread_mutex_unlock(js->lock);

	return (value ? EXIT_OK : EXIT_ERROR);
}
DBWORD js_jdev_read_in_dialog(_input_guid *guid) {
	_js_device *jdev = NULL;
	DBWORD value = 0;
	{
		int i;

		for (i = 0; i < MAX_JOYSTICK; i++) {
			if (js_is_this(i, guid)) {
				jdev = &jstick.jdd.devices[i];
				break;
			}
		}
	}

	if (jdev) {
		thread_mutex_lock(jdev->lock);

		if (jdev->present) {
			static float deadzone = (JS_AXIS_MAX / 100.0f) * 75.0f;
			unsigned int i, a;

			for (i = 0; i < LENGTH(js_axs_type); i++) {
				for (a = 0; a < js_axs_type[i]; a++) {
					_js_axis *jsx = !i ? &jdev->data.axis[a] : &jdev->data.hat[a];

					if (jsx->used & jsx->enabled) {
						if (((float)jsx->value < (jsx->center - deadzone)) || ((float)jsx->value > (jsx->center + deadzone))) {
							value = js_axs_joyval_from_joyoffset(jsx->offset);
							if ((float)jsx->value > jsx->center) {
								value |= 1;
							}
						}
					}
				}
			}
			if (value == 0) {
				for (i = 0; i < JS_MAX_BUTTONS; i++) {
					_js_button *jsx = &jdev->data.button[i];

					if (jsx->used & jsx->enabled) {
						if (jsx->value) {
							value = js_btn_joyval_from_joyoffset(jsx->offset);
						}
					}
				}
			}
		}

		thread_mutex_unlock(jdev->lock);
	}

	return (value);
}

#if defined (DEBUG)
void js_info_jdev(_js_device *jdev) {
	unsigned int i, a;
	int axes = 0, buttons = 0;

	log_info(uL("description;" uPs("")), jdev->desc);
#if !defined (_WIN32)
	log_info_box(uL("device;" uPs("")), jdev->dev);
#endif
	log_info_box(uL("usb;bustype %04X - vid:pid %04X:%04X - version %04X"),
		jdev->usb.bustype, jdev->usb.vendor_id, jdev->usb.product_id, jdev->usb.version);
	log_info_box(uL("gid;" uPs("")), js_guid_to_string(&jdev->guid));

	for (i = 0; i < LENGTH(js_axs_type); i++) {
		for (a = 0; a < js_axs_type[i]; a++) {
			_js_axis *jsx = !i ? &jdev->data.axis[a] : &jdev->data.hat[a];

			if (jsx->used) {
				axes++;
				log_info_box(uL("axis %d;0x%.3x " uPs("-15") " { %6d %6d %6d %13f}"), axes, jsx->offset,
					js_axs_joyoffset_to_name((const DBWORD)jsx->offset),
						(int)jsx->min,
						(int)jsx->max,
						(int)jsx->center,
						jsx->scale);
			}
		}
	}
	for (i = 0; i < JS_MAX_BUTTONS; i++) {
		_js_button *jsx = &jdev->data.button[i];

		if (jsx->used) {
			buttons++;
			log_info_box(uL("button %d;0x%03x " uPs("")), buttons, jsx->offset, js_btn_joyoffset_to_name(jsx->offset));
		}
	}
}
#endif

void js_scan_thread_init(void) {
	thread_create(jstick.thread, js_detection_loop, NULL);
}
void js_scan_thread_quit(void) {
	if (jstick.thread) {
		thread_join(jstick.thread);
		thread_free(jstick.thread);
	}
}

thread_funct(js_jdev_read_events_loop, void *arg) {
	_js_device *jdev = (_js_device *)arg;

	while (!info.stop) {
		thread_mutex_lock(jdev->lock);

		if (jdev->present) {
			js_os_jdev_read_events_loop(jdev);
		}

		thread_mutex_unlock(jdev->lock);

		// pausa caffe'
		gui_sleep(1);
	}

	thread_funct_return();
}
thread_funct(js_detection_loop, UNUSED(void *arg)) {
	while (!info.stop) {
		// gestione pads
		js_jdev_scan();
		// gestione shortcuts
		gui_dlgsettings_input_update_joy_combo();
		// pausa caffe'
		gui_sleep(10);
	}
	thread_funct_return();
}

const _js_db_device *js_search_in_db(int index) {
	const _js_db_device *jdb = &js_db_devices[0];
	unsigned int i;

	if (index < MAX_JOYSTICK) {
		_js_device *jdev = &jstick.jdd.devices[index];

		// cerco il default;
		for (i = 0; i < LENGTH(js_db_devices); i++) {
			const _js_db_device *db = &js_db_devices[i];

			if ((db->type == jdev->type) && db->is_default) {
				jdb = db;
				break;
			}
		}
		// cerco il caso device specifico;
		for (i = 0; i < LENGTH(js_db_devices); i++) {
			const _js_db_device *db = &js_db_devices[i];

			if ((db->type == jdev->type) && (db->vendor_id == jdev->usb.vendor_id) && (db->product_id == jdev->usb.product_id)) {
				jdb = db;
				break;
			}
		}
	}
	return (jdb);
}
