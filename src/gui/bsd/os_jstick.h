/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

#ifndef OS_JSTICK_H_
#define OS_JSTICK_H_

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

#if defined (__FreeBSD__) || defined (__FreeBSD_kernel__)
#if !defined (__DragonFly__)
#include <osreldate.h>
#endif
#include <dev/usb/usb_ioctl.h>
#include <sys/joystick.h>
#endif

#if defined (USBHID_UCR_DATA)
#define REP_BUF_DATA(rep) (rep.buf->ucr_data)
#elif defined (__FreeBSD__)
#define REP_BUF_DATA(rep) (rep.buf)
#else
#define REP_BUF_DATA(rep) (rep.buf->data)
#endif

#if defined (__OpenBSD__)
#define HUG_D_PAD_UP        0x90
#define HUG_D_PAD_DOWN      0x91
#define HUG_D_PAD_RIGHT     0x92
#define HUG_D_PAD_LEFT      0x93
#endif

#define ABS_X               HUG_X
#define ABS_Y               HUG_Y
#define ABS_Z               HUG_Z
#define ABS_RX              HUG_RX
#define ABS_RY              HUG_RY
#define ABS_RZ              HUG_RZ
#define ABS_THROTTLE        HUG_SLIDER
#define ABS_WHEEL           HUG_WHEEL
#define ABS_HAT0X           (HUG_HAT_SWITCH + 0) | 0x800
#define ABS_HAT0Y           (HUG_HAT_SWITCH + 1) | 0x800
#define ABS_HAT1X           (HUG_HAT_SWITCH + 2) | 0x800
#define ABS_HAT1Y           (HUG_HAT_SWITCH + 3) | 0x800
#define ABS_HAT2X           (HUG_HAT_SWITCH + 4) | 0x800
#define ABS_HAT2Y           (HUG_HAT_SWITCH + 5) | 0x800
#define ABS_HAT3X           (HUG_HAT_SWITCH + 6) | 0x800
#define ABS_HAT3Y           (HUG_HAT_SWITCH + 7) | 0x800
#define ABS_RUDDER          HUG_DIAL
#define ABS_GAS             0xFFFFFF
#define ABS_BRAKE           0xFFFFFF
#define ABS_PRESSURE        0xFFFFFF
#define ABS_DISTANCE        0xFFFFFF
#define ABS_TILT_X          0xFFFFFF
#define ABS_TILT_Y          0xFFFFFF
#define ABS_TOOL_WIDTH      0xFFFFFF

#define BTN_A               1
#define BTN_B               2
#define BTN_X               3
#define BTN_Y               4
#define BTN_TL              5
#define BTN_TR              6
#define BTN_SELECT          7
#define BTN_START           8
#define BTN_MODE            9
#define BTN_THUMBL          10
#define BTN_THUMBR          11
#define BTN_DPAD_UP         12
#define BTN_DPAD_DOWN       13
#define BTN_DPAD_LEFT       14
#define BTN_DPAD_RIGHT      15
#define BTN_TL2             16
#define BTN_TR2             17
#define BTN_C               18
#define BTN_Z               19
#define BTN_TRIGGER         20
#define BTN_THUMB           21
#define BTN_THUMB2          22
#define BTN_TOP             23
#define BTN_TOP2            24
#define BTN_PINKIE          25
#define BTN_BASE            26
#define BTN_BASE2           27
#define BTN_BASE3           28
#define BTN_BASE4           29
#define BTN_BASE5           30
#define BTN_BASE6           31
#define BTN_DEAD            32
#define BTN_GEAR_DOWN       33
#define BTN_GEAR_UP         34
#define BTN_TRIGGER_HAPPY   35
#define BTN_TRIGGER_HAPPY1  36
#define BTN_TRIGGER_HAPPY2  37
#define BTN_TRIGGER_HAPPY3  38
#define BTN_TRIGGER_HAPPY4  39
#define BTN_TRIGGER_HAPPY5  40
#define BTN_TRIGGER_HAPPY6  41
#define BTN_TRIGGER_HAPPY7  42
#define BTN_TRIGGER_HAPPY8  43
#define BTN_TRIGGER_HAPPY9  44
#define BTN_TRIGGER_HAPPY10 45
#define BTN_TRIGGER_HAPPY11 46
#define BTN_TRIGGER_HAPPY12 47
#define BTN_TRIGGER_HAPPY13 48
#define BTN_TRIGGER_HAPPY14 49
#define BTN_TRIGGER_HAPPY15 50
#define BTN_TRIGGER_HAPPY16 51
#define BTN_TRIGGER_HAPPY17 52
#define BTN_TRIGGER_HAPPY18 53
#define BTN_TRIGGER_HAPPY19 54
#define BTN_TRIGGER_HAPPY20 55
#define BTN_TRIGGER_HAPPY21 56
#define BTN_TRIGGER_HAPPY22 57
#define BTN_TRIGGER_HAPPY23 58
#define BTN_TRIGGER_HAPPY24 59
#define BTN_TRIGGER_HAPPY25 60
#define BTN_TRIGGER_HAPPY26 61
#define BTN_TRIGGER_HAPPY27 62
#define BTN_TRIGGER_HAPPY28 63
#define BTN_TRIGGER_HAPPY29 64
#define BTN_TRIGGER_HAPPY30 65
#define BTN_TRIGGER_HAPPY31 66
#define BTN_TRIGGER_HAPPY32 67
#define BTN_TRIGGER_HAPPY33 68
#define BTN_TRIGGER_HAPPY34 69
#define BTN_TRIGGER_HAPPY35 70
#define BTN_TRIGGER_HAPPY36 71
#define BTN_TRIGGER_HAPPY37 72
#define BTN_TRIGGER_HAPPY38 73
#define BTN_TRIGGER_HAPPY39 74
#define BTN_TRIGGER_HAPPY40 75

#endif /* OS_JSTICK_H_ */
