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

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libudev.h>
#include "gui.h"
#include "conf.h"

#define BITS_TO_LONGS (sizeof(unsigned long) * 8)
#define NBITS(x) ((((x) - 1) / BITS_TO_LONGS) + 1)
#define EVDEV_OFF(x) ((x) % BITS_TO_LONGS)
#define EVDEV_LONG(x) ((x) / BITS_TO_LONGS)
#define test_bit(bit, array) ((array[EVDEV_LONG(bit)] >> EVDEV_OFF(bit)) & 1)

struct _js_os {
	struct udev *udev;
} js_os;

void js_os_init(BYTE first_time) {
	if (first_time) {
		js_os.udev = udev_new();
	}
}
void js_os_quit(BYTE last_time) {
	if (last_time) {
		udev_unref(js_os.udev);
	}
}
void js_os_jdev_init(_js_device *jdev) {
	jdev->fd = -1;
	umemset(jdev->dev, 0x00, usizeof(jdev->dev));
}
void js_os_jdev_open(_js_device *jdev, void *arg) {
	unsigned long keybit[NBITS(KEY_CNT)] = { 0 };
	unsigned long absbit[NBITS(ABS_CNT)] = { 0 };
	struct udev_device *dev = (struct udev_device *)arg;
	unsigned int i;

	js_jdev_init(jdev);

	{
		const char *devnode = udev_device_get_devnode(dev);
		struct udev_device *udevd = dev;

		if (devnode == NULL) {
			return;
		}

		while (udevd && !udev_device_get_sysattr_value(udevd, "capabilities/ev")) {
			udevd = udev_device_get_parent_with_subsystem_devtype(udevd, "input", NULL);
		}
		if (udevd) {
			const char *btype = udev_device_get_sysattr_value(udevd, "id/bustype");
			const char *pid = udev_device_get_sysattr_value(udevd, "id/product");
			const char *vid = udev_device_get_sysattr_value(udevd, "id/vendor");
			const char *ver = udev_device_get_sysattr_value(udevd, "id/version");
			const char *name = udev_device_get_sysattr_value(udevd, "name");

			jdev->usb.bustype = strtoul(btype, NULL, 16);
			jdev->usb.vendor_id = strtoul(vid, NULL, 16);
			jdev->usb.product_id = strtoul(pid, NULL, 16);
			jdev->usb.version = strtoul(ver, NULL, 16);
			ustrncpy(jdev->desc, name, usizeof(jdev->desc) - 1);
		} else {
			return;
		}

		if (ustrlen(jdev->desc) == 0) {
			if (jdev->usb.vendor_id && jdev->usb.product_id) {
				usnprintf(jdev->desc, usizeof(jdev->desc), uL("Unknow%02d %04x:%04x"), jdev->index,
					jdev->usb.vendor_id,
					jdev->usb.product_id);
			} else {
				usnprintf(jdev->desc, usizeof(jdev->desc), uL("Controller%02d"), jdev->index);
			}
		}

		ustrncpy(jdev->dev, devnode, usizeof(jdev->dev) - 1);
	}

	if ((jdev->fd = uopen(jdev->dev, O_RDONLY | O_NONBLOCK)) < 0) {
		js_os_jdev_close(jdev);
		return;
	}

	js_jdev_type(jdev);
	jdev->is_xinput = js_jdev_is_xinput(jdev);

	if (ioctl(jdev->fd, EVIOCGBIT(EV_ABS, sizeof(absbit)), absbit) >= 0) {
		// axes
		for (i = 0; i < ABS_MAX; ++i) {
			if (jdev->info.axes < JS_MAX_AXES) {
				if (i == ABS_HAT0X) {
					i = ABS_HAT3Y;
					continue;
				}
				if (test_bit(i, absbit)) {
					struct input_absinfo absinfo;

					if (ioctl(jdev->fd, EVIOCGABS(i), &absinfo) >= 0) {
						_js_axis *jsx;

						jsx = &jdev->data.axis[jdev->info.axes];
						jsx->used = TRUE;
						jsx->offset = i;
						jsx->min = absinfo.minimum;
						jsx->max = absinfo.maximum;
						jsx->center = 0;
						jsx->scale = (float)(JS_AXIS_MAX - JS_AXIS_MIN) / (float)(jsx->max - jsx->min);
						jdev->info.axes++;
					} else {
						js_os_jdev_close(jdev);
					}
				}
			}
		}
		// hats
		for (i = ABS_HAT0X; i <= ABS_HAT3Y; i += 2) {
			if (jdev->info.hats < JS_MAX_HATS) {
				if (test_bit(i, absbit) || test_bit(i + 1, absbit)) {
					struct input_absinfo absinfo;

					if (ioctl(jdev->fd, EVIOCGABS(i), &absinfo) >= 0) {
						int a, index = jdev->info.hats * 2;
						_js_axis *jsx;

						for (a = 0; a < 2; a++) {
							jsx = &jdev->data.hat[index + a];
							jsx->used = TRUE;
							jsx->offset = i + a;
							jsx->min = absinfo.minimum;
							jsx->max = absinfo.maximum;
							jsx->center = 0;
							jsx->scale = (float)(JS_AXIS_MAX - JS_AXIS_MIN) / (float)(jsx->max - jsx->min);
							jsx->is_hat = TRUE;
						}
						jdev->info.hats++;
					} else {
						js_os_jdev_close(jdev);
					}
				}
			}
		}
	} else {
		js_os_jdev_close(jdev);
		return;
	}
	// pulsanti
	if ((ioctl(jdev->fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit) >= 0)) {
		for (i = BTN_JOYSTICK; i < KEY_MAX; i++) {
			if (jdev->info.buttons < JS_MAX_BUTTONS) {
				if (test_bit(i, keybit)) {
					_js_button *jsx = &jdev->data.button[jdev->info.buttons];

					jsx->used = TRUE;
					jsx->offset = i;
					jdev->info.buttons++;
				}
			}
		}
		for (i = 0; i < BTN_JOYSTICK; i++) {
			if (jdev->info.buttons < JS_MAX_BUTTONS) {
				if (test_bit(i, keybit)) {
					_js_button *jsx = &jdev->data.button[jdev->info.buttons];

					jsx->used = TRUE;
					jsx->offset = i;
					jdev->info.buttons++;
				}
			}
		}
	} else {
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
}
void js_os_jdev_scan(void) {
	struct udev_enumerate *enumerate = NULL;
	struct udev_list_entry *devs = NULL;
	struct udev_list_entry *item = NULL;

	enumerate = udev_enumerate_new(js_os.udev);

	udev_enumerate_add_match_subsystem(enumerate, "input");
	udev_enumerate_scan_devices(enumerate);
	devs = udev_enumerate_get_list_entry(enumerate);

	for (item = devs; item; item = udev_list_entry_get_next(item)) {
		const char *path = udev_list_entry_get_name(item);
		struct udev_device *dev = udev_device_new_from_syspath(js_os.udev, path);

		if (dev) {
			const char *devnode = udev_device_get_devnode(dev);
			int i;

			if (devnode) {
				const char *val = NULL;
				BYTE finded = FALSE;

				val = udev_device_get_property_value(dev, "ID_INPUT_JOYSTICK");
				if (val && strcmp(val, "1") == 0) {
					finded = TRUE;
				}
				val = udev_device_get_property_value(dev, "ID_INPUT_ACCELEROMETER");
				if (val && strcmp(val, "1") == 0) {
					finded = FALSE;
				}
				if (finded == FALSE) {
					val = udev_device_get_property_value(dev, "ID_CLASS");
					if (val && strcmp(val, "joystick") == 0) {
						finded = TRUE;
					}
				}
				if (finded) {
					_js_device *jdev = NULL;

					for (i = 0; i < MAX_JOYSTICK; i++) {
						_js_device *jd = &jstick.jdd.devices[i];

						thread_mutex_lock(jd->lock);

						if (ustrncmp(jd->dev, devnode, usizeof(jd->dev)) != 0) {
							if (!jdev && (jd->present == FALSE)) {
								jdev = jd;
							}
							thread_mutex_unlock(jd->lock);
							continue;
						}
						thread_mutex_unlock(jd->lock);
						jdev = NULL;
						break;
					}
					if (jdev && (jstick.jdd.count < MAX_JOYSTICK)) {
						thread_mutex_lock(jdev->lock);
						js_os_jdev_open(jdev, (void *)dev);
						thread_mutex_unlock(jdev->lock);
					}
				}
			}
			udev_device_unref(dev);
		}
	}
	udev_enumerate_unref(enumerate);
}
void js_os_jdev_read_events_loop(_js_device *jdev) {
	unsigned long keyinfo[NBITS(KEY_MAX)] = { 0 };
	struct input_absinfo absinfo;
	unsigned int i, a;

	// axes e hats
	for (i = 0; i < LENGTH(js_axs_type); i++) {
		for (a = 0; a < js_axs_type[i]; a++) {
			_js_axis *jsx = !i ? &jdev->data.axis[a] : &jdev->data.hat[a];

			if (jsx->used) {
				int rc = ioctl(jdev->fd, EVIOCGABS(jsx->offset), &absinfo);

				if (rc < 0) {
					js_os_jdev_close(jdev);
					return;
				}
				js_axs_validate(jsx, absinfo.value);
			}
		}
	}
	// pulsanti
	if (ioctl(jdev->fd, EVIOCGKEY(sizeof(keyinfo)), keyinfo) >= 0) {
		for (i = 0; i < JS_MAX_BUTTONS; i++) {
			_js_button *jsx = &jdev->data.button[i];

			if (jsx->used) {
				jsx->value = test_bit(jsx->offset, keyinfo);
			}
		}
	} else {
		js_os_jdev_close(jdev);
		return;
	}
}
