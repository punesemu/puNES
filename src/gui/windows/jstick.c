/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <dinput.h>
#include <xinput.h>
#include "thread_def.h"
#include "jstick.h"
#include "conf.h"

typedef enum joy_states {
	JOY_ST_CTRL = 0,
	JOY_ST_SCH,
	JOY_ST_MAX
} joy_states;
enum joy_misc {
	JOY_MS_UPDATE_NO_PRESENT = 2000,
	JOY_MS_UPDATE_DETECT_DEVICE = 5000,
	JOY_AXIS_MIN = -32768,
	JOY_AXIS_MAX = 32767,
	JOY_POV_UP = 0,
	JOY_POV_RIGHT = 9000,
	JOY_POV_DOWN = 18000,
	JOY_POV_LEFT = 27000,
	JOY_MAX_BUTTONS = 32
};

#define JDEV ((_jstick_device *)joy->jdev)
#define INPUT_QUEUE_SIZE 32
#define XINPUT_GAMEPAD_GUIDE 0x400
#define JOY_AXIS_SENSIBILITY 0.45f
#define JOY_AXIS_SENSIBILITY_DIALOG 0.45f
#define JOY_AXIS_TO_FLOAT(vl)\
	(vl - JOY_AXIS_MIN) / (float)(JOY_AXIS_MAX - JOY_AXIS_MIN) * 2.0f - 1.0f

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
typedef struct _jstick_di_axis_info {
	DWORD offset;
	BOOL is_pov;
	BOOL is_pov_second;
} _jstick_di_axis_info;
typedef struct _jstick_states {
	float *axis;
	BYTE *button;
} _jstick_states;
typedef struct _jstick_raw_devices {
	UINT count;
	PRAWINPUTDEVICELIST list;
} _jstick_raw_devices;
typedef struct _jstick_device {
	BYTE present;
	GUID guid;
	uTCHAR *product_name;
	unsigned int num_axes;
	unsigned int num_buttons;
	double ts_update_present;

	_jstick_states states[2];

	// xinput
	BYTE is_xinput;
	unsigned int xinput_player_index;

	// dinput
	IDirectInputDevice8W *di8device;
	BYTE buffered;
	unsigned int slider;
	unsigned int pov;
	_jstick_di_axis_info *axis_info;
	DWORD *button_offsets;
} _jstick_device;
typedef struct _jstick_devices_detected {
	UINT count;
	_jstick_device *devices;
} _jstick_devices_detected;

static void js_detect_devices(void);
static void js_update_jdev(_js *joy, BYTE enable_decode, BYTE decode_index);
static void js_update_jdevs(void);
static void js_open(_jstick_device *jdev);
static void js_close(_jstick_device *jdev);
static void js_close_detected_devices(void);
static BOOL js_guidcmp(GUID *guid1, GUID* guid2);
static BOOL js_is_xinput_dev(const GUID *pGuidProductFromDirectInput, _jstick_raw_devices *raw);
static BOOL CALLBACK cb_enum_dev(LPCDIDEVICEINSTANCEW instance, LPVOID context);
static BOOL CALLBACK cb_count_axes(LPCDIDEVICEOBJECTINSTANCEW instance, LPVOID context);
static BOOL CALLBACK cb_count_buttons(LPCDIDEVICEOBJECTINSTANCEW instance, LPVOID context);
static BOOL CALLBACK cb_enum_axes(LPCDIDEVICEOBJECTINSTANCEW instance, LPVOID context);
static BOOL CALLBACK cb_enum_buttons(LPCDIDEVICEOBJECTINSTANCEW instance, LPVOID context);
INLINE static DBWORD js_update_button(_js *joy, _port *port, joy_states sm, BYTE index, BYTE value, BYTE *mode);
INLINE static DBWORD js_update_axis_float(_js *joy, _port *port, joy_states sm, BYTE index, float value, BYTE *mode);
INLINE static DBWORD js_update_axis(_js *joy, _port *port, joy_states sm, BYTE index, LONG value, BYTE *mode);
INLINE static void js_pov_to_xy(DWORD pov, float *x, float *y);
INLINE static DBWORD js_update_pov(_js *joy, _port *port, joy_states sm, BYTE index, DWORD value, BYTE *mode);
INLINE static void js_lock(void);
INLINE static void js_unlock(void);

