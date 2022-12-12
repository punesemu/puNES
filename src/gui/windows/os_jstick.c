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

//#define _WIN32_WINNT 0x0501
#define INITGUID
#define DIRECTINPUT_VERSION 0x0800

#include <windows.h>
#include <dinput.h>
#include <xinput.h>
#include "jstick.h"
#include "conf.h"
#include "gui.h"

enum _js_windows_xinput {
	JS_XINPUT_AXIS_MIN = -32768,

	JS_XINPUT_STHUMBLX = ABS_X,
	JS_XINPUT_STHUMBLY = ABS_Y,
	JS_XINPUT_BLEFTTRIGGER = ABS_Z,
	JS_XINPUT_STHUMBRX = ABS_RX,
	JS_XINPUT_STHUMBRY = ABS_RY,
	JS_XINPUT_BRIGHTTRIGGER = ABS_RZ,

	JS_XINPUT_GAMEPAD_A = BTN_A - 1,
	JS_XINPUT_GAMEPAD_B = BTN_B - 1,
	JS_XINPUT_GAMEPAD_X = BTN_X - 1,
	JS_XINPUT_GAMEPAD_Y = BTN_Y - 1,
	JS_XINPUT_GAMEPAD_LEFT_SHOULDER = BTN_TL - 1,
	JS_XINPUT_GAMEPAD_RIGHT_SHOULDER = BTN_TR - 1,
	JS_XINPUT_GAMEPAD_BACK = BTN_SELECT - 1,
	JS_XINPUT_GAMEPAD_START = BTN_START - 1,
	JS_XINPUT_GAMEPAD_GUIDE = BTN_MODE - 1,
	JS_XINPUT_GAMEPAD_LEFT_THUMB = BTN_THUMBL - 1,
	JS_XINPUT_GAMEPAD_RIGHT_THUMB = BTN_THUMBR - 1,
	JS_XINPUT_GAMEPAD_11 = BTN_B - 1,
	JS_XINPUT_GAMEPAD_DPAD_UP = BTN_DPAD_UP - 1,
	JS_XINPUT_GAMEPAD_DPAD_DOWN = BTN_DPAD_DOWN - 1,
	JS_XINPUT_GAMEPAD_DPAD_LEFT = BTN_DPAD_LEFT - 1,
	JS_XINPUT_GAMEPAD_DPAD_RIGHT = BTN_DPAD_RIGHT - 1,
};
enum _js_windows_hat {
	JS_OS_HAT_UP = 0,
	JS_OS_HAT_RIGHT = 9000,
	JS_OS_HAT_DOWN = 18000,
	JS_OS_HAT_LEFT = 27000
};

#define JDEVIDID8W (IDirectInputDevice8W *)jdev->di8device
#define INPUT_QUEUE_SIZE 32
#define XINPUT_GAMEPAD_GUIDE 0x400

typedef struct XINPUT_GAMEPAD_EX {
	WORD wButtons;
	BYTE bLeftTrigger;
	BYTE bRightTrigger;
	SHORT sThumbLX;
	SHORT sThumbLY;
	SHORT sThumbRX;
	SHORT sThumbRY;
	DWORD dwPaddingReserved;
} XINPUT_GAMEPAD_EX;
typedef struct XINPUT_STATE_EX {
	DWORD dwPacketNumber;
	XINPUT_GAMEPAD_EX Gamepad;
} XINPUT_STATE_EX;
typedef struct _js_raw_devices {
	UINT count;
	PRAWINPUTDEVICELIST list;
} _js_raw_devices;

INLINE static void hat_to_xy(DWORD hat, SDBWORD *x, SDBWORD *y);

static BOOL CALLBACK cb_enum_dev(LPCDIDEVICEINSTANCEW instance, LPVOID context);
static BOOL CALLBACK cb_enum_obj(LPCDIDEVICEOBJECTINSTANCEW instance, LPVOID context);

