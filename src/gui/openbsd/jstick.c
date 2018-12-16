/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/stat.h>
#include "jstick.h"
#include "input.h"
#include "conf.h"
#include "gui.h"

#ifndef __FreeBSD_kernel_version
#define __FreeBSD_kernel_version __FreeBSD_version
#endif

#if defined (HAVE_USB_H)
#include <usb.h>
#endif
#if defined (__DragonFly__)
#include <bus/usb/usb.h>
#include <bus/usb/usbhid.h>
#else
#include <dev/usb/usb.h>
#include <dev/usb/usbhid.h>
#endif

#if defined (HAVE_USBHID_H)
#include <usbhid.h>
#elif defined (HAVE_LIBUSB_H)
#include <libusb.h>
#elif defined (HAVE_LIBUSBHID_H)
#include <libusbhid.h>
#endif

#if defined (__FREEBSD__) || defined (__FreeBSD_kernel__)
#if !defned (__DragonFly__)
#include <osreldate.h>
#endif
#if __FreeBSD_kernel_version > 800063
#include <dev/usb/usb_ioctl.h>
#endif
#include <sys/joystick.h>
#endif

#if defined (USBHID_MACHINE_JOYSTICK)
#include <machine/joystick.h>
#endif

#if defined (USBHID_UCR_DATA) || (defined (__FreeBSD_kernel__) && __FreeBSD_kernel_version <= 800063)
#define REP_BUF_DATA(rep) (rep.buf->ucr_data)
#elif (defined (__FREEBSD__) && (__FreeBSD_kernel_version > 900000))
#define REP_BUF_DATA(rep) (rep.buf)
#elif (defined (__FREEBSD__) && (__FreeBSD_kernel_version > 800063))
#define REP_BUF_DATA(rep) (rep.buf->ugd_data)
#else
#define REP_BUF_DATA(rep) (rep.buf->data)
#endif

#define JDEV ((_js_device *)joy->jdev)
#define JS_AXIS_SENSIBILITY 0.45f
#define JS_AXIS_SENSIBILITY_DIALOG 0.45f
#define JS_AXIS_TO_FLOAT(vl)\
	(float)(vl - jdev->axis[index].min) / (float)(jdev->axis[index].max - jdev->axis[index].min) * 2.0f - 1.0f

enum _js_types {
	JS_TYPE_JOY,
	JS_TYPE_UHID,
	JS_TYPE_UNKNOW
};
enum _js_limit_devs {
#if defined (__FREEBSD__) || defined (USBHID_MACHINE_JOYSTICK) || defined (__FreeBSD_kernel__)
	JS_MAX_JOY_DEV = 2,
#else
	JS_MAX_JOY_DEV = 0,
#endif
	JS_MAX_UHID_DEV = MAX_JOYSTICK - JS_MAX_JOY_DEV,
	JS_MAX_DEV = JS_MAX_UHID_DEV + JS_MAX_JOY_DEV
};
enum _js_states {
	JS_ST_CTRL,
	JS_ST_SCH,
	JS_ST_MAX
};
enum _js_misc {
	JS_MS_UPDATE_DETECT_DEVICES = 5000,
};
enum _js_hat {
	JS_HAT_UP,
	JS_HAT_UP_RIGHT,
	JS_HAT_RIGHT,
	JS_HAT_RIGHT_DOWN,
	JS_HAT_DOWN,
	JS_HAT_DOWN_LEFT,
	JS_HAT_LEFT,
	JS_HAT_LEFT_UP,
	JS_HATS = 2,
};
enum _js_axes {
	JS_AXE_X,
	JS_AXE_Y,
	JS_AXE_Z,
	JS_AXE_RX,
	JS_AXE_RY,
	JS_AXE_RZ,
	JS_AXE_SLIDER,
	JS_AXE_WHEEL,
	JS_HAT0_A,
	JS_HAT0_B,
	JS_HAT1_A,
	JS_HAT1_B,
	JS_AXES,
	// xbox 360 (I treat them like buttons)
	JS_DPAD_UP = JS_HAT0_A,
	JS_DPAD_DOWN = JS_HAT0_B,
	JS_DPAD_LEFT = JS_HAT1_A,
	JS_DPAD_RIGHT = JS_HAT1_B
};

typedef struct _udb_hid_device {
	uTCHAR *desc;
	uTCHAR *serial;
	unsigned int flags;
} _usb_hid_device;
typedef struct _js_last_states {
	float *axis;
	SDBWORD *hat;
	SDBWORD *button;
} _js_last_states;
typedef struct _js_device {
	BYTE id;
	BYTE type;
	uTCHAR dev[30];
	int fd;
	struct report_desc *repdesc;
	struct _js_report {
		int id;
#if defined (__FREEBSD__) && (__FreeBSD_kernel_version > 900000)
		void *buf;
#elif defined (__FREEBSD__) && (__FreeBSD_kernel_version > 800063)
		struct usb_gen_descriptor *buf;
#else
		struct usb_ctl_report *buf;
#endif
		int size;
	} report;
	struct _js_info {
		int axes;
		int buttons;
		int hats;
		int balls;
	} info;
	struct js_axis_info {
		BYTE used;
		float min;
		float max;
		float center;
	} axis [JS_AXES];
	struct js_hat_info {
		BYTE used;
		float min;
		float max;
	} hat [JS_HATS];
	_js_last_states states[JS_ST_MAX];
#if defined (__FREEBSD__) || defined (USBHID_MACHINE_JOYSTICK) || defined (__FreeBSD_kernel__)
	// only with gameport joystick
	uTCHAR *desc;
#endif
	// uhidev
	_usb_hid_device *usb;
} _js_device;

static _usb_hid_device *usb_alloc_device(void);
static void usb_free_device(_usb_hid_device *udev);
static void usb_free_udd(void);
static void usb_detect_devices(void);