DEFINE_GUID(IID_ZeroGUID,              MAKELONG(0x0000, 0x0000), 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
DEFINE_GUID(IID_ValveStreamingGamepad, MAKELONG(0x28DE, 0x11FF), 0x0000, 0x0000, 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44);
DEFINE_GUID(IID_X360WiredGamepad,      MAKELONG(0x045E, 0x02A1), 0x0000, 0x0000, 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44);
DEFINE_GUID(IID_X360WirelessGamepad,   MAKELONG(0x045E, 0x028E), 0x0000, 0x0000, 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44);

DIOBJECTDATAFORMAT dfDIJoystick2[] = {
	{ &GUID_XAxis,  DIJOFS_X,                                 DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_YAxis,  DIJOFS_Y,                                 DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_ZAxis,  DIJOFS_Z,                                 DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_RxAxis, DIJOFS_RX,                                DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_RyAxis, DIJOFS_RY,                                DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_RzAxis, DIJOFS_RZ,                                DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_Slider, DIJOFS_SLIDER(0),                         DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_Slider, DIJOFS_SLIDER(1),                         DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
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
	{ &GUID_XAxis,  FIELD_OFFSET(DIJOYSTATE2, lVX),           DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_YAxis,  FIELD_OFFSET(DIJOYSTATE2, lVY),           DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_ZAxis,  FIELD_OFFSET(DIJOYSTATE2, lVZ),           DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_RxAxis, FIELD_OFFSET(DIJOYSTATE2, lVRx),          DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_RyAxis, FIELD_OFFSET(DIJOYSTATE2, lVRy),          DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_RzAxis, FIELD_OFFSET(DIJOYSTATE2, lVRz),          DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_Slider, FIELD_OFFSET(DIJOYSTATE2, rglVSlider[0]), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_Slider, FIELD_OFFSET(DIJOYSTATE2, rglVSlider[1]), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_XAxis,  FIELD_OFFSET(DIJOYSTATE2, lAX),           DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_YAxis,  FIELD_OFFSET(DIJOYSTATE2, lAY),           DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_ZAxis,  FIELD_OFFSET(DIJOYSTATE2, lAZ),           DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_RxAxis, FIELD_OFFSET(DIJOYSTATE2, lARx),          DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_RyAxis, FIELD_OFFSET(DIJOYSTATE2, lARy),          DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_RzAxis, FIELD_OFFSET(DIJOYSTATE2, lARz),          DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_Slider, FIELD_OFFSET(DIJOYSTATE2, rglASlider[0]), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_Slider, FIELD_OFFSET(DIJOYSTATE2, rglASlider[1]), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_XAxis,  FIELD_OFFSET(DIJOYSTATE2, lFX),           DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_YAxis,  FIELD_OFFSET(DIJOYSTATE2, lFY),           DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_ZAxis,  FIELD_OFFSET(DIJOYSTATE2, lFZ),           DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_RxAxis, FIELD_OFFSET(DIJOYSTATE2, lFRx),          DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_RyAxis, FIELD_OFFSET(DIJOYSTATE2, lFRy),          DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_RzAxis, FIELD_OFFSET(DIJOYSTATE2, lFRz),          DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_Slider, FIELD_OFFSET(DIJOYSTATE2, rglFSlider[0]), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
	{ &GUID_Slider, FIELD_OFFSET(DIJOYSTATE2, rglFSlider[1]), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE,   0 },
};
const DIDATAFORMAT c_dfDIJoystick2 = {
	sizeof(DIDATAFORMAT),
	sizeof(DIOBJECTDATAFORMAT),
	DIDF_ABSAXIS,
	sizeof(DIJOYSTATE2),
	LENGTH(dfDIJoystick2),
	dfDIJoystick2
};
struct _jstick {
	_jstick_devices_detected jdd;
	thread_mutex_t lock;
	double ts_update_devices;

	// DirectInput
	HMODULE di8;
	LPDIRECTINPUTW directInputInterface;

	// XInput
	HMODULE xinput;
	BYTE xinput_available;
	unsigned int xinput_player_index;
	DWORD (WINAPI *XInputGetStateEx_proc)(DWORD dwUserIndex, XINPUT_STATE_EX *pState);
	DWORD (WINAPI *XInputGetState_proc)(DWORD dwUserIndex, XINPUT_STATE *pState);
	DWORD (WINAPI *XInputGetCapabilities_proc)(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES *pCapabilities);
} jstick;

_js js[PORT_MAX], js_shcut;

void js_init(BYTE first_time) {
	int i;

	if (first_time) {
		memset(&jstick, 0x00, sizeof(jstick));

#if defined (__GNUC__)
#if __GNUC__ >= 8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
#endif
		if (((jstick.xinput = LoadLibrary("XInput1_4.dll")) == NULL) &&
			((jstick.xinput = LoadLibrary("XInput1_3.dll")) == NULL)) {
			fprintf(stderr, "XInput : failed to load XInput dll\n");
			jstick.xinput_available = FALSE;
		} else {
			jstick.xinput_available = TRUE;
			jstick.XInputGetStateEx_proc =
				(DWORD (WINAPI *)(DWORD, XINPUT_STATE_EX *))GetProcAddress(jstick.xinput, (LPCSTR) 100);
			jstick.XInputGetState_proc =
				(DWORD (WINAPI *)(DWORD, XINPUT_STATE *))GetProcAddress(jstick.xinput, "XInputGetState");
			jstick.XInputGetCapabilities_proc =
				(DWORD (WINAPI *)(DWORD, DWORD, XINPUT_CAPABILITIES *))GetProcAddress(jstick.xinput, "XInputGetCapabilities");
		}

		if ((jstick.di8 = LoadLibrary("DINPUT8.dll")) == NULL) {
			fprintf(stderr, "DirectInput8 : failed to load DINPUT8.dll\n");
		} else {
			HRESULT (WINAPI *DirectInput8Create_proc)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);

			DirectInput8Create_proc =
				(HRESULT (WINAPI *)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN))GetProcAddress(jstick.di8, "DirectInput8Create");
			DirectInput8Create_proc(GetModuleHandle(NULL), DIRECTINPUT_VERSION, &IID_IDirectInput8W, (void **)&jstick.directInputInterface, NULL);
		}
#if defined (__GNUC__)
#if __GNUC__ >= 8
#pragma GCC diagnostic pop
#endif
#endif
		thread_mutex_init(jstick.lock);
	}

	js_lock();
	for (i = PORT1; i < PORT_MAX; i++) {
		memset(&js[i], 0x00, sizeof(_js));
		memcpy(&js[i].guid, &port[i].joy_id, sizeof(GUID));
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
		if (jstick.di8) {
			FreeLibrary(jstick.di8);
		}
		if (jstick.xinput) {
			FreeLibrary(jstick.xinput);
		}
		thread_mutex_destroy(jstick.lock);
	}
}
void js_update_detected_devices(void) {
	js_lock();
	js_detect_devices();
	js_unlock();
}
void js_control(_js *joy, _port *port) {
	_jstick_device *jdev;
	BYTE mode = 0;

#define js_control_button(index, bts)\
	js_update_button(joy, port, JOY_ST_CTRL, index, bts, &mode)
#define js_control_axis_float(index, axs)\
	js_update_axis_float(joy, port, JOY_ST_CTRL, index, axs, &mode)
#define js_control_axis(index, axs)\
	js_update_axis(joy, port, JOY_ST_CTRL, index, axs, &mode)
#define js_control_pov(index, pov)\
	js_update_pov(joy, port, JOY_ST_CTRL, index, pov, &mode)

	js_lock();

	jdev = joy->jdev;

	if (joy->inited == FALSE) {
		js_unlock();
		return;
	} else if (jdev == NULL) {
		double now = gui_get_ms();

		if ((now - jstick.ts_update_devices) > JOY_MS_UPDATE_DETECT_DEVICE) {
			js_detect_devices();
		}
		js_unlock();
		return;
	} else if (jdev->present == FALSE) {
		double now = gui_get_ms();

		if ((now - jdev->ts_update_present) > JOY_MS_UPDATE_NO_PRESENT) {
			jdev->ts_update_present = now;
			// faccio un tentativo per controllare se e' stato ricollegato.
			jdev->present = TRUE;
		} else {
			js_unlock();
			return;
		}
	}

	if (jdev->is_xinput && (jdev->xinput_player_index < 4)) {
		XINPUT_STATE state;
		DWORD xrc;

		if (jstick.XInputGetStateEx_proc != NULL) {
			XINPUT_STATE_EX stateEx;

			xrc = jstick.XInputGetStateEx_proc(jdev->xinput_player_index, &stateEx);
			state.Gamepad.wButtons = stateEx.Gamepad.wButtons;
			state.Gamepad.sThumbLX = stateEx.Gamepad.sThumbLX;
			state.Gamepad.sThumbLY = stateEx.Gamepad.sThumbLY;
			state.Gamepad.sThumbRX = stateEx.Gamepad.sThumbRX;
			state.Gamepad.sThumbRY = stateEx.Gamepad.sThumbRY;
			state.Gamepad.bLeftTrigger = stateEx.Gamepad.bLeftTrigger;
			state.Gamepad.bRightTrigger = stateEx.Gamepad.bRightTrigger;
		} else {
			xrc = jstick.XInputGetState_proc(jdev->xinput_player_index, &state);
		}
		if (xrc == ERROR_SUCCESS) {
			js_control_button(0, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_A));
			js_control_button(1, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_B));
			js_control_button(2, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_X));
			js_control_button(3, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_Y));
			js_control_button(4, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER));
			js_control_button(5, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER));
			js_control_button(6, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB));
			js_control_button(7, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB));
			js_control_button(8, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK));
			js_control_button(9, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_START));
			js_control_button(10, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_GUIDE));
			js_control_button(11, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP));
			js_control_button(12, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN));
			js_control_button(13, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT));
			js_control_button(14, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT));
			js_control_axis(0, state.Gamepad.sThumbLX);
			js_control_axis(1, -state.Gamepad.sThumbLY);
			js_control_axis(4, state.Gamepad.sThumbRX);
			js_control_axis(3, -state.Gamepad.sThumbRY);
			js_control_axis_float(2, state.Gamepad.bLeftTrigger / 256.0f);
			js_control_axis_float(5, state.Gamepad.bRightTrigger / 256.0f);
		} else {
			jdev->present = FALSE;
			js_unlock();
			return;
		}
	} else if (jdev->di8device) {
		HRESULT rc;
		unsigned int button, axis;

		rc = IDirectInputDevice8_Poll(jdev->di8device);
		if ((rc == DIERR_INPUTLOST) || (rc == DIERR_NOTACQUIRED)) {
			IDirectInputDevice8_Acquire(jdev->di8device);
			IDirectInputDevice8_Poll(jdev->di8device);
		}

		if (jdev->buffered) {
			DIDEVICEOBJECTDATA events[INPUT_QUEUE_SIZE];
			DWORD count = INPUT_QUEUE_SIZE;
			unsigned int i;

			rc = IDirectInputDevice8_GetDeviceData(jdev->di8device, sizeof(DIDEVICEOBJECTDATA), events, &count, 0);
			if ((rc == DIERR_INPUTLOST) || (rc == DIERR_NOTACQUIRED)) {
				IDirectInputDevice8_Acquire(jdev->di8device);
				rc = IDirectInputDevice8_GetDeviceData(jdev->di8device, sizeof(DIDEVICEOBJECTDATA), events, &count, 0);
			}
			if (!SUCCEEDED(rc)) {
				jdev->present = FALSE;
				js_unlock();
				return;
			}

			for (i = 0; i < count; i++) {
				for (button = 0; button < jdev->num_buttons; button++) {
					if (events[i].dwOfs == jdev->button_offsets[button]) {
						js_control_button(button, !!events[i].dwData);
					}
				}
				for (axis = 0; axis < jdev->num_axes; axis++) {
					if (events[i].dwOfs == jdev->axis_info[axis].offset) {
						if (jdev->axis_info[axis].is_pov) {
							js_control_pov(axis, events[i].dwData);
							axis++;
						} else {
							js_control_axis(axis, events[i].dwData);
						}
					}
				}
			}
		} else {
			DIJOYSTATE2 state;

			rc = IDirectInputDevice8_GetDeviceState(jdev->di8device, sizeof(DIJOYSTATE2),
					&state);
			if ((rc == DIERR_INPUTLOST) || (rc == DIERR_NOTACQUIRED)) {
				IDirectInputDevice8_Acquire(jdev->di8device);
				rc = IDirectInputDevice8_GetDeviceState(jdev->di8device, sizeof(DIJOYSTATE2), &state);
			}
			if (rc != DI_OK) {
				jdev->present = FALSE;
				js_unlock();
				return;
			}

			for (button = 0; button < jdev->num_buttons; button++) {
				js_control_button(button, !!state.rgbButtons[button]);
			}

			for (axis = 0; axis < jdev->num_axes; axis++) {
				switch (jdev->axis_info[axis].offset) {
					case DIJOFS_X:
						js_control_axis(axis, state.lX);
						break;
					case DIJOFS_Y:
						js_control_axis(axis, state.lY);
						break;
					case DIJOFS_Z:
						js_control_axis(axis, state.lZ);
						break;
					case DIJOFS_RX:
						js_control_axis(axis, state.lRx);
						break;
					case DIJOFS_RY:
						js_control_axis(axis, state.lRy);
						break;
					case DIJOFS_RZ:
						js_control_axis(axis, state.lRz);
						break;
					case DIJOFS_SLIDER(0):
						js_control_axis(axis, state.rglSlider[0]);
						break;
					case DIJOFS_SLIDER(1):
						js_control_axis(axis, state.rglSlider[1]);
						break;
					case DIJOFS_POV(0):
						js_control_pov(axis, state.rgdwPOV[0]);
						axis++;
						break;
					case DIJOFS_POV(1):
						js_control_pov(axis, state.rgdwPOV[1]);
						axis++;
						break;
					case DIJOFS_POV(2):
						js_control_pov(axis, state.rgdwPOV[2]);
						axis++;
						break;
					case DIJOFS_POV(3):
						js_control_pov(axis, state.rgdwPOV[3]);
						axis++;
						break;
				}
			}
		}
	}

