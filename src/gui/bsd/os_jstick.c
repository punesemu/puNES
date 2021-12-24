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

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "gui.h"
#include "conf.h"

#define UHID_DEV_PATH "/dev/uhid"
#define UJOY_DEV_PATH "/dev/ujoy/"
#define USB_DEV_PATH "/dev/usb"

enum _js_bsd_hat {
	JS_OS_HAT_UP,
	JS_OS_HAT_UP_RIGHT,
	JS_OS_HAT_RIGHT,
	JS_OS_HAT_RIGHT_DOWN,
	JS_OS_HAT_DOWN,
	JS_OS_HAT_DOWN_LEFT,
	JS_OS_HAT_LEFT,
	JS_OS_HAT_LEFT_UP,
};

typedef struct _js_usb_hid_device {
	uTCHAR desc[128];
	WORD bustype;
	WORD vendor_id;
	WORD product_id;
	WORD version;
	unsigned int flags;
} _js_usb_hid_device;

INLINE static BYTE is_abs_y_xinput(_js_device *jdev, _js_axis *jsx);
INLINE static void usb_dev_scan(void);
INLINE static void usb_dev_info(_js_usb_hid_device *uhdev, struct usb_device_info *udi);
INLINE static void hat_validate(_js_device *jdev, int index, SDBWORD value);
INLINE static void hat_to_xy(SDBWORD hat, SDBWORD *x, SDBWORD *y);

struct _js_os {
	struct _usb_devices_detected {
		int count;
		_js_usb_hid_device devices[MAX_JOYSTICK];
	} udd;
} js_os;