DEFINE_GUID(IID_ZeroGUID,              MAKELONG(0x0000, 0x0000), 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
//DEFINE_GUID(IID_ValveStreamingGamepad, MAKELONG(0x28DE, 0x11FF), 0x0000, 0x0000, 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44);
//DEFINE_GUID(IID_Dualshock4,            MAKELONG(0x054C, 0x05C4), 0x0000, 0x0000, 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44);

DIOBJECTDATAFORMAT dfDIJoystick2[] = {
	{ &GUID_XAxis,  DIJOFS_X,                                 DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTPOSITION },
	{ &GUID_YAxis,  DIJOFS_Y,                                 DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTPOSITION },
	{ &GUID_ZAxis,  DIJOFS_Z,                                 DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTPOSITION },
	{ &GUID_RxAxis, DIJOFS_RX,                                DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTPOSITION },
	{ &GUID_RyAxis, DIJOFS_RY,                                DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTPOSITION },
	{ &GUID_RzAxis, DIJOFS_RZ,                                DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTPOSITION },
	{ &GUID_Slider, DIJOFS_SLIDER(0),                         DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTPOSITION },
	{ &GUID_Slider, DIJOFS_SLIDER(1),                         DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTPOSITION },
	{ &GUID_POV,    DIJOFS_POV(0),                            DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE,    0 },
	{ &GUID_POV,    DIJOFS_POV(1),                            DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE,    0 },
	{ &GUID_POV,    DIJOFS_POV(2),                            DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE,    0 },
	{ &GUID_POV,    DIJOFS_POV(3),                            DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE,    0 },
	{ NULL,         DIJOFS_BUTTON(0),                         DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(1),                         DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(2),                         DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(3),                         DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(4),                         DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(5),                         DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(6),                         DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(7),                         DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(8),                         DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(9),                         DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(10),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(11),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(12),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(13),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(14),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(15),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(16),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(17),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(18),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(19),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(20),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(21),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(22),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(23),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(24),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(25),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(26),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(27),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(28),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(29),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(30),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(31),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(32),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(33),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(34),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(35),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(36),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(37),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(38),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(39),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(40),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(41),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(42),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(43),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(44),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(45),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(46),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(47),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(48),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(49),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(50),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(51),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(52),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(53),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(54),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(55),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(56),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(57),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(58),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(59),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(60),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(61),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(62),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(63),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(64),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(65),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(66),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(67),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(68),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(69),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(70),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(71),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(72),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(73),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(74),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(75),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(76),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(77),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(78),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(79),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(80),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(81),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(82),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(83),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(84),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(85),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(86),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(87),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(88),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(89),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(90),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(91),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(92),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(93),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(94),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(95),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(96),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(97),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(98),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(99),                        DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(100),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(101),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(102),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(103),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(104),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(105),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(106),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(107),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(108),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(109),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(110),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(111),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(112),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(113),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(114),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(115),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(116),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(117),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(118),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(119),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(120),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(121),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(122),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(123),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(124),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(125),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(126),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ NULL,         DIJOFS_BUTTON(127),                       DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
	{ &GUID_XAxis,  FIELD_OFFSET(DIJOYSTATE2, lVX),           DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTVELOCITY },
	{ &GUID_YAxis,  FIELD_OFFSET(DIJOYSTATE2, lVY),           DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTVELOCITY },
	{ &GUID_ZAxis,  FIELD_OFFSET(DIJOYSTATE2, lVZ),           DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTVELOCITY },
	{ &GUID_RxAxis, FIELD_OFFSET(DIJOYSTATE2, lVRx),          DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTVELOCITY },
	{ &GUID_RyAxis, FIELD_OFFSET(DIJOYSTATE2, lVRy),          DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTVELOCITY },
	{ &GUID_RzAxis, FIELD_OFFSET(DIJOYSTATE2, lVRz),          DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTVELOCITY },
	{ &GUID_Slider, DIJOFS_SLIDER(0),                         DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTVELOCITY },
	{ &GUID_Slider, DIJOFS_SLIDER(1),                         DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTVELOCITY },
	{ &GUID_XAxis,  FIELD_OFFSET(DIJOYSTATE2, lAX),           DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTACCEL },
	{ &GUID_YAxis,  FIELD_OFFSET(DIJOYSTATE2, lAY),           DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTACCEL },
	{ &GUID_ZAxis,  FIELD_OFFSET(DIJOYSTATE2, lAZ),           DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTACCEL },
	{ &GUID_RxAxis, FIELD_OFFSET(DIJOYSTATE2, lARx),          DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTACCEL },
	{ &GUID_RyAxis, FIELD_OFFSET(DIJOYSTATE2, lARy),          DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTACCEL },
	{ &GUID_RzAxis, FIELD_OFFSET(DIJOYSTATE2, lARz),          DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTACCEL },
	{ &GUID_Slider, DIJOFS_SLIDER(0),                         DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTACCEL },
	{ &GUID_Slider, DIJOFS_SLIDER(1),                         DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTACCEL },
	{ &GUID_XAxis,  FIELD_OFFSET(DIJOYSTATE2, lFX),           DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTFORCE },
	{ &GUID_YAxis,  FIELD_OFFSET(DIJOYSTATE2, lFY),           DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTFORCE },
	{ &GUID_ZAxis,  FIELD_OFFSET(DIJOYSTATE2, lFZ),           DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTFORCE },
	{ &GUID_RxAxis, FIELD_OFFSET(DIJOYSTATE2, lFRx),          DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTFORCE },
	{ &GUID_RyAxis, FIELD_OFFSET(DIJOYSTATE2, lFRy),          DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTFORCE },
	{ &GUID_RzAxis, FIELD_OFFSET(DIJOYSTATE2, lFRz),          DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTFORCE },
	{ &GUID_Slider, DIJOFS_SLIDER(0),                         DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTFORCE },
	{ &GUID_Slider, DIJOFS_SLIDER(1),                         DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   DIDOI_ASPECTFORCE },
};
const DIDATAFORMAT c_dfDIJoystick2 = {
	sizeof(DIDATAFORMAT),
	sizeof(DIOBJECTDATAFORMAT),
	DIDF_ABSAXIS,
	sizeof(DIJOYSTATE2),
	LENGTH(dfDIJoystick2),
	dfDIJoystick2
};
struct _js_os {
	// DirectInput
	HMODULE di8;
	LPDIRECTINPUTW directInputInterface;

	// XInput
	HMODULE xinput;
	BYTE xinput_available;
	unsigned int xinput_player_index;
	unsigned int xinput_player_count;
	DWORD (WINAPI *XInputGetStateEx_proc)(DWORD dwUserIndex, XINPUT_STATE_EX *pState);
	DWORD (WINAPI *XInputGetState_proc)(DWORD dwUserIndex, XINPUT_STATE *pState);
	DWORD (WINAPI *XInputGetCapabilities_proc)(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES *pCapabilities);
} js_os;

void js_os_init(BYTE first_time) {
	if (first_time) {
		memset(&js_os, 0x00, sizeof(js_os));

#if defined (__GNUC__)
#if __GNUC__ >= 8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
#endif
		if (((js_os.xinput = LoadLibrary("XInput1_4.dll")) == NULL) &&
			((js_os.xinput = LoadLibrary("XInput1_3.dll")) == NULL)) {
			log_error(uL("xinput;failed to load xinput dll"));
			js_os.xinput_available = FALSE;
		} else {
			js_os.xinput_available = TRUE;
			js_os.XInputGetStateEx_proc =(DWORD (WINAPI *)(DWORD, XINPUT_STATE_EX *))GetProcAddress(js_os.xinput, (LPCSTR) 100);
			js_os.XInputGetState_proc = (DWORD (WINAPI *)(DWORD, XINPUT_STATE *))GetProcAddress(js_os.xinput, "XInputGetState");
			js_os.XInputGetCapabilities_proc = (DWORD (WINAPI *)(DWORD, DWORD, XINPUT_CAPABILITIES *))GetProcAddress(js_os.xinput, "XInputGetCapabilities");
		}

		if ((js_os.di8 = LoadLibrary("DINPUT8.dll")) == NULL) {
			log_error(uL("directinput;failed to load dinput8.dll"));
		} else {
			HRESULT (WINAPI *DirectInput8Create_proc)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);

			DirectInput8Create_proc = (HRESULT (WINAPI *)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN))GetProcAddress(js_os.di8, "DirectInput8Create");
			DirectInput8Create_proc(GetModuleHandle(NULL),
				DIRECTINPUT_VERSION,
				&IID_IDirectInput8W,
				(void **)&js_os.directInputInterface,
				NULL);
		}
#if defined (__GNUC__)
#if __GNUC__ >= 8
#pragma GCC diagnostic pop
#endif
#endif
	}
}
void js_os_quit(BYTE last_time) {
	if (last_time) {
		if (js_os.di8) {
			FreeLibrary(js_os.di8);
		}
		if (js_os.xinput) {
			FreeLibrary(js_os.xinput);
		}
	}
}
void js_os_jdev_init(_js_device *jdev) {
	jdev->xinput_player_index = 0;
	jdev->di8device = NULL;
	jdev->buffered = FALSE;
}
void js_os_jdev_open(_js_device *jdev, void *arg) {
	_js_raw_devices *raw = (_js_raw_devices *)arg;
	BYTE is_xinput = FALSE;

	if (jdev == NULL) {
		return;
	}

	if (raw->count && (raw->list != NULL)) {
		unsigned int i;

		for (i = 0; i < raw->count; i++) {
			RID_DEVICE_INFO rdi;
			char name[128];
			UINT srdi = sizeof(rdi), sname = sizeof(name);

			rdi.cbSize = sizeof(rdi);

			if ((raw->list[i].dwType != RIM_TYPEHID) ||
				(GetRawInputDeviceInfoA(raw->list[i].hDevice, RIDI_DEVICEINFO, &rdi, &srdi) == (UINT)-1) ||
				(MAKELONG(rdi.hid.dwVendorId, rdi.hid.dwProductId) != (LONG)jdev->product_guid.Data1)) {
				continue;
			}

			jdev->usb.bustype = 0;
			jdev->usb.vendor_id = rdi.hid.dwVendorId;
			jdev->usb.product_id = rdi.hid.dwProductId;
			jdev->usb.version = rdi.hid.dwVersionNumber;

			if ((GetRawInputDeviceInfoA(raw->list[i].hDevice, RIDI_DEVICENAME, name, &sname) != (UINT)-1) &&
				(strstr(name, "IG_") != NULL)) {
				is_xinput = TRUE;
			}
		}
	}

	js_jdev_type(jdev);

	if (!is_xinput) {
		is_xinput = js_jdev_is_xinput(jdev);
	}

	if (js_os.xinput_available && is_xinput && (js_os.xinput_player_count < 4)) {
		jdev->is_xinput = TRUE;
		jdev->xinput_player_index = js_os.xinput_player_index;
	}

	if (jdev->is_xinput && (jdev->xinput_player_index < 4)) {
		DWORD xrc;
		int i;

		for (i = 0; i < 4; i++) {
			XINPUT_STATE state;

			ZeroMemory(&state, sizeof(XINPUT_STATE));
			xrc = js_os.XInputGetState_proc(jdev->xinput_player_index, &state);
			if (xrc == ERROR_SUCCESS) {
				js_os.xinput_player_index = (jdev->xinput_player_index + 1) % 4;
				js_os.xinput_player_count++;
				break;
			}
			jdev->xinput_player_index = (jdev->xinput_player_index + 1) % 4;
		}
		if (xrc == ERROR_SUCCESS) {
			jdev->info.axes = 6;
			jdev->info.buttons = 15;
			for (i = 0; i < jdev->info.axes; i++) {
				_js_axis *jsx = &jdev->data.axis[i];

				switch (i) {
					case JS_XINPUT_STHUMBLX:
						jsx->offset = ABS_X;
						jsx->min = JS_XINPUT_AXIS_MIN;
						jsx->max = JS_AXIS_MAX;
						break;
					case JS_XINPUT_STHUMBLY:
						jsx->offset = ABS_Y;
						jsx->min = -JS_AXIS_MAX;
						jsx->max = -JS_XINPUT_AXIS_MIN;
						break;
					case JS_XINPUT_STHUMBRX:
						jsx->offset = ABS_RX;
						jsx->min = JS_XINPUT_AXIS_MIN;
						jsx->max = JS_AXIS_MAX;
						break;
					case JS_XINPUT_STHUMBRY:
						jsx->offset = ABS_RY;
						jsx->min = -JS_AXIS_MAX;
						jsx->max = -JS_XINPUT_AXIS_MIN;
						break;
					case JS_XINPUT_BLEFTTRIGGER:
						jsx->offset = ABS_Z;
						jsx->min = 0;
						jsx->max = 255;
						break;
					case JS_XINPUT_BRIGHTTRIGGER:
						jsx->offset = ABS_RZ;
						jsx->min = 0;
						jsx->max = 255;
						break;
					default:
						break;
				}
				jsx->used = TRUE;
				jsx->center = 0;
				jsx->scale = (float)(JS_AXIS_MAX - JS_AXIS_MIN) / (float)(jsx->max - jsx->min);
				jsx->is_hat = FALSE;
			}
			for (i = 0; i < jdev->info.buttons; i++) {
				_js_button *jsx = &jdev->data.button[i];

				jsx->used = TRUE;
				jsx->offset = i + 1;
			}
#if defined (DEBUG)
			js_info_jdev(jdev);
#endif
			jdev->present = TRUE;
		} else {
			jdev->is_xinput = FALSE;
		}
	}

	if (!jdev->present) {
		HRESULT rc;
		IDirectInputDeviceW *didevice;
		IDirectInputDevice8W *di8device;
		DIPROPDWORD bufferSizeProp;
		BYTE buffered = TRUE;

		if ((rc = IDirectInput8_CreateDevice(js_os.directInputInterface, &jdev->guid, &didevice, NULL)) != DI_OK) {
			log_warning(uL("directinput;IDirectInput8_CreateDevice 0x%X"), (unsigned int)rc);
		}

		if ((rc = IDirectInputDevice8_QueryInterface(didevice, &IID_IDirectInputDevice8W, (LPVOID *)&di8device)) != DI_OK) {
			log_warning(uL("directinput;IDirectInputDevice8_QueryInterface 0x%X"), (unsigned int)rc);
		}
		IDirectInputDevice8_Release(didevice);

		if ((rc = IDirectInputDevice8_SetCooperativeLevel(di8device, GetActiveWindow(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)) != DI_OK) {
			log_warning(uL("directinput;IDirectInputDevice8_SetCooperativeLevel 0x%X"), (unsigned int)rc);
		}

		if ((rc = IDirectInputDevice8_SetDataFormat(di8device, &c_dfDIJoystick2)) != DI_OK) {
			log_warning(uL("directinput;IDirectInputDevice8_SetDataFormat 0x%X"), (unsigned int)rc);
		}

		bufferSizeProp.diph.dwSize = sizeof(DIPROPDWORD);
		bufferSizeProp.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		bufferSizeProp.diph.dwObj = 0;
		bufferSizeProp.diph.dwHow = DIPH_DEVICE;
		bufferSizeProp.dwData = INPUT_QUEUE_SIZE;

		rc = IDirectInputDevice8_SetProperty(di8device, DIPROP_BUFFERSIZE, &bufferSizeProp.diph);
		if (rc == DI_POLLEDDEVICE) {
			buffered = FALSE;
		} else if (rc != DI_OK) {
			log_warning(uL("directinput;IDirectInputDevice8_SetProperty 0x%X"), (unsigned int)rc);
		}

		jdev->di8device = (void *)di8device;
		jdev->buffered = buffered;
		jdev->info.axes = 0;
		jdev->info.buttons = 0;
		IDirectInputDevice_EnumObjects(di8device, cb_enum_obj, jdev, DIDFT_AXIS | DIDFT_POV | DIDFT_BUTTON);
#if defined (DEBUG)
		js_info_jdev(jdev);
#endif
		jdev->present = TRUE;
	}

	if (jdev->present) {
		jstick.jdd.count++;
		js_jdev_open_common(jdev);
	}
}
void js_os_jdev_close(_js_device *jdev) {
	if (jdev == NULL) {
		return;
	}

	if (jdev->present) {
		jstick.jdd.count--;
#if defined (DEBUG)
		log_warning(uL("jstick disc.;slot%d \"" uPs("") "\" (%d)\n"),
			jdev->index,
			jdev->desc,
			jstick.jdd.count);
#endif
	}

	jdev->present = FALSE;
	js_guid_unset(&jdev->guid);
	js_guid_unset(&jdev->product_guid);

	if (jdev->is_xinput) {
		js_os.xinput_player_count--;
	} else if (jdev->di8device) {
		IDirectInputDevice8_Release(JDEVIDID8W);
		jdev->di8device = NULL;
	}
}
void js_os_jdev_scan(void) {
	_js_raw_devices raw;
	HRESULT result;

	if ((GetRawInputDeviceList(NULL, &raw.count, sizeof(RAWINPUTDEVICELIST)) != (UINT)-1) && (raw.count > 0)) {
		raw.list = malloc(sizeof(RAWINPUTDEVICELIST) * raw.count);
		if (GetRawInputDeviceList(raw.list, &raw.count, sizeof(RAWINPUTDEVICELIST)) == (UINT)-1) {
			free(raw.list);
			raw.list = NULL;
			raw.count = 0;
		}
	}

	if ((result = IDirectInput_EnumDevices(js_os.directInputInterface,
		DI8DEVCLASS_GAMECTRL,
		cb_enum_dev,
		(void *)&raw,
		DIEDFL_ALLDEVICES)) != DI_OK) {
		log_warning(uL("directinput;IDirectInput_EnumDevices 0x%X"), (unsigned int)result);
	}

	if (raw.list) {
		free(raw.list);
		raw.list = NULL;
	}
	raw.count = 0;
}
void js_os_jdev_read_events_loop(_js_device *jdev) {
	BYTE error = TRUE;
	int i;

	if (jdev->is_xinput && (jdev->xinput_player_index < 4)) {
		XINPUT_STATE state;
		DWORD xrc;

		if (js_os.XInputGetStateEx_proc != NULL) {
			XINPUT_STATE_EX stateEx;

			xrc = js_os.XInputGetStateEx_proc(jdev->xinput_player_index, &stateEx);
			state.Gamepad.wButtons = stateEx.Gamepad.wButtons;
			state.Gamepad.sThumbLX = stateEx.Gamepad.sThumbLX;
			state.Gamepad.sThumbLY = stateEx.Gamepad.sThumbLY;
			state.Gamepad.sThumbRX = stateEx.Gamepad.sThumbRX;
			state.Gamepad.sThumbRY = stateEx.Gamepad.sThumbRY;
			state.Gamepad.bLeftTrigger = stateEx.Gamepad.bLeftTrigger;
			state.Gamepad.bRightTrigger = stateEx.Gamepad.bRightTrigger;
		} else {
			xrc = js_os.XInputGetState_proc(jdev->xinput_player_index, &state);
		}
		if (xrc == ERROR_SUCCESS) {
			_js_axis *jsx;

#define __js_axs_validate(index, value)\
	jsx = &jdev->data.axis[index];\
	js_axs_validate(jsx, value)
			__js_axs_validate(JS_XINPUT_STHUMBLX, state.Gamepad.sThumbLX);
			__js_axs_validate(JS_XINPUT_STHUMBLY, -state.Gamepad.sThumbLY);
			__js_axs_validate(JS_XINPUT_BLEFTTRIGGER, state.Gamepad.bLeftTrigger);
			__js_axs_validate(JS_XINPUT_STHUMBRX, state.Gamepad.sThumbRX);
			__js_axs_validate(JS_XINPUT_STHUMBRY, -state.Gamepad.sThumbRY);
			__js_axs_validate(JS_XINPUT_BRIGHTTRIGGER, state.Gamepad.bRightTrigger);
#undef __js_axs_validate

			jdev->data.button[JS_XINPUT_GAMEPAD_A].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
			jdev->data.button[JS_XINPUT_GAMEPAD_B].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;
			jdev->data.button[JS_XINPUT_GAMEPAD_X].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0;
			jdev->data.button[JS_XINPUT_GAMEPAD_Y].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;
			jdev->data.button[JS_XINPUT_GAMEPAD_LEFT_SHOULDER].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
			jdev->data.button[JS_XINPUT_GAMEPAD_RIGHT_SHOULDER].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
			jdev->data.button[JS_XINPUT_GAMEPAD_LEFT_THUMB].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
			jdev->data.button[JS_XINPUT_GAMEPAD_RIGHT_THUMB].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
			jdev->data.button[JS_XINPUT_GAMEPAD_BACK].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0;
			jdev->data.button[JS_XINPUT_GAMEPAD_START].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
			jdev->data.button[JS_XINPUT_GAMEPAD_GUIDE].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_GUIDE) != 0;
			jdev->data.button[JS_XINPUT_GAMEPAD_DPAD_UP].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
			jdev->data.button[JS_XINPUT_GAMEPAD_DPAD_DOWN].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
			jdev->data.button[JS_XINPUT_GAMEPAD_DPAD_LEFT].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
			jdev->data.button[JS_XINPUT_GAMEPAD_DPAD_RIGHT].value = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;

			error = FALSE;
		}
	} else if (jdev->di8device) {
		HRESULT rc;

		rc = IDirectInputDevice8_Poll(JDEVIDID8W);
		if ((rc == DIERR_INPUTLOST) || (rc == DIERR_NOTACQUIRED)) {
			IDirectInputDevice8_Acquire(JDEVIDID8W);
			IDirectInputDevice8_Poll(JDEVIDID8W);
		}

		if (jdev->buffered) {
			DIDEVICEOBJECTDATA events[INPUT_QUEUE_SIZE];
			DWORD count = INPUT_QUEUE_SIZE;

			rc = IDirectInputDevice8_GetDeviceData(JDEVIDID8W, sizeof(DIDEVICEOBJECTDATA), events, &count, 0);
			if ((rc == DIERR_INPUTLOST) || (rc == DIERR_NOTACQUIRED)) {
				IDirectInputDevice8_Acquire(JDEVIDID8W);
				rc = IDirectInputDevice8_GetDeviceData(JDEVIDID8W, sizeof(DIDEVICEOBJECTDATA), events, &count, 0);
			}
			if (SUCCEEDED(rc)) {
				for (i = 0; (unsigned int)i < count; i++) {
					unsigned int a, b;

					// axes e hats
					for (a = 0; a < LENGTH(js_axs_type); a++) {
						for (b = 0; b < js_axs_type[a]; b++) {
							int hat_index = (int)b * 2;
							_js_axis *jsx = !a ? &jdev->data.axis[b] : &jdev->data.hat[hat_index];

							if (jsx->used && (events[i].dwOfs == jsx->offset_di8)) {
								if (jsx->is_hat) {
									SDBWORD x, y;

									hat_to_xy(events[i].dwData, &x, &y);
									js_axs_validate(jsx, x);
									jsx = &jdev->data.hat[hat_index + 1];
									js_axs_validate(jsx, y);
								} else {
									js_axs_validate(jsx, (SDBWORD)events[i].dwData);
								}
							}
						}
					}
					// pulsanti
					for (a = 0; a < JS_MAX_BUTTONS; a++) {
						_js_button *jsx = &jdev->data.button[a];

						if (jsx->used && (events[i].dwOfs == jsx->offset_di8)) {
							jsx->value = events[i].dwData != 0;
						}
					}
				}

				error = FALSE;
			}
		} else {
			DIJOYSTATE2 state;

			rc = IDirectInputDevice8_GetDeviceState(JDEVIDID8W, sizeof(DIJOYSTATE2), &state);
			if ((rc == DIERR_INPUTLOST) || (rc == DIERR_NOTACQUIRED)) {
				IDirectInputDevice8_Acquire(JDEVIDID8W);
				rc = IDirectInputDevice8_GetDeviceState(JDEVIDID8W, sizeof(DIJOYSTATE2), &state);
			}
			if (rc == DI_OK) {
				// axes
				for (i = 0; i < JS_MAX_AXES; i++) {
					_js_axis *jsx = &jdev->data.axis[i];

					if (jsx->used) {
						SWORD value = 0;

						switch (jsx->offset_di8) {
							case DIJOFS_X:
								value = (SWORD)state.lX;
								break;
							case DIJOFS_Y:
								value = (SWORD)state.lY;
								break;
							case DIJOFS_Z:
								value = (SWORD)state.lZ;
								break;
							case DIJOFS_RX:
								value = (SWORD)state.lRx;
								break;
							case DIJOFS_RY:
								value = (SWORD)state.lRy;
								break;
							case DIJOFS_RZ:
								value = (SWORD)state.lRz;
								break;
							case DIJOFS_SLIDER(0):
								value = (SWORD)state.rglSlider[0];
								break;
							case DIJOFS_SLIDER(1):
								value = (SWORD)state.rglSlider[1];
								break;
						}
						js_axs_validate(jsx, value);
					}
				}
				// hats
				for (i = 0; i < JS_MAX_HATS; i++) {
					int hat_index = i * 2;
					_js_axis *jsx = &jdev->data.hat[hat_index];

					if (jsx->used) {
						SDBWORD x, y;
						int index = -1;

						switch (jsx->offset_di8) {
							case DIJOFS_POV(0):
								index = 0;
								break;
							case DIJOFS_POV(1):
								index = 1;
								break;
							case DIJOFS_POV(2):
								index = 2;
								break;
							case DIJOFS_POV(3):
								index = 3;
								break;
						}
						if (index != -1) {
							hat_to_xy(state.rgdwPOV[index], &x, &y);
							js_axs_validate(jsx, x);
							jsx = &jdev->data.hat[hat_index + 1];
							js_axs_validate(jsx, y);
						}
					}
				}
				// pulsanti
				for (i = 0; i < JS_MAX_BUTTONS; i++) {
					_js_button *jsx = &jdev->data.button[i];

					if (jsx->used) {
						jsx->value = state.rgbButtons[i] != 0;
					}
				}

				error = FALSE;
			}
		}
	}

	if (error) {
		js_os_jdev_close(jdev);
		return;
	}
}