#undef js_control_pov
#undef js_control_axis
#undef js_control_axis_float
#undef js_control_button

	js_unlock();
}

BYTE js_is_connected(int dev) {
	if (dev >= (int)jstick.jdd.count) {
		return (EXIT_ERROR);
	}

	return (jstick.jdd.devices[dev].present ? EXIT_OK : EXIT_ERROR);
}
BYTE js_is_this(BYTE dev, GUID *guid) {
	if (dev >= jstick.jdd.count) {
		return (FALSE);
	}

	return (js_guidcmp(guid, &jstick.jdd.devices[dev].guid));
}
BYTE js_is_null(GUID *guid) {
	return (js_guidcmp(guid, (GUID *)&IID_ZeroGUID));
}
void js_set_id(GUID *guid, int dev) {
	if (dev >= (int)jstick.jdd.count) {
		memcpy(guid, &IID_ZeroGUID, sizeof(GUID));
		return;
	}
	memcpy(guid, &jstick.jdd.devices[dev].guid, sizeof(GUID));
}
uTCHAR *js_name_device(int dev) {
	if (dev >= (int)jstick.jdd.count) {
		return (NULL);
	}
	return (jstick.jdd.devices[dev].product_name);
}
uTCHAR *js_to_name(const DBWORD val, const _js_element *list, const DBWORD length) {
	static uTCHAR str[20];
	BYTE index;

	umemset(str, 0x00, usizeof(str));

	for (index = 0; index < length; index++) {
		if (val == list[index].value) {
			ustrncpy(str, list[index].name, usizeof(str));
			return ((uTCHAR *)str);
		}
	}
	return ((uTCHAR *)list[0].name);
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
DBWORD js_read_in_dialog(GUID *guid, UNUSED(int fd)) {
	DBWORD value = 0;
	_jstick_device *jdev = NULL;
	float fl, x, y;
	unsigned int i;

#define js_read_in_dialog_button(index, bts)\
	if (bts) {\
		value = index | 0x400;\
		goto js_read_in_dialog_exit;\
	}
#define js_read_in_dialog_axis_float(index, axs)\
	fl = axs;\
	if (fl >= JOY_AXIS_SENSIBILITY_DIALOG) {\
		value = (index << 1) + 1 + 1;\
		goto js_read_in_dialog_exit;\
	} else if (fl <= -JOY_AXIS_SENSIBILITY_DIALOG) {\
		value = (index << 1) + 1;\
		goto js_read_in_dialog_exit;\
	}
#define js_read_in_dialog_axis(index, axs)\
	js_read_in_dialog_axis_float(index, JOY_AXIS_TO_FLOAT(axs))
#define js_read_in_dialog_pov(index, pov)\
	if (LOWORD(pov) == 0xFFFF) {\
		x = y = 0.0f;\
	} else {\
		if ((pov > JOY_POV_UP) && (pov < JOY_POV_DOWN)) {\
			x = 1.0f;\
		} else if (pov > JOY_POV_DOWN) {\
			x = -1.0f;\
		} else {\
			x = 0.0f;\
		}\
		if ((pov > JOY_POV_LEFT) || (pov < JOY_POV_RIGHT)) {\
			y = -1.0f;\
		} else if ((pov > JOY_POV_RIGHT) && (pov < JOY_POV_LEFT)) {\
			y = 1.0f;\
		} else {\
			y = 0.0f;\
		}\
	}\
	js_read_in_dialog_axis_float(index, x)\
	js_read_in_dialog_axis_float((index + 1), y)

	js_lock();

	for (i = 0; i < jstick.jdd.count; i++) {
		if (js_guidcmp(guid, &jstick.jdd.devices[i].guid)) {
			jdev = &jstick.jdd.devices[i];
		}
	}

	if ((jdev == NULL) || (jdev->present == FALSE)) {
		goto js_read_in_dialog_exit;
	}

	if (jdev->is_xinput && (jdev->xinput_player_index < 4)) {
		XINPUT_STATE state;
		DWORD xrc;

		if (jstick.XInputGetStateEx_proc != NULL) {
			XINPUT_STATE_EX stateEx;

			xrc = jstick.XInputGetStateEx_proc(jdev->xinput_player_index, &stateEx);
			state.Gamepad.wButtons = stateEx.Gamepad.wButtons;
			state.Gamepad.sThumbLX = stateEx.Gamepad.sThumbLX;
			state.Gamepad.sThumbLY = stateEx.Gamepad.sThumbLY;
			state.Gamepad.sThumbRX = stateEx.Gamepad.sThumbRX;
			state.Gamepad.sThumbRY = stateEx.Gamepad.sThumbRY;
			state.Gamepad.bLeftTrigger = stateEx.Gamepad.bLeftTrigger;
			state.Gamepad.bRightTrigger = stateEx.Gamepad.bRightTrigger;
		} else {
			xrc = jstick.XInputGetState_proc(jdev->xinput_player_index, &state);
		}
		if (xrc == ERROR_SUCCESS) {
			js_read_in_dialog_button(0, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_A));
			js_read_in_dialog_button(1, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_B));
			js_read_in_dialog_button(2, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_X));
			js_read_in_dialog_button(3, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_Y));
			js_read_in_dialog_button(4, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER));
			js_read_in_dialog_button(5, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER));
			js_read_in_dialog_button(6, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB));
			js_read_in_dialog_button(7, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB));
			js_read_in_dialog_button(8, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK));
			js_read_in_dialog_button(9, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_START));
			js_read_in_dialog_button(10, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_GUIDE));
			js_read_in_dialog_button(11, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP));
			js_read_in_dialog_button(12, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN));
			js_read_in_dialog_button(13, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT));
			js_read_in_dialog_button(14, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT));
			js_read_in_dialog_axis(0, state.Gamepad.sThumbLX)
			js_read_in_dialog_axis(1, -state.Gamepad.sThumbLY)
			js_read_in_dialog_axis(4, state.Gamepad.sThumbRX)
			js_read_in_dialog_axis(3, -state.Gamepad.sThumbRY)
			js_read_in_dialog_axis_float(2, state.Gamepad.bLeftTrigger / 256.0f)
			js_read_in_dialog_axis_float(5, state.Gamepad.bRightTrigger / 256.0f)
		} else {
			goto js_read_in_dialog_exit;
		}
	} else if (jdev->di8device) {
		DIJOYSTATE2 state;
		unsigned int button, axis;
		HRESULT rc;

		rc = IDirectInputDevice8_Poll(jdev->di8device);
		if ((rc == DIERR_INPUTLOST) || (rc == DIERR_NOTACQUIRED)) {
			IDirectInputDevice8_Acquire(jdev->di8device);
			IDirectInputDevice8_Poll(jdev->di8device);
		}

		rc = IDirectInputDevice8_GetDeviceState(jdev->di8device, sizeof(DIJOYSTATE2), &state);
		if ((rc == DIERR_INPUTLOST) || (rc == DIERR_NOTACQUIRED)) {
			IDirectInputDevice8_Acquire(jdev->di8device);
			rc = IDirectInputDevice8_GetDeviceState(jdev->di8device, sizeof(DIJOYSTATE2), &state);
		}
		if (rc != DI_OK) {
			goto js_read_in_dialog_exit;
		}

		for (button = 0; button < jdev->num_buttons; button++) {
			js_read_in_dialog_button(button, !!state.rgbButtons[button]);
		}

		for (axis = 0; axis < jdev->num_axes; axis++) {
			switch (jdev->axis_info[axis].offset) {
				case DIJOFS_X:
					js_read_in_dialog_axis(axis, state.lX);
					break;
				case DIJOFS_Y:
					js_read_in_dialog_axis(axis, state.lY);
					break;
				case DIJOFS_Z:
					js_read_in_dialog_axis(axis, state.lZ);
					break;
				case DIJOFS_RX:
					js_read_in_dialog_axis(axis, state.lRx);
					break;
				case DIJOFS_RY:
					js_read_in_dialog_axis(axis, state.lRy);
					break;
				case DIJOFS_RZ:
					js_read_in_dialog_axis(axis, state.lRz);
					break;
				case DIJOFS_SLIDER(0):
					js_read_in_dialog_axis(axis, state.rglSlider[0]);
					break;
				case DIJOFS_SLIDER(1):
					js_read_in_dialog_axis(axis, state.rglSlider[1]);
					break;
				case DIJOFS_POV(0):
					js_read_in_dialog_pov(axis, state.rgdwPOV[0]);
					axis++;
					break;
				case DIJOFS_POV(1):
					js_read_in_dialog_pov(axis, state.rgdwPOV[1]);
					axis++;
					break;
				case DIJOFS_POV(2):
					js_read_in_dialog_pov(axis, state.rgdwPOV[2]);
					axis++;
					break;
				case DIJOFS_POV(3):
					js_read_in_dialog_pov(axis, state.rgdwPOV[3]);
					axis++;
					break;
			}
		}
	}