void js_os_init(UNUSED(BYTE first_time)) {}
void js_os_quit(UNUSED(BYTE last_time)) {}
void js_os_jdev_init(_js_device *jdev) {
	jdev->fd = -1;
	umemset(jdev->dev, 0x00, usizeof(jdev->dev));

	jdev->hug_d_pad_state = 0;

	jdev->repdesc = NULL;
	jdev->report.buf = NULL;
	jdev->report.id = -1;
	jdev->report.size = 0;
}
void js_os_jdev_open(_js_device *jdev, UNUSED(void *arg)) {
	struct hid_item hitem;
	struct hid_data *hdata;
	BYTE is_a_joystick = FALSE;

	js_jdev_init(jdev);

#if defined (__OpenBSD__)
	usnprintf(jdev->dev, usizeof(jdev->dev), uL("" UJOY_DEV_PATH "%d"), jdev->index);
#else
	usnprintf(jdev->dev, usizeof(jdev->dev), uL("" UHID_DEV_PATH "%d"), jdev->index);
#endif
	if ((jdev->fd = uopen(jdev->dev, O_RDONLY | O_NONBLOCK)) < 0) {
		return;
	}

	{
		_js_usb_hid_device *uhdev = NULL;
#if defined (__FreeBSD__)
		struct usb_device_info udi;
		_js_usb_hid_device uhd;

		if (ioctl(jdev->fd, USB_GET_DEVICEINFO, &udi) != -1) {
			usb_dev_info(&uhd, &udi);
			uhdev = &uhd;
		} else {
			ustrncpy(jdev->desc, uL("Unknow"), usizeof(jdev->desc) - 1);
		}
#else
		uhdev = &js_os.udd.devices[jdev->index];
#endif
		if (uhdev) {
			umemcpy(&jdev->desc, uhdev->desc, usizeof(jdev->desc) - 1);
			jdev->usb.bustype = uhdev->bustype;
			jdev->usb.vendor_id = uhdev->vendor_id;
			jdev->usb.product_id = uhdev->product_id;
			jdev->usb.version = uhdev->version;
		} else {
			ustrncpy(jdev->desc, uL("Unknow"), usizeof(jdev->desc) - 1);
		}
	}

	if ((jdev->repdesc = hid_get_report_desc(jdev->fd)) == NULL) {
		fprintf(stderr, "%s: hid_get_report_desc: %s\n", jdev->dev, strerror(errno));
		js_os_jdev_close(jdev);
		return;
	}

	js_jdev_type(jdev);
	jdev->is_xinput = js_jdev_is_xinput(jdev);

#if defined (__FreeBSD__)
	if ((jdev->report.id = hid_get_report_id(jdev->fd)) < 0) {
		jdev->report.id = -1;
	}
#else
	if (ioctl(jdev->fd, USB_GET_REPORT_ID, &jdev->report.id) < 0) {
		jdev->report.id = -1;
	}
#endif

#if defined (__DragonFly__)
	jdev->report.size = hid_report_size(jdev->repdesc, jdev->report.id, hid_input);
#elif defined (__FreeBSD__)
	jdev->report.size = hid_report_size(jdev->repdesc, hid_input, jdev->report.id);
#elif defined (USBHID_NEW)
	jdev->report.size = hid_report_size(jdev->repdesc, hid_input, jdev->report.id);
#else
	jdev->report.size = hid_report_size(jdev->repdesc, hid_input, &jdev->report.id);
#endif
	if (jdev->report.size <= 0) {
		fprintf(stderr, "%s: hid_report_size() return invalid size\n", jdev->dev);
		js_os_jdev_close(jdev);
		return;
	}

#if defined (__FreeBSD__)
	jdev->report.buf = malloc(jdev->report.size);
#else
	jdev->report.buf = malloc(sizeof(*jdev->report.buf) - sizeof(REP_BUF_DATA(jdev->report)) + jdev->report.size);
#endif
	if (jdev->report.buf == NULL) {
		fprintf(stderr, "%s: error on malloc report buf\n", jdev->dev);
		js_os_jdev_close(jdev);
		return;
	}

#if defined (USBHID_NEW) || defined (__FreeBSD__) || defined (__FreeBSD_kernel__)
	hdata = hid_start_parse(jdev->repdesc, 1 << hid_input, jdev->report.id);
#else
	hdata = hid_start_parse(jdev->repdesc, 1 << hid_input);
#endif
	if (hdata == NULL) {
		fprintf(stderr, "%s: hid_start_parse() don't start", jdev->dev);
		js_os_jdev_close(jdev);
		return;
	}

	while (hid_get_item(hdata, &hitem) > 0) {
		unsigned int usage = HID_USAGE(hitem.usage);
		unsigned int page = HID_PAGE(hitem.usage);

		switch (hitem.kind) {
			case hid_collection:
				if ((page == HUP_GENERIC_DESKTOP) && ((usage == HUG_JOYSTICK) || (usage == HUG_GAME_PAD))) {
					is_a_joystick = TRUE;
				}
				break;
			case hid_input:
				if (page == HUP_GENERIC_DESKTOP) {
					_js_axis *jsx;

					switch (usage) {
						case ABS_X:
						case ABS_Y:
						case ABS_Z:
						case ABS_RX:
						case ABS_RY:
						case ABS_RZ:
						case ABS_THROTTLE:
						case ABS_RUDDER:
						case ABS_WHEEL:
							jsx = &jdev->data.axis[jdev->info.axes];
							jsx->used = TRUE;
							jsx->offset = usage;
							jsx->min = is_abs_y_xinput(jdev, jsx) ? -hitem.logical_maximum : hitem.logical_minimum;
							jsx->max = is_abs_y_xinput(jdev, jsx) ? -hitem.logical_minimum : hitem.logical_maximum;
							jsx->center = 0;
							jsx->scale = (float)(JS_AXIS_MAX - JS_AXIS_MIN) / (float)(jsx->max - jsx->min);
							jdev->info.axes++;
							continue;
						case HUG_D_PAD_UP:
						case HUG_D_PAD_LEFT:
						case HUG_D_PAD_RIGHT:
							continue;
						case HUG_D_PAD_DOWN:
							usage = HUG_HAT_SWITCH;
							break;
						default:
							break;
					}
					if (usage == HUG_HAT_SWITCH) {
						if (jdev->info.hats < JS_MAX_HATS) {
							int a, index = jdev->info.hats * 2;
 							_js_axis *jsx;

							for (a = 0; a < 2; a++) {
								jsx = &jdev->data.hat[index + a];
								jsx->used = TRUE;
								jsx->offset = HUG_HAT_SWITCH + a | 0x800;
								jsx->min = JS_AXIS_MIN;
								jsx->max = JS_AXIS_MAX;
								jsx->center = 0;
								jsx->scale = (float)(JS_AXIS_MAX - JS_AXIS_MIN) / (float)(jsx->max - jsx->min);
								jsx->is_hat = TRUE;
							}
							jdev->info.hats++;
						}
					}
				} else if (page == HUP_BUTTON) {
					if (jdev->info.buttons < JS_MAX_BUTTONS) {
						_js_button *jsx = &jdev->data.button[jdev->info.buttons];

						jsx->used = TRUE;
						jsx->offset = usage;
						jdev->info.buttons++;
					}
				}
				break;
			default:
				break;
		}
	}
	hid_end_parse(hdata);

	if ((is_a_joystick == FALSE) && ((jdev->info.axes == 0) && (jdev->info.buttons == 0) && (jdev->info.hats == 0))) {
		fprintf(stderr, "%s: is not a joystick\n", jdev->dev);
		js_os_jdev_close(jdev);
		return;
	}

	js_guid_create(jdev);
	jdev->present = TRUE;
	jstick.jdd.count++;

	js_jdev_open_common(jdev);

#if defined (DEBUG)
	js_info_jdev(jdev);
#endif
}
void js_os_jdev_close(_js_device *jdev) {
	if (jdev == NULL) {
		return;
	}

	if (jdev->present == TRUE) {
		jstick.jdd.count--;
#if defined (DEBUG)
		ufprintf(stderr, uL("jstick disc. : slot%d \"" uPs("") "\" (%d)\n"),
			jdev->index,
			jdev->desc,
			jstick.jdd.count);
#endif
	}

	jdev->present = FALSE;
	jdev->is_xinput = FALSE;
	js_guid_unset(&jdev->guid);
	umemset(jdev->dev, 0x00, usizeof(jdev->dev));

	if (jdev->fd >= 0) {
		close(jdev->fd);
		jdev->fd = -1;
	}
	if (jdev->repdesc) {
		hid_dispose_report_desc(jdev->repdesc);
		jdev->repdesc = NULL;
	}
	if (jdev->report.buf) {
		free(jdev->report.buf);
		jdev->report.buf = NULL;
	}
}
void js_os_jdev_scan(void) {
	int i;

	usb_dev_scan();

	for (i = 0; i < MAX_JOYSTICK; i++) {
		_js_device *jdev = &jstick.jdd.devices[i];

		thread_mutex_lock(jdev->lock);

		if (jdev->present == FALSE) {
			js_os_jdev_open(jdev, NULL);
		}

		thread_mutex_unlock(jdev->lock);
	}
}
void js_os_jdev_read_events_loop(_js_device *jdev) {
	int i;

	while (read(jdev->fd, REP_BUF_DATA(jdev->report), jdev->report.size) == jdev->report.size) {
		struct hid_item hitem;
		struct hid_data *hdata;
		int hat = 0;

#if defined (USBHID_NEW) || defined (__FreeBSD__) || defined (__FreeBSD_kernel__)
		hdata = hid_start_parse(jdev->repdesc, 1 << hid_input, jdev->report.id);
#else
		hdata = hid_start_parse(jdev->repdesc, 1 << hid_input);
#endif
		if (hdata == NULL) {
			continue;
		}

		while (hid_get_item(hdata, &hitem) > 0) {
			unsigned int usage = HID_USAGE(hitem.usage);
			unsigned int page = HID_PAGE(hitem.usage);
			BYTE read_hat_value = TRUE;
			SDBWORD value;

			if (hitem.kind != hid_input) {
				continue;
			}

			if (page == HUP_GENERIC_DESKTOP) {
				if ((usage >= HUG_D_PAD_UP) && (usage <= HUG_D_PAD_LEFT)) {
					value = !!hid_get_data(REP_BUF_DATA(jdev->report), &hitem);

					if (usage == HUG_D_PAD_UP) {
						jdev->hug_d_pad_state = (jdev->hug_d_pad_state & 0x0E) | (value << 0);
					} else if (usage == HUG_D_PAD_DOWN) {
						jdev->hug_d_pad_state = (jdev->hug_d_pad_state & 0x0D) | (value << 1);
					} else if (usage == HUG_D_PAD_LEFT) {
						jdev->hug_d_pad_state = (jdev->hug_d_pad_state & 0x0B) | (value << 2);
					} else if (usage == HUG_D_PAD_RIGHT) {
						jdev->hug_d_pad_state = (jdev->hug_d_pad_state & 0x07) | (value << 3);
					}

					switch (jdev->hug_d_pad_state) {
						default:
						case 0:
							value = 0x0F;
							break;
						case 1:
							value = JS_OS_HAT_UP;
							break;
						case 9:
							value = JS_OS_HAT_UP_RIGHT;
							break;
						case 8:
							value = JS_OS_HAT_RIGHT;
							break;
						case 10:
							value = JS_OS_HAT_RIGHT_DOWN;
							break;
						case 2:
							value = JS_OS_HAT_DOWN;
							break;
						case 6:
							value = JS_OS_HAT_DOWN_LEFT;
							break;
						case 4:
							value = JS_OS_HAT_LEFT;
							break;
						case 5:
							value = JS_OS_HAT_LEFT_UP;
							break;
					}

					if (usage == HUG_D_PAD_DOWN) {
						read_hat_value = FALSE;
						usage = HUG_HAT_SWITCH;
					} else {
						continue;
					}
				}

				switch (usage) {
					case ABS_X:
					case ABS_Y:
					case ABS_Z:
					case ABS_RX:
					case ABS_RY:
					case ABS_RZ:
					case ABS_THROTTLE:
					case ABS_RUDDER:
					case ABS_WHEEL:
						for (i = 0; i < JS_MAX_AXES; i++) {
							_js_axis *jsx = &jdev->data.axis[i];

							if ((jsx->offset == usage) && jsx->used) {
								value = hid_get_data(REP_BUF_DATA(jdev->report), &hitem);
								js_axs_validate(jsx, is_abs_y_xinput(jdev, jsx) ? -value : value);
							}
						}
						continue;
					case HUG_HAT_SWITCH:
						if (hat < jdev->info.hats) {
							if (read_hat_value) {
								value = hid_get_data(REP_BUF_DATA(jdev->report), &hitem);
							}
							hat_validate(jdev, hat * 2, value);
							hat++;
						}
						continue;
					default:
						continue;
				}
			} else if (page == HUP_BUTTON) {
				for (i = 0; i < JS_MAX_BUTTONS; i++) {
					_js_button *jsx = &jdev->data.button[i];

					if ((jsx->offset == usage) && jsx->used) {
						value = hid_get_data(REP_BUF_DATA(jdev->report), &hitem);
						jsx->value = !!value;
					}
				}
			}
		}
		hid_end_parse(hdata);
	}

	if (errno != EAGAIN) {
		js_os_jdev_close(jdev);
		return;
	}
}