INLINE static void hat_to_xy(DWORD hat, SDBWORD *x, SDBWORD *y) {
	if (LOWORD(hat) == 0xFFFF) {
		(*x) = (*y) = 0;
	} else {
		if ((hat > JS_OS_HAT_UP) && (hat < JS_OS_HAT_DOWN)) {
			(*x) = JS_AXIS_MAX;
		} else if (hat > JS_OS_HAT_DOWN) {
			(*x) = JS_AXIS_MIN;
		} else {
			(*x) = 0;
		}

		if ((hat > JS_OS_HAT_LEFT) || (hat < JS_OS_HAT_RIGHT)) {
			(*y) = JS_AXIS_MIN;
		} else if ((hat > JS_OS_HAT_RIGHT) && (hat < JS_OS_HAT_LEFT)) {
			(*y) = JS_AXIS_MAX;
		} else {
			(*y) = 0;
		}
	}
}

static BOOL CALLBACK cb_enum_dev(LPCDIDEVICEINSTANCEW instance, LPVOID context) {
	if (jstick.jdd.count < MAX_JOYSTICK) {
		_js_device *jdev, *jd = NULL;
		int i;

		for (i = 0; i < MAX_JOYSTICK; i++) {
			jdev = &jstick.jdd.devices[i];

			if (js_is_null(&jdev->guid)) {
				if (jd == NULL) {
					jd = jdev;
				}
			} else if (js_guid_cmp(&jdev->guid, (_input_guid *)&instance->guidInstance)) {
				jd = !jdev->present ? jdev : NULL;
				break;
			}
		}
		if (jd) {
			thread_mutex_lock(jd->lock);

			js_jdev_init(jd);

			memcpy(&jd->guid, &instance->guidInstance, sizeof(_input_guid));
			memcpy(&jd->product_guid, &instance->guidProduct, sizeof(GUID));
			ustrncpy(jd->desc, instance->tszProductName, usizeof(jd->desc) - 1);

			js_os_jdev_open(jd, (void *)context);

			thread_mutex_unlock(jd->lock);
		}
	}
	return (DIENUM_CONTINUE);
}
static BOOL CALLBACK cb_enum_obj(LPCDIDEVICEOBJECTINSTANCEW instance, LPVOID context) {
	_js_device *jdev = context;

	if (instance->dwType & DIDFT_POV) {
		if (jdev->info.hats < JS_MAX_HATS) {
			int index = jdev->info.hats * 2;
			_js_axis *jsx;

			jsx = &jdev->data.hat[index];
			jsx->used = TRUE;
			jsx->min = JS_AXIS_MIN;
			jsx->max = JS_AXIS_MAX;
			jsx->center = 0;
			jsx->scale = (float)(JS_AXIS_MAX - JS_AXIS_MIN) / (float)(jsx->max - jsx->min);
			jsx->offset = ABS_HAT0X + index;
			jsx->offset_di8 = DIJOFS_POV(jdev->info.hats);
			jsx->is_hat = TRUE;
			jdev->info.axes++;

			jsx = &jdev->data.hat[index + 1];
			jsx->used = TRUE;
			jsx->min = JS_AXIS_MIN;
			jsx->max = JS_AXIS_MAX;
			jsx->center = 0;
			jsx->scale = (float)(JS_AXIS_MAX - JS_AXIS_MIN) / (float)(jsx->max - jsx->min);
			jsx->offset = ABS_HAT0X + index + 1;
			jsx->offset_di8 = DIJOFS_POV(jdev->info.hats);
			jsx->is_hat = TRUE;
			jdev->info.hats++;
		}
	} else if (instance->dwType & DIDFT_AXIS) {
		DIPROPRANGE range;
		DIPROPDWORD deadzone;
		_js_axis *jsx = NULL;
		HRESULT rc;

		if (!memcmp(&instance->guidType, &GUID_XAxis, sizeof(instance->guidType))) {
			jsx = &jdev->data.axis[JS_XINPUT_STHUMBLX];
			jsx->offset = ABS_X;
			jsx->offset_di8 = DIJOFS_X;
		} else if (!memcmp(&instance->guidType, &GUID_YAxis, sizeof(instance->guidType))) {
			jsx = &jdev->data.axis[JS_XINPUT_STHUMBLY];
			jsx->offset = ABS_Y;
			jsx->offset_di8 = DIJOFS_Y;
		} else if (!memcmp(&instance->guidType, &GUID_ZAxis, sizeof(instance->guidType))) {
			jsx = &jdev->data.axis[JS_XINPUT_BLEFTTRIGGER];
			jsx->offset = ABS_Z;
			jsx->offset_di8 = DIJOFS_Z;
		} else if (!memcmp(&instance->guidType, &GUID_RxAxis, sizeof(instance->guidType))) {
			jsx = &jdev->data.axis[JS_XINPUT_STHUMBRX];
			jsx->offset = ABS_RX;
			jsx->offset_di8 = DIJOFS_RX;
		} else if (!memcmp(&instance->guidType, &GUID_RyAxis, sizeof(instance->guidType))) {
			jsx = &jdev->data.axis[JS_XINPUT_STHUMBRY];
			jsx->offset = ABS_RY;
			jsx->offset_di8 = DIJOFS_RY;
		} else if (!memcmp(&instance->guidType, &GUID_RzAxis, sizeof(instance->guidType))) {
			jsx = &jdev->data.axis[JS_XINPUT_BRIGHTTRIGGER];
			jsx->offset = ABS_RZ;
			jsx->offset_di8 = DIJOFS_RZ;
		} else if (!memcmp(&instance->guidType, &GUID_Slider, sizeof(instance->guidType))) {
			//jsx = &jdev->data.axis[ABS_THROTTLE + jdev->info.sliders];
			//jsx->offset = ABS_THROTTLE + jdev->info.sliders;
			//jsx->offset_di8 = DIJOFS_SLIDER(jdev->info.sliders);
			jdev->info.sliders++;
		}

		if (jsx) {
			range.diph.dwSize = sizeof(range);
			range.diph.dwHeaderSize = sizeof(range.diph);
			range.diph.dwObj = instance->dwType;
			range.diph.dwHow = DIPH_BYID;
			range.lMin = JS_AXIS_MIN;
			range.lMax = JS_AXIS_MAX;

			if ((rc = IDirectInputDevice8_SetProperty(JDEVIDID8W, DIPROP_RANGE, &range.diph)) != DI_OK) {
				log_warning(uL("directinput;IDIrectInputDevice8_SetProperty 0x%X"), (unsigned int)rc);
			}

			deadzone.diph.dwSize = sizeof(deadzone);
			deadzone.diph.dwHeaderSize = sizeof(deadzone.diph);
			deadzone.diph.dwObj = instance->dwType;
			deadzone.diph.dwHow = DIPH_BYID;
			deadzone.dwData = 0;
			if ((rc = IDirectInputDevice8_SetProperty(JDEVIDID8W, DIPROP_DEADZONE, &deadzone.diph)) != DI_OK) {
				log_warning(uL("directinput;IDIrectInputDevice8_SetProperty 0x%X"), (unsigned int)rc);
			}

			jsx->used = TRUE;
			jsx->min = JS_AXIS_MIN;
			jsx->max = JS_AXIS_MAX;
			jsx->center = 0;
			jsx->scale = (float)(JS_AXIS_MAX - JS_AXIS_MIN) / (float)(jsx->max - jsx->min);
			jsx->is_hat = FALSE;
			jdev->info.axes++;
		}
	} else if (jdev->info.buttons < JS_MAX_BUTTONS) {
		if (jdev->info.buttons < JS_MAX_BUTTONS) {
			_js_button *jsx = &jdev->data.button[jdev->info.buttons];

			jsx->used = TRUE;
			jsx->offset = jdev->info.buttons + 1;
			jsx->offset_di8 = DIJOFS_BUTTON(jdev->info.buttons);
			jdev->info.buttons++;
		}
	}
	return (DIENUM_CONTINUE);
}