#undef js_read_in_dialog_pov
#undef js_read_in_dialog_axis
#undef js_read_in_dialog_axis_float
#undef js_read_in_dialog_button

	js_read_in_dialog_exit:
	js_unlock();
	return (value);
}

void js_shcut_init(void) {
	memset(&js_shcut, 0x00, sizeof(_js));
	memcpy(&js_shcut.guid, &cfg->input.shcjoy_id, sizeof(GUID));
	js_update_jdev(&js_shcut, FALSE, 0);
}
void js_shcut_stop(void) {}
BYTE js_shcut_read(_js_sch *js_sch) {
	_js *joy = &js_shcut;
	_jstick_device *jdev;
	DBWORD value = 0;
	BYTE mode = 0;

#define _js_shcut_read_control(funct, index, val)\
	if ((value = funct(joy, NULL, JOY_ST_SCH, index, val, &mode))) {\
		js_sch->value = value;\
		js_sch->mode = mode;\
		js_unlock();\
		return (EXIT_OK);\
	}
#define js_shcut_read_button(index, bts)\
	_js_shcut_read_control(js_update_button, index, bts)
#define js_shcut_read_axis_float(index, axs)\
	_js_shcut_read_control(js_update_axis_float, index, axs)
#define js_shcut_read_axis(index, axs)\
	_js_shcut_read_control(js_update_axis, index, axs)
#define js_shcut_read_pov(index, pov)\
	_js_shcut_read_control(js_update_pov, index, pov)

	js_sch->value = 0;
	js_sch->mode = 255;

	js_lock();

	jdev = joy->jdev;

	if (joy->inited == FALSE) {
		js_unlock();
		return (EXIT_ERROR);
	} else if (jdev == NULL) {
		double now = gui_get_ms();

		if ((now - jstick.ts_update_devices) > JOY_MS_UPDATE_DETECT_DEVICE) {
			js_detect_devices();
		}
		js_unlock();
		return (EXIT_ERROR);
	} else if (jdev->present == FALSE) {
		double now = gui_get_ms();

		if ((now - jdev->ts_update_present) > JOY_MS_UPDATE_NO_PRESENT) {
			jdev->ts_update_present = now;
			// faccio un tentativo per controllare se e' stato ricollegato.
			jdev->present = TRUE;
		} else {
			js_unlock();
			return (EXIT_ERROR);
		}
	}

	if (jdev->is_xinput && (jdev->xinput_player_index < 4)) {
		XINPUT_STATE state;
		DWORD xrc;

		if (jstick.XInputGetStateEx_proc != NULL) {
			XINPUT_STATE_EX stateEx;

			xrc = jstick.XInputGetStateEx_proc(jdev->xinput_player_index, &stateEx);
			state.Gamepad.wButtons = stateEx.Gamepad.wButtons;
			state.Gamepad.sThumbLX = stateEx.Gamepad.sThumbLX;
			state.Gamepad.sThumbLY = stateEx.Gamepad.sThumbLY;
			state.Gamepad.sThumbRX = stateEx.Gamepad.sThumbRX;
			state.Gamepad.sThumbRY = stateEx.Gamepad.sThumbRY;
			state.Gamepad.bLeftTrigger = stateEx.Gamepad.bLeftTrigger;
			state.Gamepad.bRightTrigger = stateEx.Gamepad.bRightTrigger;
		} else {
			xrc = jstick.XInputGetState_proc(jdev->xinput_player_index, &state);
		}
		if (xrc == ERROR_SUCCESS) {
			js_shcut_read_button(0, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_A));
			js_shcut_read_button(1, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_B));
			js_shcut_read_button(2, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_X));
			js_shcut_read_button(3, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_Y));
			js_shcut_read_button(4, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER));
			js_shcut_read_button(5, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER));
			js_shcut_read_button(6, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB));
			js_shcut_read_button(7, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB));
			js_shcut_read_button(8, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK));
			js_shcut_read_button(9, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_START));
			js_shcut_read_button(10, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_GUIDE));
			js_shcut_read_button(11, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP));
			js_shcut_read_button(12, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN));
			js_shcut_read_button(13, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT));
			js_shcut_read_button(14, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT));
			js_shcut_read_axis(0, state.Gamepad.sThumbLX)
			js_shcut_read_axis(1, -state.Gamepad.sThumbLY)
			js_shcut_read_axis(4, state.Gamepad.sThumbRX)
			js_shcut_read_axis(3, -state.Gamepad.sThumbRY)
			js_shcut_read_axis_float(2, state.Gamepad.bLeftTrigger / 256.0f)
			js_shcut_read_axis_float(5, state.Gamepad.bRightTrigger / 256.0f)
		} else {
			jdev->present = FALSE;
			js_unlock();
			return (EXIT_ERROR);
		}
	} else if (jdev->di8device) {
		DIJOYSTATE2 state;
		unsigned int button, axis;
		HRESULT rc;

		rc = IDirectInputDevice8_Poll(jdev->di8device);
		if ((rc == DIERR_INPUTLOST) || (rc == DIERR_NOTACQUIRED)) {
			IDirectInputDevice8_Acquire(jdev->di8device);
			IDirectInputDevice8_Poll(jdev->di8device);
		}

		rc = IDirectInputDevice8_GetDeviceState(jdev->di8device, sizeof(DIJOYSTATE2), &state);
		if ((rc == DIERR_INPUTLOST) || (rc == DIERR_NOTACQUIRED)) {
			IDirectInputDevice8_Acquire(jdev->di8device);
			rc = IDirectInputDevice8_GetDeviceState(jdev->di8device, sizeof(DIJOYSTATE2), &state);
		}
		if (rc != DI_OK) {
			jdev->present = FALSE;
			js_unlock();
			return (EXIT_ERROR);
		}

		for (button = 0; button < jdev->num_buttons; button++) {
			js_shcut_read_button(button, !!state.rgbButtons[button]);
		}

		for (axis = 0; axis < jdev->num_axes; axis++) {
			switch (jdev->axis_info[axis].offset) {
				case DIJOFS_X:
					js_shcut_read_axis(axis, state.lX);
					break;
				case DIJOFS_Y:
					js_shcut_read_axis(axis, state.lY);
					break;
				case DIJOFS_Z:
					js_shcut_read_axis(axis, state.lZ);
					break;
				case DIJOFS_RX:
					js_shcut_read_axis(axis, state.lRx);
					break;
				case DIJOFS_RY:
					js_shcut_read_axis(axis, state.lRy);
					break;
				case DIJOFS_RZ:
					js_shcut_read_axis(axis, state.lRz);
					break;
				case DIJOFS_SLIDER(0):
					js_shcut_read_axis(axis, state.rglSlider[0]);
					break;
				case DIJOFS_SLIDER(1):
					js_shcut_read_axis(axis, state.rglSlider[1]);
					break;
				case DIJOFS_POV(0):
					js_shcut_read_pov(axis, state.rgdwPOV[0]);
					axis++;
					break;
				case DIJOFS_POV(1):
					js_shcut_read_pov(axis, state.rgdwPOV[1]);
					axis++;
					break;
				case DIJOFS_POV(2):
					js_shcut_read_pov(axis, state.rgdwPOV[2]);
					axis++;
					break;
				case DIJOFS_POV(3):
					js_shcut_read_pov(axis, state.rgdwPOV[3]);
					axis++;
					break;
			}
		}
	}