INLINE static BYTE is_abs_y_xinput(_js_device *jdev, _js_axis *jsx) {
	return (((jsx->offset == ABS_Y) || (jsx->offset == ABS_RY)) && jdev->is_xinput);
}
INLINE static void usb_dev_scan(void) {
#if !defined (__FreeBSD__)
	int i;

	js_os.udd.count = 0;

	for (i = 0; i < MAX_JOYSTICK; i++) {
		uTCHAR dev[30];
		int a, fd;

		usnprintf(dev, usizeof(dev), uL("" USB_DEV_PATH "%d"), i);

		if ((fd = uopen(dev, O_RDONLY | O_NONBLOCK)) < 0) {
			continue;
		}

		for (a = 1; a < USB_MAX_DEVICES; a++) {
			struct usb_device_info udi;
			int b;

			udi.udi_addr = a;

			if (ioctl(fd, USB_DEVICEINFO, &udi) == -1) {
				continue;
			}

			for (b = 0; b < USB_MAX_DEVNAMES; b++) {
				_js_usb_hid_device *uhidev = &js_os.udd.devices[js_os.udd.count];

				if (ustrncmp(udi.udi_devnames[b], uL("uhidev"), 6) != 0) {
					continue;
				}
				usb_dev_info(uhidev, &udi);
				js_os.udd.count++;
			}
		}
		close(fd);
	}
#endif
}
INLINE static void usb_dev_info(_js_usb_hid_device *uhdev, struct usb_device_info *udi) {
	BYTE vendor = FALSE;
	unsigned int size;

	umemset(uhdev->desc, 0x00, usizeof(uhdev->desc));

#if defined (__NetBSD__)
	{
		usb_device_descriptor_t uddesc;
		struct usb_string_desc usd;

		if (ioctl(fd, USB_GET_DEVICE_DESC, &uddesc) != -1) {
			// Get default language
			usd.usd_string_index = USB_LANGUAGE_TABLE;
			usd.usd_language_id = 0;

			if ((ioctl(fd, USB_GET_STRING_DESC, &usd) == -1) || (usd.usd_desc.bLength < 4)) {
				usd.usd_language_id = 0;
			} else {
				usd.usd_language_id = UGETW(usd.usd_desc.bString[0]);
			}

			usd.usd_string_index = uddesc.iProduct;

			if (ioctl(fd, USB_GET_STRING_DESC, &usd) == 0) {
				int i;

				for (i = 0; i < (usd.usd_desc.bLength >> 1) - 1 && i < sizeof(str) - 1; i++) {
					uhdev->desc[i] = UGETW(usd.usd_desc.bString[i]);
				}
				uhdev->desc[i] = '\0';
			}
		}
	}
#else
	size = ustrlen(udi->udi_product) + 1;

	if (ustrlen(udi->udi_vendor) && ustrncmp(udi->udi_vendor, uL("vendor 0x"), 9) != 0) {
		size += ustrlen(udi->udi_vendor) + 1;
		vendor = TRUE;
	}

	size = FHMIN(size, (int)(usizeof(uhdev->desc) - 1));

	if (size > 1) {
		if (vendor == TRUE) {
			usnprintf(uhdev->desc, size, uL("" uPs("") " " uPs("")), udi->udi_vendor, udi->udi_product);
		} else {
			usnprintf(uhdev->desc, size, uL("" uPs("")), udi->udi_product);
		}
	}
#endif
	uhdev->bustype = udi->udi_bus;
	uhdev->vendor_id = udi->udi_vendorNo;
	uhdev->product_id = udi->udi_productNo;
	uhdev->version = udi->udi_releaseNo;
}
INLINE static void hat_validate(_js_device *jdev, int index, SDBWORD value) {
	_js_axis *jsx = &jdev->data.hat[index];

	if (jsx->used) {
		SDBWORD x, y;

		hat_to_xy(value, &x, &y);
		js_axs_validate(jsx, x);
		jsx = &jdev->data.hat[index + 1];
		js_axs_validate(jsx, y);
	}
}
INLINE static void hat_to_xy(SDBWORD hat, SDBWORD *x, SDBWORD *y) {
	(*x) = (*y) = 0;
	if ((hat & 0x000F) == 0x000F) {
		return;
	}
	if ((hat & 0x07) == hat) {
		switch (hat) {
			case JS_OS_HAT_UP:
				(*y) = JS_AXIS_MIN;
				break;
			case JS_OS_HAT_UP_RIGHT:
				(*y) = JS_AXIS_MIN;
				(*x) = JS_AXIS_MAX;
				break;
			case JS_OS_HAT_RIGHT:
				(*x) = JS_AXIS_MAX;
				break;
			case JS_OS_HAT_RIGHT_DOWN:
				(*x) = JS_AXIS_MAX;
				(*y) = JS_AXIS_MAX;
				break;
			case JS_OS_HAT_DOWN:
				(*y) = JS_AXIS_MAX;
				break;
			case JS_OS_HAT_DOWN_LEFT:
				(*x) = JS_AXIS_MIN;
				(*y) = JS_AXIS_MAX;
				break;
			case JS_OS_HAT_LEFT:
				(*x) = JS_AXIS_MIN;
				break;
			case JS_OS_HAT_LEFT_UP:
				(*x) = JS_AXIS_MIN;
				(*y) = JS_AXIS_MIN;
				break;
		}
	}
}