static _js_device *js_alloc_device(void);
static void js_free_device(_js_device *jdev);
static void js_free_jdd(void);
static void js_detect_devices(void);
static void js_close_detected_devices(void);

static void js_update_jdev(_js *joy, BYTE enable_decode, BYTE decode_index);
static void js_update_jdevs(void);

INLINE static DBWORD js_update_axis(_js *joy, _port *port, enum _js_states st, int index, float value, BYTE *mode);
INLINE static DBWORD js_update_button(_js *joy, _port *port, enum _js_states st, int index, SDBWORD value, BYTE *mode);
INLINE static DBWORD js_update_hat(_js *joy, _port *port, enum _js_states st, int index, SDBWORD value, BYTE *mode);

INLINE static void js_hat_to_xy(SDBWORD hat, float *x, float *y);
INLINE static int js_usage_to_axis(unsigned int usage);

INLINE static void js_lock(void);
INLINE static void js_unlock(void);

static const struct _js_special {
	u_int16_t vendor;
	u_int16_t product;
	enum _js_sc_flags {
		JS_SC_NONE,
		JS_SC_MS_XBOX_360_GAMEPD = (1 << 0),
	} flags;
} js_special_case[] = {
	{ 0x045E, 0x028E, JS_SC_MS_XBOX_360_GAMEPD },
};
static struct _jstick {
	double ts_update_devices;
	pthread_mutex_t lock;
	struct _usb_devices_detected {
		int count;
		_usb_hid_device **devices;
	} udd;
	struct _js_devices_detected {
		int count;
		_js_device **devices;
	} jdd;
} jstick;