#undef js_shcut_read_pov
#undef js_shcut_read_axis
#undef js_shcut_read_axis_float
#undef js_shcut_read_button
#undef _js_shcut_read_control

	js_unlock();
	return (EXIT_ERROR);
}

static void js_detect_devices(void) {
	_jstick_raw_devices raw;
	HRESULT result;

	js_close_detected_devices();

	jstick.xinput_player_index = 0;

	if ((GetRawInputDeviceList(NULL, &raw.count, sizeof(RAWINPUTDEVICELIST)) != (UINT)-1) && (raw.count > 0)) {
		raw.list = malloc(sizeof(RAWINPUTDEVICELIST) * raw.count);
		if (GetRawInputDeviceList(raw.list, &raw.count, sizeof(RAWINPUTDEVICELIST)) == (UINT)-1) {
			free(raw.list);
			raw.list = NULL;
			raw.count = 0;
		}
	}

	if ((result = IDirectInput_EnumDevices(jstick.directInputInterface, DI8DEVCLASS_GAMECTRL, cb_enum_dev, (void *)&raw, DIEDFL_ALLDEVICES)) != DI_OK) {
		fprintf(stderr, "IDirectInput_EnumDevices : 0x%X\n", (unsigned int)result);
	}

	if (raw.list) {
		free(raw.list);
		raw.list = NULL;
	}
	raw.count = 0;

	js_update_jdevs();

	jstick.ts_update_devices = gui_get_ms();
}
static void js_update_jdev(_js *joy, BYTE enable_decode, BYTE decode_index) {
	joy->jdev = NULL;
	joy->inited = FALSE;
	joy->input_decode_event = NULL;

	if (js_is_null(&joy->guid) == FALSE) {
		unsigned int d;

		for (d = 0; d < jstick.jdd.count; d++) {
			if (js_guidcmp(&joy->guid, &jstick.jdd.devices[d].guid) == TRUE) {
				joy->jdev = &jstick.jdd.devices[d];
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
static void js_open(_jstick_device *jdev) {
	int i;

	if (jdev == NULL) {
		return;
	}

	jdev->present = FALSE;

	if (jdev->is_xinput && (jdev->xinput_player_index < 4)) {
		XINPUT_CAPABILITIES capabilities;

		if (jstick.XInputGetCapabilities_proc(jdev->xinput_player_index, 0, &capabilities) == ERROR_SUCCESS) {
			jdev->num_axes = 6;
			jdev->num_buttons = 15;
			jdev->ts_update_present = gui_get_ms();
			for (i = 0; i < JOY_ST_MAX; i++) {
				jdev->states[i].axis = calloc(sizeof(float), jdev->num_axes);
				jdev->states[i].button = calloc(sizeof(BYTE), jdev->num_buttons);
			}
			jdev->present = TRUE;
		}
	}

	if (jdev->present == FALSE) {
		HRESULT rc;
		IDirectInputDeviceW *didevice;
		IDirectInputDevice8W *di8device;
		DIPROPDWORD bufferSizeProp;
		BYTE buffered = TRUE;

		if ((rc = IDirectInput8_CreateDevice(jstick.directInputInterface, &jdev->guid, &didevice, NULL)) != DI_OK) {
			fprintf(stderr, "IDirectInput8_CreateDevice : 0x%X\n", (unsigned int)rc);
		}

		if ((rc = IDirectInputDevice8_QueryInterface(didevice, &IID_IDirectInputDevice8W, (LPVOID * ) &di8device)) != DI_OK) {
			fprintf(stderr, "IDirectInputDevice8_QueryInterface : 0x%X\n", (unsigned int)rc);
		}
		IDirectInputDevice8_Release(didevice);

		if ((rc = IDirectInputDevice8_SetCooperativeLevel(di8device, GetActiveWindow(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)) != DI_OK) {
			fprintf(stderr, "IDirectInputDevice8_SetCooperativeLevel : 0x%X\n", (unsigned int)rc);
		}

		if ((rc = IDirectInputDevice8_SetDataFormat(di8device, &c_dfDIJoystick2)) != DI_OK) {
			fprintf(stderr, "IDirectInputDevice8_SetDataFormat : 0x%X\n", (unsigned int)rc);
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
			fprintf(stderr, "IDirectInputDevice8_SetProperty : 0x%X\n", (unsigned int)rc);
		}

		jdev->di8device = di8device;
		jdev->buffered = buffered;
		jdev->slider = 0;
		jdev->pov = 0;
		jdev->num_axes = 0;
		IDirectInputDevice_EnumObjects(di8device, cb_count_axes, jdev, DIDFT_AXIS | DIDFT_POV);
		jdev->num_buttons = 0;
		IDirectInputDevice_EnumObjects(di8device, cb_count_buttons, jdev, DIDFT_BUTTON);
		jdev->ts_update_present = gui_get_ms();
		for (i = 0; i < JOY_ST_MAX; i++) {
			jdev->states[i].axis = calloc(sizeof(float), jdev->num_axes);
			jdev->states[i].button = calloc(sizeof(BYTE), jdev->num_buttons);
		}
		jdev->axis_info = calloc(sizeof(_jstick_di_axis_info), jdev->num_axes);
		jdev->button_offsets = calloc(sizeof(DWORD), jdev->num_buttons);
		jdev->num_axes = 0;
		IDirectInputDevice_EnumObjects(di8device, cb_enum_axes, jdev, DIDFT_AXIS | DIDFT_POV);
		jdev->num_buttons = 0;
		IDirectInputDevice_EnumObjects(di8device, cb_enum_buttons, jdev, DIDFT_BUTTON);
		jdev->present = TRUE;
	}
}
static void js_close(_jstick_device *jdev) {
	int i;

	if (jdev == NULL) {
		return;
	}

	jdev->present = FALSE;

	if (jdev->product_name) {
		free(jdev->product_name);
		jdev->product_name = NULL;
	}
	if ((jdev->is_xinput == FALSE) && jdev->di8device) {
		if (jdev->di8device) {
			IDirectInputDevice8_Release(jdev->di8device);
			jdev->di8device = NULL;
		}
		if (jdev->axis_info) {
			free(jdev->axis_info);
			jdev->axis_info = NULL;
		}
		if (jdev->button_offsets) {
			free(jdev->button_offsets);
			jdev->button_offsets = NULL;
		}
	}
	for (i = 0; i < JOY_ST_MAX; i++) {
		if (jdev->states[i].axis) {
			free(jdev->states[i].axis);
			jdev->states[i].axis = NULL;
		}
		if (jdev->states[i].button) {
			free(jdev->states[i].button);
			jdev->states[i].button = NULL;
		}
	}
}
static void js_close_detected_devices(void) {
	if (jstick.jdd.devices) {
		unsigned int i;

		for (i = 0; i < jstick.jdd.count; i++) {
			js_close(&jstick.jdd.devices[i]);
		}
		free(jstick.jdd.devices);
		jstick.jdd.devices = NULL;
	}

	jstick.jdd.count = 0;
}
static BOOL js_guidcmp(GUID *guid1, GUID *guid2) {
	if ((memcmp(guid1, guid2, sizeof(GUID))) == 0) {
		return (TRUE);
	}
	return (FALSE);
}
static BOOL js_is_xinput_dev(const GUID *pGuidProductFromDirectInput, _jstick_raw_devices *raw) {
	static const GUID *s_XInputProductGUID[] = {
		&IID_ValveStreamingGamepad,
		&IID_X360WiredGamepad,
		&IID_X360WirelessGamepad
	};
	unsigned int i;

	for (i = 0; i < LENGTH(s_XInputProductGUID); i++) {
		if (js_guidcmp((GUID *)pGuidProductFromDirectInput, (GUID *)s_XInputProductGUID[i]) == TRUE) {
			return (TRUE);
		}
	}

	if (!raw->count || (raw->list == NULL)) {
		return (FALSE);
	}

	for (i = 0; i < raw->count; i++) {
		RID_DEVICE_INFO rdi;
		char name[128];
		UINT srdi = sizeof(rdi), sname = sizeof(name);

		rdi.cbSize = sizeof(rdi);
		if ((raw->list[i].dwType == RIM_TYPEHID) &&
			(GetRawInputDeviceInfoA(raw->list[i].hDevice, RIDI_DEVICEINFO, &rdi, &srdi) != (UINT)-1) &&
			(MAKELONG(rdi.hid.dwVendorId, rdi.hid.dwProductId) == (LONG) pGuidProductFromDirectInput->Data1) &&
			(GetRawInputDeviceInfoA(raw->list[i].hDevice, RIDI_DEVICENAME, name, &sname) != (UINT)-1) &&
			(strstr(name, "IG_") != NULL)) {
			return (TRUE);
		}
	}

	return (FALSE);
}
static BOOL CALLBACK cb_enum_dev(LPCDIDEVICEINSTANCEW instance, LPVOID context) {
	_jstick_raw_devices *raw = (_jstick_raw_devices *)context;
	_jstick_device *jdev, *jdevs;

	if ((jdevs = (_jstick_device *)realloc(jstick.jdd.devices, (jstick.jdd.count + 1) * sizeof(_jstick_device)))) {
		jstick.jdd.devices = jdevs;
		jdev = &jstick.jdd.devices[jstick.jdd.count];
		jstick.jdd.count++;

		memcpy(&jdev->guid, &instance->guidInstance, sizeof(GUID));
		jdev->product_name = ustrdup(instance->tszProductName);
		jdev->is_xinput = FALSE;

		if (jstick.xinput_available && js_is_xinput_dev(&instance->guidProduct, raw)) {
			jdev->is_xinput = TRUE;
			jdev->xinput_player_index = jstick.xinput_player_index++;
		}

		js_open(jdev);
	}

	return (DIENUM_CONTINUE);
}
static BOOL CALLBACK cb_count_axes(LPCDIDEVICEOBJECTINSTANCEW instance, LPVOID context) {
	_jstick_device *jdev = context;

	jdev->num_axes++;
	if (instance->dwType & DIDFT_POV) {
		jdev->num_axes++;
	}
	return (DIENUM_CONTINUE);
}
static BOOL CALLBACK cb_count_buttons(UNUSED(LPCDIDEVICEOBJECTINSTANCEW instance), LPVOID context) {
	_jstick_device *jdev = context;

	if (jdev->num_buttons < JOY_MAX_BUTTONS) {
		jdev->num_buttons++;
	}
	return (DIENUM_CONTINUE);
}
static BOOL CALLBACK cb_enum_axes(LPCDIDEVICEOBJECTINSTANCEW instance, LPVOID context) {
	_jstick_device *jdev = context;
	DWORD offset;

	jdev->num_axes++;
	if (instance->dwType & DIDFT_POV) {
		offset = DIJOFS_POV(jdev->pov);
		jdev->axis_info[jdev->num_axes - 1].offset = offset;
		jdev->axis_info[jdev->num_axes - 1].is_pov = TRUE;
		jdev->num_axes++;
		jdev->axis_info[jdev->num_axes - 1].offset = offset;
		jdev->axis_info[jdev->num_axes - 1].is_pov = TRUE;
		jdev->pov++;
	} else {
		DIPROPRANGE range;
		DIPROPDWORD deadZone;
		HRESULT rc;

		if (!memcmp(&instance->guidType, &GUID_XAxis, sizeof(instance->guidType))) {
			offset = DIJOFS_X;
		} else if (!memcmp(&instance->guidType, &GUID_YAxis, sizeof(instance->guidType))) {
			offset = DIJOFS_Y;
		} else if (!memcmp(&instance->guidType, &GUID_ZAxis, sizeof(instance->guidType))) {
			offset = DIJOFS_Z;
		} else if (!memcmp(&instance->guidType, &GUID_RxAxis, sizeof(instance->guidType))) {
			offset = DIJOFS_RX;
		} else if (!memcmp(&instance->guidType, &GUID_RyAxis, sizeof(instance->guidType))) {
			offset = DIJOFS_RY;
		} else if (!memcmp(&instance->guidType, &GUID_RzAxis, sizeof(instance->guidType))) {
			offset = DIJOFS_RZ;
		} else if (!memcmp(&instance->guidType, &GUID_Slider, sizeof(instance->guidType))) {
			offset = DIJOFS_SLIDER(jdev->slider++);
		} else {
			offset = -1;
		}
		jdev->axis_info[jdev->num_axes - 1].offset = offset;
		jdev->axis_info[jdev->num_axes - 1].is_pov = FALSE;

		range.diph.dwSize = sizeof(range);
		range.diph.dwHeaderSize = sizeof(range.diph);
		range.diph.dwObj = instance->dwType;
		range.diph.dwHow = DIPH_BYID;
		range.lMin = JOY_AXIS_MIN;
		range.lMax = JOY_AXIS_MAX;

		if ((rc = IDirectInputDevice8_SetProperty(jdev->di8device, DIPROP_RANGE, &range.diph)) != DI_OK) {
			fprintf(stderr, "IDIrectInputDevice8_SetProperty : 0x%X\n", (unsigned int)rc);
		}

		deadZone.diph.dwSize = sizeof(deadZone);
		deadZone.diph.dwHeaderSize = sizeof(deadZone.diph);
		deadZone.diph.dwObj = instance->dwType;
		deadZone.diph.dwHow = DIPH_BYID;
		deadZone.dwData = 0;
		if ((rc = IDirectInputDevice8_SetProperty(jdev->di8device, DIPROP_DEADZONE, &deadZone.diph)) != DI_OK) {
			fprintf(stderr, "IDIrectInputDevice8_SetProperty : 0x%X\n", (unsigned int)rc);
		}
	}
	return (DIENUM_CONTINUE);
}
static BOOL CALLBACK cb_enum_buttons(UNUSED(LPCDIDEVICEOBJECTINSTANCEW instance), LPVOID context) {
	_jstick_device *jdev = context;

	if (jdev->num_buttons < JOY_MAX_BUTTONS) {
		jdev->button_offsets[jdev->num_buttons] = DIJOFS_BUTTON(jdev->num_buttons);
		jdev->num_buttons++;
	}
	return (DIENUM_CONTINUE);
}
INLINE static DBWORD js_update_button(_js *joy, _port *port, joy_states st, BYTE index, BYTE value, BYTE *mode) {
	DBWORD event = 0;

	(*mode) = value;

	if (value != JDEV->states[st].button[index]) {
		JDEV->states[st].button[index] = value;
		event = index | 0x400;
		//if (!state)
		//	wprintf(uL("%d : %s %d\n"), st, jsv_to_name(event), (*mode));
		if (joy->input_decode_event) {
			joy->input_decode_event(value, FALSE, event, JOYSTICK, port);
		}
	}
	return (event);
}
INLINE static DBWORD js_update_axis_float(_js *joy, _port *port, joy_states st, BYTE index, float value, BYTE *mode) {
	float lastValue;
	DBWORD event = (index << 1) + 1;
	(*mode) = PRESSED;

	lastValue = JDEV->states[st].axis[index];

	if ((value < JOY_AXIS_SENSIBILITY) && (value > -JOY_AXIS_SENSIBILITY)) {
		value = 0.0f;
		(*mode) = RELEASED;
		if (lastValue > 0) {
			event++;
		}
	} else if (value <= -JOY_AXIS_SENSIBILITY) {
		value = -1.0f;
	} else {
		value = 1.0f;
		event++;
	}

	JDEV->states[st].axis[index] = value;

	if (value != lastValue) {
		//if (!state)
		//	wprintf(uL("%d : %s %d\n"), st, jsv_to_name(event), (*mode));
		if (joy->input_decode_event) {
			joy->input_decode_event((*mode), FALSE, event, JOYSTICK, port);
		}
		return (event);
	}
	return (0);
}
INLINE static DBWORD js_update_axis(_js *joy, _port *port, joy_states st, BYTE index, LONG value, BYTE *mode) {
	return (js_update_axis_float(joy, port, st, index, JOY_AXIS_TO_FLOAT(value), mode));
}
INLINE static void js_pov_to_xy(DWORD pov, float *x, float *y) {
	if (LOWORD(pov) == 0xFFFF) {
		(*x) = (*y) = 0.0f;
	} else {
		if ((pov > JOY_POV_UP) && (pov < JOY_POV_DOWN)) {
			(*x) = 1.0f;
		} else if (pov > JOY_POV_DOWN) {
			(*x) = -1.0f;
		} else {
			(*x) = 0.0f;
		}

		if ((pov > JOY_POV_LEFT) || (pov < JOY_POV_RIGHT)) {
			(*y) = -1.0f;
		} else if ((pov > JOY_POV_RIGHT) && (pov < JOY_POV_LEFT)) {
			(*y) = 1.0f;
		} else {
			(*y) = 0.0f;
		}
	}
}
INLINE static DBWORD js_update_pov(_js *joy, _port *port, joy_states st, BYTE index, DWORD value, BYTE *mode) {
	DBWORD event = 0;
	float x = 0.0f, y = 0.0f;

	js_pov_to_xy(value, &x, &y);
	if (!(event = js_update_axis_float(joy, port, st, index, x, mode))) {
		event = js_update_axis_float(joy, port, st, index + 1, y, mode);
	};
	return (event);
}
INLINE static void js_lock(void) {
	thread_mutex_lock(jstick.lock);
}
INLINE static void js_unlock(void) {
	thread_mutex_unlock(jstick.lock);
}