void js_init(BYTE first_time) {
	int i;

	if (first_time) {
		memset(&jstick, 0x00, sizeof(jstick));

		if (pthread_mutex_init(&jstick.lock, NULL) != 0) {
			fprintf(stderr, "Unable to allocate the thread mutex\n");
		}
	}

	js_lock();
	for (i = PORT1; i < PORT_MAX; i++) {
		memset(&js[i], 0x00, sizeof(_js));
		js[i].id = port[i].joy_id;
	}
	js_detect_devices();
	js_unlock();
}
void js_quit(BYTE last_time) {
	int i;

	js_close_detected_devices();

	for (i = PORT1; i < PORT_MAX; i++) {
		js[i].jdev = NULL;
	}

	if (last_time) {
		pthread_mutex_destroy(&jstick.lock);
	}
}
void js_update_detected_devices(void) {
	js_lock();
	js_detect_devices();
	js_unlock();
}
void js_control(_js *joy, _port *port) {
	struct hid_item hitem;
	struct hid_data *hdata;
	_js_device *jdev = joy->jdev;
	_js_last_states *states = NULL;
	BYTE mode = 0;

	js_lock();

	jdev = joy->jdev;

	if (joy->inited == FALSE) {
		js_unlock();
		return;
	} else if (jdev == NULL) {
		double now = gui_get_ms();

		if ((now - jstick.ts_update_devices) > JS_MS_UPDATE_DETECT_DEVICES) {
			js_detect_devices();
		}
		js_unlock();
		return;
	}

	states = &jdev->states[JS_ST_CTRL];

#if defined (__FREEBSD__) || defined (USBHID_MACHINE_JOYSTICK) || defined (__FreeBSD_kernel__)
	if (jdev->type == JS_TYPE_JOY) {
		// FIXME : to implement
		js_unlock();
		return;
	}
#endif

#define js_control_axis(index)\
	dvl = hid_get_data(REP_BUF_DATA(jdev->report), &hitem);\
	fvl = JS_AXIS_TO_FLOAT(dvl);\
	js_update_axis(joy, port, JS_ST_CTRL, index, fvl, &mode);
#define js_control_button(index)\
	dvl = hid_get_data(REP_BUF_DATA(jdev->report), &hitem);\
	js_update_button(joy, port, JS_ST_CTRL, index, dvl, &mode);
#define js_control_hat(index)\
	dvl = hid_get_data(REP_BUF_DATA(jdev->report), &hitem);\
	js_update_hat(joy, port, JS_ST_CTRL, index, dvl, &mode);

	while (read(jdev->fd, REP_BUF_DATA(jdev->report), jdev->report.size) == jdev->report.size) {
		int hat = 0;

#if defined (USBHID_NEW) || (defined (__FREEBSD__) && __FreeBSD_kernel_version >= 500111) || defined (__FreeBSD_kernel__)
		if ((hdata = hid_start_parse(jdev->repdesc, 1 << hid_input, jdev->report.id)) == NULL) {
#else
		if ((hdata = hid_start_parse(jdev->repdesc, 1 << hid_input)) == NULL) {
#endif
				continue;
		}

		while (hid_get_item(hdata, &hitem) > 0) {
			unsigned int usage = HID_USAGE(hitem.usage);
			unsigned int page = HID_PAGE(hitem.usage);
			SDBWORD dvl = 0;
			float fvl = 0;

			if (hitem.kind != hid_input) {
				continue;
			}

			if (page == HUP_GENERIC_DESKTOP) {
				int index;

				if (usage == HUG_HAT_SWITCH) {
					if (hat >= JS_HATS) {
						hat++;
						continue;
					}
					js_control_hat(hat);
					hat++;
					continue;
				}

				switch ((index = js_usage_to_axis(usage))) {
					case JS_DPAD_UP:
					case JS_DPAD_DOWN:
					case JS_DPAD_LEFT:
					case JS_DPAD_RIGHT:
						js_control_button((index - JS_DPAD_UP + jdev->info.buttons))
						break;
					case JS_AXE_X:
					case JS_AXE_Y:
					case JS_AXE_RX:
					case JS_AXE_RY:
					case JS_AXE_Z:
					case JS_AXE_RZ:
						js_control_axis(index)
						break;
					case JS_AXE_SLIDER:
					case JS_AXE_WHEEL:
						break;
					default:
						break;
				}
			} else if (page == HUP_BUTTON) {
				js_control_button((hitem.usage & 0x0F))
			}
		}
		hid_end_parse(hdata);
	}

#undef js_control_axis
#undef js_control_button
#undef js_control_hat

	js_unlock();
}

BYTE js_is_connected(int dev) {
	struct stat sb;

	if (dev >= jstick.jdd.count) {
		return (EXIT_ERROR);
	}

	return ((fstat(jstick.jdd.devices[dev]->fd, &sb) == -1) ? EXIT_ERROR : EXIT_OK);
}
BYTE js_is_this(BYTE dev, BYTE *id) {
	if (dev >= jstick.jdd.count) {
		return (FALSE);
	}

	return (jstick.jdd.devices[dev]->id == (*id));
}
BYTE js_is_null(BYTE *id) {
	return ((*id) == name_to_jsn(uL("NULL")));
}
void js_set_id(BYTE *id, int dev) {
	if (dev >= jstick.jdd.count) {
		(*id) = name_to_jsn(uL("NULL"));
		return;
	}

	(*id) = jstick.jdd.devices[dev]->id;
}
uTCHAR *js_name_device(int dev) {
	if (dev >= jstick.jdd.count) {
		return (NULL);
	}

#if defined (__FREEBSD__) || defined (USBHID_MACHINE_JOYSTICK) || defined (__FreeBSD_kernel__)
	// only with gameport joystick
	if (jstick.jdd.devices[dev]->desc) {
		return (jstick.jdd.devices[dev]->desc);
	}
#endif

	// uhidev
	return (jstick.jdd.devices[dev]->usb->desc);
}
uTCHAR *js_to_joyname(const DBWORD val) {
	static uTCHAR str[20];

	umemset(str, 0x00, usizeof(str));

	if (val < JS_MAX_DEV) {
		usnprintf(str, usizeof(str), uL("JOYSTICKID%d"), val + 1);
	} else {
		usnprintf(str, usizeof(str), uL("NULL"));
	}

	return (str);
}
uTCHAR *js_to_name(const DBWORD val, const _js_element *list, const DBWORD length) {
	BYTE index;
	static uTCHAR str[20];

	umemset(str, 0x00, usizeof(str));

	for (index = 0; index < length; index++) {
		if (val == list[index].value) {
			ustrncpy(str, list[index].name, usizeof(str));
			return ((uTCHAR *)str);
		}
	}
	return ((uTCHAR *)list[0].name);
}
DBWORD js_from_joyname(const uTCHAR *name) {
	if (ustrncmp(name, uL("NULL"), 4) == 0) {
		return (0xFF);
	}

	if (ustrncmp(name, uL("JOYSTICKID"), 10) == 0) {
		return (((DBWORD)strtol(name + 10, NULL, 10) - 1));
	}

	return (0);
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
DBWORD js_read_in_dialog(BYTE *id, UNUSED(int fd)) {
	struct hid_item hitem;
	struct hid_data *hdata;
	_js_device *jdev = NULL;
	_js_last_states *states = NULL;
	DBWORD value = 0;
	int i;

	js_lock();

	for (i = 0; i < jstick.jdd.count; i++) {
		if ((*id) == jstick.jdd.devices[i]->id) {
			jdev = jstick.jdd.devices[i];
		}
	}

	if (jdev == NULL) {
		js_unlock();
		return (value);
	}

	states = &jdev->states[JS_ST_SCH];

#if defined (__FREEBSD__) || defined (USBHID_MACHINE_JOYSTICK) || defined (__FreeBSD_kernel__)
	if (jdev->type == JS_TYPE_JOY) {
		// FIXME : to implement
		js_unlock();
		return (value);
	}
#endif

	for (i = 0; i < jdev->info.axes; i++) {
		states->axis[i] = 0.0f;
	}
	for (i = 0; i < jdev->info.hats; i++) {
		states->hat[i] = 0;
	}
	for (i = 0; i < jdev->info.buttons; i++) {
		states->button[i] = 0;
	}

#define js_read_in_dialog_axis(axis, vl)\
	fvl = vl;\
	if (fvl >= JS_AXIS_SENSIBILITY_DIALOG) {\
		value = (axis << 1) + 1 + 1;\
	} else if (fvl <= -JS_AXIS_SENSIBILITY_DIALOG) {\
		value = (axis << 1) + 1;\
	}
#define js_read_in_dialog_button(btn)\
	if ((dvl = hid_get_data(REP_BUF_DATA(jdev->report), &hitem))) {\
		value = btn | 0x400;\
	}

	while (read(jdev->fd, REP_BUF_DATA(jdev->report), jdev->report.size) == jdev->report.size) {
		int hat = 0;

#if defined (USBHID_NEW) || (defined (__FREEBSD__) && __FreeBSD_kernel_version >= 500111) || defined (__FreeBSD_kernel__)
		if ((hdata = hid_start_parse(jdev->repdesc, 1 << hid_input, jdev->report.id)) == NULL) {
#else
		if ((hdata = hid_start_parse(jdev->repdesc, 1 << hid_input)) == NULL) {
#endif
				continue;
		}

		while (hid_get_item(hdata, &hitem) > 0) {
			unsigned int usage = HID_USAGE(hitem.usage);
			unsigned int page = HID_PAGE(hitem.usage);
			SDBWORD dvl = 0;
			float fvl = 0;

			if (hitem.kind != hid_input) {
				continue;
			}

			if (page == HUP_GENERIC_DESKTOP) {
				int index = js_usage_to_axis(usage);

				if (usage == HUG_HAT_SWITCH) {
					float x, y;

					dvl = hid_get_data(REP_BUF_DATA(jdev->report), &hitem);

					if ((hat >= JS_HATS) || (states->hat[hat] == dvl)) {
						hat++;
						continue;
					}

					states->hat[hat] = dvl;
					js_hat_to_xy(dvl, &x, &y);

					if (x != 0.0f) {
						js_read_in_dialog_axis((JS_HAT0_A + hat), x)
					} else {
						js_read_in_dialog_axis((JS_HAT0_B + hat), y)
					}

					hat++;
					continue;
				}

				switch (index) {
					case JS_DPAD_UP:
					case JS_DPAD_DOWN:
					case JS_DPAD_LEFT:
					case JS_DPAD_RIGHT:
						js_read_in_dialog_button((index - JS_DPAD_UP + jdev->info.buttons))
						break;
					case JS_AXE_X:
					case JS_AXE_Y:
					case JS_AXE_RX:
					case JS_AXE_RY:
					case JS_AXE_Z:
					case JS_AXE_RZ:
						dvl = hid_get_data(REP_BUF_DATA(jdev->report), &hitem);
						fvl = JS_AXIS_TO_FLOAT(dvl);

						if (states->axis[index] == fvl) {
							break;
						}

						states->axis[index] = fvl;
						js_read_in_dialog_axis(index, fvl)
						break;
					case JS_AXE_SLIDER:
					case JS_AXE_WHEEL:
						break;
					default:
						break;
				}
			} else if (page == HUP_BUTTON) {
				js_read_in_dialog_button((hitem.usage & 0x0F))
			}
		}
		hid_end_parse(hdata);
	}

#undef js_read_in_dialog_axis
#undef js_read_in_dialog_button

	js_unlock();

	return (value);
}

void js_shcut_init(void) {
	memset(&js_shcut, 0x00, sizeof(_js));
	js_shcut.id = cfg->input.shcjoy_id;
	js_update_jdev(&js_shcut, FALSE, 0);
}
void js_shcut_stop(void) {}
BYTE js_shcut_read(_js_sch *js_sch) {
	struct hid_item hitem;
	struct hid_data *hdata;
	_js *joy= &js_shcut;
	_js_device *jdev;
	_js_last_states *states = NULL;
	SDBWORD value = 0;
	BYTE mode = 0;

	js_sch->value = 0;
	js_sch->mode = 255;

	js_lock();

	jdev = joy->jdev;

	if (joy->inited == FALSE) {
		js_unlock();
		return (value);
	} else if (jdev == NULL) {
		double now = gui_get_ms();

		if ((now - jstick.ts_update_devices) > JS_MS_UPDATE_DETECT_DEVICES) {
			js_detect_devices();
		}
		js_unlock();
		return (value);
	}

	states = &jdev->states[JS_ST_CTRL];

#if defined (__FREEBSD__) || defined (USBHID_MACHINE_JOYSTICK) || defined (__FreeBSD_kernel__)
	if (jdev->type == JS_TYPE_JOY) {
		// FIXME : to implement
		js_unlock();
		return (value);
	}
#endif

#define _js_shcut_read_control(funct, index, val)\
	if ((value = funct(joy, NULL, JS_ST_SCH, index, val, &mode))) {\
		js_sch->value = value;\
		js_sch->mode = mode;\
		js_unlock();\
		return (EXIT_OK);\
	}
#define js_shcut_read_axis(index)\
	dvl = hid_get_data(REP_BUF_DATA(jdev->report), &hitem);\
	fvl = JS_AXIS_TO_FLOAT(dvl);\
	_js_shcut_read_control(js_update_axis, index, fvl)
#define js_shcut_read_button(index)\
	dvl = hid_get_data(REP_BUF_DATA(jdev->report), &hitem);\
	_js_shcut_read_control(js_update_button, index, dvl)
#define js_shcut_read_hat(index)\
	dvl = hid_get_data(REP_BUF_DATA(jdev->report), &hitem);\
	_js_shcut_read_control(js_update_hat, index, dvl)

	while (read(jdev->fd, REP_BUF_DATA(jdev->report), jdev->report.size) == jdev->report.size) {
		int hat = 0;

#if defined (USBHID_NEW) || (defined (__FREEBSD__) && __FreeBSD_kernel_version >= 500111) || defined (__FreeBSD_kernel__)
		if ((hdata = hid_start_parse(jdev->repdesc, 1 << hid_input, jdev->report.id)) == NULL) {
#else
		if ((hdata = hid_start_parse(jdev->repdesc, 1 << hid_input)) == NULL) {
#endif
				continue;
		}

		while (hid_get_item(hdata, &hitem) > 0) {
			unsigned int usage = HID_USAGE(hitem.usage);
			unsigned int page = HID_PAGE(hitem.usage);
			SDBWORD dvl = 0;
			float fvl = 0;

			if (hitem.kind != hid_input) {
				continue;
			}

			if (page == HUP_GENERIC_DESKTOP) {
				int index;

				if (usage == HUG_HAT_SWITCH) {
					if (hat >= JS_HATS) {
						hat++;
						continue;
					}
					js_shcut_read_hat(hat);
					hat++;
					continue;
				}

				switch ((index = js_usage_to_axis(usage))) {
					case JS_DPAD_UP:
					case JS_DPAD_DOWN:
					case JS_DPAD_LEFT:
					case JS_DPAD_RIGHT:
						js_shcut_read_button((index - JS_DPAD_UP + jdev->info.buttons))
						break;
					case JS_AXE_X:
					case JS_AXE_Y:
					case JS_AXE_RX:
					case JS_AXE_RY:
					case JS_AXE_Z:
					case JS_AXE_RZ:
						js_shcut_read_axis(index)
						break;
					case JS_AXE_SLIDER:
					case JS_AXE_WHEEL:
						break;
					default:
						break;
				}
			} else if (page == HUP_BUTTON) {
				js_shcut_read_button((hitem.usage & 0x0F))
			}
		}
		hid_end_parse(hdata);
	}

#undef _js_shcut_read_control
#undef js_shcut_read_axis
#undef js_shcut_read_button
#undef js_shcut_read_hat

	js_unlock();

	return (EXIT_ERROR);
}

static _usb_hid_device *usb_alloc_device(void) {
	_usb_hid_device *udev;

	udev = malloc(sizeof(_usb_hid_device));

	udev->desc = NULL;
	udev->serial = NULL;

	udev->flags = JS_SC_NONE;

	return (udev);
}
static void usb_free_device(_usb_hid_device *udev) {
	if (udev->desc) {
		free(udev->desc);
		udev->desc = NULL;
	}
	if (udev->serial) {
		free(udev->serial);
		udev->serial = NULL;
	}

	free(udev);
}
static void usb_free_udd(void) {
	if (jstick.udd.devices) {
		int i;

		for (i = 0; i < jstick.udd.count; i++) {
			_usb_hid_device *udev = jstick.udd.devices[i];

			usb_free_device(udev);
		}

		free(jstick.udd.devices);
		jstick.udd.devices = NULL;
	}

	jstick.udd.count = 0;
}
static void usb_detect_devices(void) {
	int i;

	usb_free_udd();

	for (i = 0; i < JS_MAX_UHID_DEV; i++) {
		struct usb_device_info udi;
		uTCHAR dev[30];
		int a, fd;

		usnprintf(dev, usizeof(dev), uL("" USB_DEV_PATH "%d"), i);

		if ((fd = uopen(dev, O_RDONLY | O_NONBLOCK)) < 0) {
			continue;
		}

		for (a = 1; a < USB_MAX_DEVICES; a++) {
			BYTE vendor = FALSE;
			udi.udi_addr = a;
			int b, size;

			if (ioctl(fd, USB_DEVICEINFO, &udi) == -1) {
				continue;
			}

			for (b = 0; b < USB_MAX_DEVNAMES; b++) {
				_usb_hid_device *udev;
				_usb_hid_device **devices;
				unsigned int c;

				if (ustrncmp(udi.udi_devnames[b], uL("uhidev"), 6) != 0) {
					continue;
				}

				udev = usb_alloc_device();

#if defined (__NetBSD__)
				{
					usb_device_descriptor_t udd;
					struct usb_string_desc usd;

					if (ioctl(fd, USB_GET_DEVICE_DESC, &udd) != -1) {
						// Get default language
						usd.usd_string_index = USB_LANGUAGE_TABLE;
						usd.usd_language_id = 0;

						if ((ioctl(fd, USB_GET_STRING_DESC, &usd) == -1) || (usd.usd_desc.bLength < 4)) {
							usd.usd_language_id = 0;
						} else {
							usd.usd_language_id = UGETW(usd.usd_desc.bString[0]);
						}

						usd.usd_string_index = udd.iProduct;

						if (ioctl(fd, USB_GET_STRING_DESC, &usd) == 0) {
							char str[128];
							int d;

							for (d = 0; d < (usd.usd_desc.bLength >> 1) - 1 && d < sizeof(str) - 1; d++) {
								str[d] = UGETW(usd.usd_desc.bString[d]);
							}
							str[d] = '\0';

							asprintf(&udev->desc, "%s", str);
						}
					}
				}
#else
				size = ustrlen(udi.udi_product) + 1;

				if (ustrlen(udi.udi_vendor) && ustrncmp(udi.udi_vendor, uL("vendor 0x"), 9) != 0) {
					size += ustrlen(udi.udi_vendor) + 1;
					vendor = TRUE;
				}

				if (size > 1) {
					if ((udev->desc = (uTCHAR *)malloc(size * sizeof(uTCHAR))) == NULL) {
						fprintf(stderr, "%s: out of memory\n", dev);
						goto usb_detect_devices_error;
					}

					umemset(udev->desc, 0x00, size);

					if (vendor == TRUE) {
						usnprintf(udev->desc, size, uL("" uPERCENTs " " uPERCENTs), udi.udi_vendor, udi.udi_product);
					} else {
						usnprintf(udev->desc, size, uL("" uPERCENTs), udi.udi_product);
					}
				}

				if ((size = ustrlen(udi.udi_serial) + 1) > 1) {
					if ((udev->serial = (uTCHAR *)malloc(size * sizeof(uTCHAR))) == NULL) {
						fprintf(stderr, "%s: out of memory\n", dev);
						goto usb_detect_devices_error;
					}

					umemset(udev->serial, 0x00, size);

					usnprintf(udev->serial, size, uL("" uPERCENTs), udi.udi_serial);
				}
#endif

				// special case
				for (c = 0; c < LENGTH(js_special_case); c++) {
					if ((udi.udi_vendorNo == js_special_case[c].vendor) &&
						(udi.udi_productNo == js_special_case[c].product)) {
						udev->flags |= js_special_case[c].flags;
					}
				}

				if ((devices = (_usb_hid_device **)realloc(jstick.udd.devices,
					(jstick.udd.count + 1) * sizeof(_usb_hid_device *))) == NULL) {
					fprintf(stderr, "%s: out of memory\n", dev);
					goto usb_detect_devices_error;
				}

				jstick.udd.devices = devices;
				jstick.udd.devices[jstick.udd.count] = udev;
				jstick.udd.count++;
				continue;

				usb_detect_devices_error:
				usb_free_device(udev);
			}
		}
		close(fd);
	}
}

static _js_device *js_alloc_device(void) {
	_js_device *jdev;
	int i;

	jdev = malloc(sizeof(_js_device));

	jdev->type = JS_TYPE_UNKNOW;
	jdev->fd = 0;
	jdev->repdesc = NULL;
	jdev->report.buf = NULL;
	jdev->report.id = -1;
	jdev->report.size = 0;
	jdev->info.axes = 0;
	jdev->info.buttons = 0;
	jdev->info.hats = 0;
	jdev->info.balls = 0;

	umemset(&jdev->dev, 0x00, usizeof(jdev->dev));

	for (i = 0; i < JS_AXES; i++) {
		jdev->axis[i].used = FALSE;
		jdev->axis[i].min = 0.0f;
		jdev->axis[i].max = 0.0f;
		jdev->axis[i].center = 0.0f;
	}

	for (i = 0; i < JS_HATS; i++) {
		jdev->hat[i].used = FALSE;
		jdev->hat[i].min = 0.0f;
		jdev->hat[i].max = 0.0f;
	}

	for (i = 0; i < JS_ST_MAX; i++) {
		jdev->states[i].axis = NULL;
		jdev->states[i].hat = NULL;
		jdev->states[i].button = NULL;
	};

#if defined (__FREEBSD__) || defined (USBHID_MACHINE_JOYSTICK) || defined (__FreeBSD_kernel__)
	jdev->desc = NULL;
#endif
	jdev->usb = NULL;

	return (jdev);
}
static void js_free_device(_js_device *jdev) {
	int i;

	if (jdev->repdesc) {
		hid_dispose_report_desc(jdev->repdesc);
		jdev->repdesc = NULL;
	}
	if (jdev->report.buf) {
		free(jdev->report.buf);
		jdev->report.buf = NULL;
	}
	if (jdev->fd) {
		close(jdev->fd);
		jdev->fd = 0;
	}
	for (i = 0; i < JS_ST_MAX; i++) {
		if (jdev->states[i].axis) {
			free(jdev->states[i].axis);
			jdev->states[i].axis = NULL;
		}
		if (jdev->states[i].hat) {
			free(jdev->states[i].hat);
			jdev->states[i].hat = NULL;
		}
		if (jdev->states[i].button) {
			free(jdev->states[i].button);
			jdev->states[i].button = NULL;
		}
	}
#if defined (__FREEBSD__) || defined (USBHID_MACHINE_JOYSTICK) || defined (__FreeBSD_kernel__)
	if (jdev->desc) {
		free(jdev->desc);
		jdev->desc = NULL;
	}
#endif
	jdev->usb = NULL;

	free(jdev);
}
static void js_free_jdd(void) {
	if (jstick.jdd.devices) {
		int i;

		for (i = 0; i < jstick.jdd.count; i++) {
			_js_device *jdev = jstick.jdd.devices[i];

			js_free_device(jdev);
		}

		free(jstick.jdd.devices);
		jstick.jdd.devices = NULL;
	}

	jstick.jdd.count = 0;
}
static void js_detect_devices(void) {
	int i, hid_connected = 0;

	usb_detect_devices();

	js_free_jdd();

	for (i = 0; i < JS_MAX_DEV; i++) {
		static _js_device *jdev;
		struct hid_item hitem;
		struct hid_data *hdata;

		jdev = js_alloc_device();

		jdev->id = i;

#if defined (__FREEBSD__) || defined (USBHID_MACHINE_JOYSTICK) || defined (__FreeBSD_kernel__)
		if (jdev->id < JS_MAX_JOY_DEV) {
			usnprintf(jdev->dev, usizeof(jdev->dev), uL("" JOY_DEV_PATH "%d"), jdev->id);
			jdev->type = JS_TYPE_JOY;
		} else {
			usnprintf(jdev->dev, usizeof(jdev->dev), uL("" UHID_DEV_PATH "%d"), jdev->id - JS_MAX_JOY_DEV);
			jdev->type = JS_TYPE_UHID;
		}
#else
		usnprintf(jdev->dev, usizeof(jdev->dev), uL("" UHID_DEV_PATH "%d"), jdev->id);
		jdev->type = JS_TYPE_UHID;
#endif

		if ((jdev->fd = uopen(jdev->dev, O_RDONLY | O_NONBLOCK)) < 0) {
			goto js_detect_devices_error;
		}

#if defined (__FREEBSD__) || defined (USBHID_MACHINE_JOYSTICK) || defined (__FreeBSD_kernel__)
		if (jdev->type == JS_TYPE_JOY) {
			uTCHAR joystick_desc[] = uL("Gameport Joystick");

			jdev->desc = malloc(ustrlen(joystick_desc) + 1);
			umemcpy(jdev->desc, joystick_desc, ustrlen(joystick_desc));

			jdev->info.axes = 2;
			jdev->info.buttons = 2;
			jdev->info.hats = 0;
			jdev->info.balls = 0;

			goto js_detect_devices_ok;
		}
#endif

		if ((jdev->repdesc = hid_get_report_desc(jdev->fd)) == NULL) {
			fprintf(stderr, "%s: hid_get_report_desc: %s\n", jdev->dev, strerror(errno));
			goto js_detect_devices_error;
		}

#if defined (__FREEBSD__) && (__FreeBSD_kernel_version > 800063) || defined (__FreeBSD_kernel__)
		if ((jdev->report.id = hid_get_report_id(jdev->fd)) < 0) {
#else
		if (ioctl(jdev->fd, USB_GET_REPORT_ID, &jdev->report.id) < 0) {
#endif
			jdev->report.id = -1;
		}

#if defined (__DragonFly__)
		if ((jdev->report.size = hid_report_size(jdev->repdesc, jdev->report.id, hid_input)) <= 0) {
#elif defined (__FREEBSD__)
#if (__FreeBSD_kernel_version >= 460000) || defined (__FreeBSD_kernel__)
#if (__FreeBSD_kernel_version <= 500111)
		if ((jdev->report.size = hid_report_size(jdev->repdesc, jdev->report.id, hid_input)) <= 0) {
#else
		if ((jdev->report.size = hid_report_size(jdev->repdesc, hid_input, jdev->report.id)) <= 0) {
#endif
#else
		if ((jdev->report.size = hid_report_size(jdev->repdesc, hid_input, &jdev->report.id)) <= 0) {
#endif
#else
#if defined (USBHID_NEW)
		if ((jdev->report.size = hid_report_size(jdev->repdesc, hid_input, jdev->report.id)) <= 0) {
#else
		if ((jdev->report.size = hid_report_size(jdev->repdesc, hid_input, &jdev->report.id)) <= 0) {
#endif
#endif
			fprintf(stderr, "%s: hid_report_size() return invalid size\n", jdev->dev);
			goto js_detect_devices_error;
		}

#if defined (__FREEBSD__) && (__FreeBSD_kernel_version > 900000)
		if ((jdev->report.buf = malloc(jdev->report.size)) == NULL) {
#else
		if ((jdev->report.buf = malloc(sizeof(*jdev->report.buf) - sizeof(REP_BUF_DATA(jdev->report)) + jdev->report.size)) == NULL) {
#endif
			fprintf(stderr, "%s: error on malloc report buf\n", jdev->dev);
			goto js_detect_devices_error;
		}

#if defined (USBHID_NEW) || (defined (__FREEBSD__) && __FreeBSD_kernel_version >= 500111) || defined (__FreeBSD_kernel__)
		if ((hdata = hid_start_parse(jdev->repdesc, 1 << hid_input, jdev->report.id)) == NULL) {
#else
		if ((hdata = hid_start_parse(jdev->repdesc, 1 << hid_input)) == NULL) {
#endif
			fprintf(stderr, "%s: hid_start_parse() don't start", jdev->dev);
			goto js_detect_devices_error;
		}

		while (hid_get_item(hdata, &hitem) > 0) {
			unsigned int usage = HID_USAGE(hitem.usage);
			unsigned int page = HID_PAGE(hitem.usage);

			switch (hitem.kind) {
				case hid_collection:
					if ((page == HUP_GENERIC_DESKTOP) &&
						((usage == HUG_JOYSTICK) || (usage == HUG_GAME_PAD))) {
						jdev->usb = jstick.udd.devices[hid_connected];
					}
					break;
				case hid_input:
					if (page == HUP_GENERIC_DESKTOP) {
						int index = js_usage_to_axis(usage);

						if (index >= 0) {
							jdev->axis[index].used = TRUE;
							jdev->axis[index].min = hitem.logical_minimum;
							jdev->axis[index].max = hitem.logical_maximum;
							if ((jdev->usb->flags & JS_SC_MS_XBOX_360_GAMEPD) &&
								((index == JS_AXE_Z) || (index == JS_AXE_RZ))) {
								jdev->axis[index].min = -hitem.logical_maximum;
							}
							jdev->axis[index].center = (jdev->axis[index].min / jdev->axis[index].max) * 0.5f;
							jdev->info.axes++;
						} else if (usage == HUG_HAT_SWITCH) {
							if (jdev->info.hats < JS_HATS) {
								jdev->hat[jdev->info.hats].used = TRUE;
								jdev->hat[jdev->info.hats].min = hitem.logical_minimum;
								jdev->hat[jdev->info.hats].max = hitem.logical_maximum;
								jdev->info.hats++;
							}
						}
					} else if (page == HUP_BUTTON) {
						jdev->info.buttons++;
					}
					break;
				default:
					break;
			}
		}
		hid_end_parse(hdata);

		hid_connected++;

		if ((jdev->info.axes == 0) && (jdev->info.buttons == 0) &&
			(jdev->info.hats == 0) && (jdev->info.balls == 0)) {
			fprintf(stderr, "%s: is not a joystick\n", jdev->dev);
			goto js_detect_devices_error;
		}

		// flush pending events
		while (read(jdev->fd, REP_BUF_DATA(jdev->report), jdev->report.size) == jdev->report.size) {
			;
		}

#if defined (__FREEBSD__) || defined (USBHID_MACHINE_JOYSTICK) || defined (__FreeBSD_kernel__)
		js_detect_devices_ok:
#endif
		{
			_js_device **devices;
			int a;

			for (a = 0; a < JS_ST_MAX; a++) {
				_js_last_states *st = &jdev->states[a];

				if (jdev->info.axes && ((st->axis = malloc(jdev->info.axes * sizeof(float))) == NULL)) {
					fprintf(stderr, "%s: error on malloc states axes\n", jdev->dev);
					goto js_detect_devices_error;
				}
				if (jdev->info.hats && ((st->hat = malloc(jdev->info.hats * sizeof(SDBWORD))) == NULL)) {
					fprintf(stderr, "%s: error on malloc states hats\n", jdev->dev);
					goto js_detect_devices_error;
				}
				if (jdev->info.buttons && ((st->button = malloc(jdev->info.buttons * sizeof(SDBWORD)))) == NULL) {
					fprintf(stderr, "%s: error on malloc states buttons\n", jdev->dev);
					goto js_detect_devices_error;
				}
				memset(st->axis, 0x00, jdev->info.axes * sizeof(float));
				memset(st->hat, 0x00, jdev->info.hats * sizeof(SDBWORD));
				memset(st->button, 0x00, jdev->info.buttons * sizeof(SDBWORD));
			}

			if ((devices = (_js_device **)realloc(jstick.jdd.devices,
				(jstick.jdd.count + 1) * sizeof(_js_device))) == NULL) {
				fprintf(stderr, "%s: out of memory\n", jdev->dev);
				goto js_detect_devices_error;
			}

			jstick.jdd.devices = devices;
			jstick.jdd.devices[jstick.jdd.count] = jdev;
			jstick.jdd.count++;

			jdev = jstick.jdd.devices[jstick.jdd.count - 1];

#if !defined (RELEASE)
			fprintf(stderr, "%d : %s @ %s\n",
				jstick.jdd.count,
				jdev->dev,
				jdev->usb->desc);
#endif

			continue;
		}

		js_detect_devices_error:
		js_free_device(jdev);
	}

	js_update_jdevs();

	jstick.ts_update_devices = gui_get_ms();
}
static void js_close_detected_devices(void) {
	usb_free_udd();
	js_free_jdd();
}

static void js_update_jdev(_js *joy, BYTE enable_decode, BYTE decode_index) {
	joy->jdev = NULL;
	joy->inited = FALSE;
	joy->input_decode_event = NULL;

	if (js_is_null(&joy->id) == FALSE) {
		int d;

		for (d = 0; d < jstick.jdd.count; d++) {
			if (joy->id == jstick.jdd.devices[d]->id) {
				joy->jdev = jstick.jdd.devices[d];
				break;
			}
		}

		joy->inited = TRUE;

		if (enable_decode) {
			joy->input_decode_event = port_funct[decode_index].input_decode_event;
		}
	}
}
static void js_update_jdevs(void) {
	int i;

	for (i = PORT1; i < PORT_MAX; i++) {
		js_update_jdev(&js[i], TRUE, i);
	}
	// shortcut
	js_update_jdev(&js_shcut, FALSE, 0);
}

INLINE static DBWORD js_update_axis(_js *joy, _port *port, enum _js_states st, int index, float value, BYTE *mode) {
	DBWORD event = (index << 1) + 1;
	float last_value;

	(*mode) = PRESSED;

	last_value = JDEV->states[st].axis[index];

	if ((value < JS_AXIS_SENSIBILITY) && (value > -JS_AXIS_SENSIBILITY)) {
		value = 0.0f;
		(*mode) = RELEASED;
		if (last_value > 0) {
			event++;
		}
	} else if (value <= - JS_AXIS_SENSIBILITY) {
		value = -1.0f;
	} else {
		value = 1.0f;
		event++;
	}

	JDEV->states[st].axis[index] = value;

	if (value != last_value) {
		if (joy->input_decode_event) {
			joy->input_decode_event((*mode), FALSE, event, JOYSTICK, port);
		}
		return (event);
	}
	return (0);
}
INLINE static DBWORD js_update_button(_js *joy, _port *port, enum _js_states st, int index, SDBWORD value, UNUSED(BYTE *mode)) {
	DBWORD event = 0;

	if (value != JDEV->states[st].button[index]) {
		JDEV->states[st].button[index] = value;
		event = index | 0x400;
		if (joy->input_decode_event) {
			joy->input_decode_event(value, FALSE, event, JOYSTICK, port);
		}
	}
	return (event);
}
INLINE static DBWORD js_update_hat(_js *joy, _port *port, enum _js_states st, int index, SDBWORD value, BYTE *mode) {
	float x = 0.0f, y = 0.0f;
	DBWORD event = 0;

	js_hat_to_xy(value, &x, &y);

	if (!(event = js_update_axis(joy, port, st, JS_HAT0_A + index, x, mode))) {
		event = js_update_axis(joy, port, st, JS_HAT0_B + index, y, mode);
	}
	return (event);
}

INLINE static void js_hat_to_xy(SDBWORD hat, float *x, float *y) {
	static const int hat_map[8] = {
		JS_HAT_UP,
		JS_HAT_UP_RIGHT,
		JS_HAT_RIGHT,
		JS_HAT_RIGHT_DOWN,
		JS_HAT_DOWN,
		JS_HAT_DOWN_LEFT,
		JS_HAT_LEFT,
		JS_HAT_LEFT_UP,
	};

	(*x) = (*y) = 0.0f;

	if ((hat & 0x000F) == 0x000F) {
		return;
	}

	if ((hat & 0x07) == hat) {
		switch (hat_map[hat]) {
			case JS_HAT_UP:
				(*x) = 1.0f;
				break;
			case JS_HAT_UP_RIGHT:
				(*x) = 1.0f;
				(*y) = 1.0f;
				break;
			case JS_HAT_RIGHT:
				(*y) = 1.0f;
				break;
			case JS_HAT_RIGHT_DOWN:
				(*x) = -1.0f;
				(*y) = 1.0f;
				break;
			case JS_HAT_DOWN:
				(*x) = -1.0f;
				break;
			case JS_HAT_DOWN_LEFT:
				(*x) = -1.0f;
				(*y) = -1.0f;
				break;
			case JS_HAT_LEFT:
				(*y) = -1.0f;
				break;
			case JS_HAT_LEFT_UP:
				(*x) = 1.0f;
				(*y) = -1.0f;
				break;
		}
	}
}
INLINE static int js_usage_to_axis(unsigned int usage) {
	int axis;

	switch (usage) {
		case HUG_X:
			axis = JS_AXE_X;
			break;
		case HUG_Y:
			axis = JS_AXE_Y;
			break;
		case HUG_Z:
			axis = JS_AXE_Z;
			break;
		case HUG_RX:
			axis = JS_AXE_RX;
			break;
		case HUG_RY:
			axis = JS_AXE_RY;
			break;
		case HUG_RZ:
			axis = JS_AXE_RZ;
			break;
		case HUG_SLIDER:
			axis = JS_AXE_SLIDER;
			break;
		case HUG_WHEEL:
			axis = JS_AXE_WHEEL;
			break;
		case 0x90:
			axis = JS_DPAD_UP;
			break;
		case 0x91:
			axis = JS_DPAD_DOWN;
			break;
		case 0x93:
			axis = JS_DPAD_LEFT;
			break;
		case 0x92:
			axis = JS_DPAD_RIGHT;
			break;
		default:
			axis = -1;
			break;
	}

	return (axis);
}

INLINE static void js_lock(void) {
	pthread_mutex_lock(&jstick.lock);
}
INLINE static void js_unlock(void) {
	pthread_mutex_unlock(&jstick.lock);
}
