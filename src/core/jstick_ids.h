/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

#ifndef JSTICK_IDS_H_
#define JSTICK_IDS_H_

#define MAKE_CONTROLLER_ID(vid, pid) vid, pid
#define k_eControllerType_XBox360Controller JS_SC_MS_XBOX_360_GAMEPAD
#define k_eControllerType_XBoxOneController JS_SC_MS_XBOX_ONE_GAMEPAD
#define k_eControllerType_PS4Controller JS_SC_SONY_PS4_GAMEPAD
#define k_eControllerType_PS3Controller JS_SC_SONY_PS3_GAMEPAD

enum _js_gamepad_type {
	JS_SC_UNKNOWN,
	JS_SC_MS_XBOX_360_GAMEPAD,
	JS_SC_MS_XBOX_ONE_GAMEPAD,
	JS_SC_SONY_PS3_GAMEPAD,
	JS_SC_SONY_PS4_GAMEPAD
};
enum _js_usb_vendor_id {
	JS_USB_VENDOR_ID_VALVE = 0x28DE
};
enum _js_usb_product_id {
	JS_USB_PID_STEAM_VIRTUAL_GAMEPAD = 0x11FF
};

typedef struct _js_gamepad {
	long int vendor_id;
	long int product_id;
	enum _js_gamepad_type type;
	const uTCHAR *desc;
} _js_gamepad;

// Thx to SDL2.0 for this list
// https://github.com/libsdl-org/SDL/blob/main/src/joystick/controller_type.h
static const _js_gamepad js_gamepads_list[] =
{
	{ MAKE_CONTROLLER_ID( 0x28DE, 0x1102 ), k_eControllerType_XBox360Controller, uL("Valve Software Wired Controller") },	// Valve Software Wired Controller
	{ MAKE_CONTROLLER_ID( 0x0c12, 0x0e13 ), k_eControllerType_PS4Controller, NULL },    // Xtreme 90418

	{ MAKE_CONTROLLER_ID( 0x0079, 0x181a ), k_eControllerType_PS3Controller, NULL },	// Venom Arcade Stick
	{ MAKE_CONTROLLER_ID( 0x0079, 0x1844 ), k_eControllerType_PS3Controller, NULL },	// From SDL
	{ MAKE_CONTROLLER_ID( 0x044f, 0xb315 ), k_eControllerType_PS3Controller, NULL },	// Firestorm Dual Analog 3
	{ MAKE_CONTROLLER_ID( 0x044f, 0xd007 ), k_eControllerType_PS3Controller, NULL },	// Thrustmaster wireless 3-1
	{ MAKE_CONTROLLER_ID( 0x054c, 0x0268 ), k_eControllerType_PS3Controller, NULL },	// Sony PS3 Controller
	{ MAKE_CONTROLLER_ID( 0x056e, 0x200f ), k_eControllerType_PS3Controller, NULL },	// From SDL
	{ MAKE_CONTROLLER_ID( 0x056e, 0x2013 ), k_eControllerType_PS3Controller, NULL },	// JC-U4113SBK
	{ MAKE_CONTROLLER_ID( 0x05b8, 0x1004 ), k_eControllerType_PS3Controller, NULL },	// From SDL
	{ MAKE_CONTROLLER_ID( 0x05b8, 0x1006 ), k_eControllerType_PS3Controller, NULL },	// JC-U3412SBK
	{ MAKE_CONTROLLER_ID( 0x06a3, 0xf622 ), k_eControllerType_PS3Controller, NULL },	// Cyborg V3
	{ MAKE_CONTROLLER_ID( 0x0738, 0x3180 ), k_eControllerType_PS3Controller, NULL },	// Mad Catz Alpha PS3 mode
	{ MAKE_CONTROLLER_ID( 0x0738, 0x3250 ), k_eControllerType_PS3Controller, NULL },	// madcats fightpad pro ps3
	{ MAKE_CONTROLLER_ID( 0x0738, 0x8180 ), k_eControllerType_PS3Controller, NULL },	// Mad Catz Alpha PS4 mode (no touchpad on device)
	{ MAKE_CONTROLLER_ID( 0x0738, 0x8838 ), k_eControllerType_PS3Controller, NULL },	// Madcatz Fightstick Pro
	{ MAKE_CONTROLLER_ID( 0x0810, 0x0001 ), k_eControllerType_PS3Controller, NULL },	// actually ps2 - maybe break out later
	{ MAKE_CONTROLLER_ID( 0x0810, 0x0003 ), k_eControllerType_PS3Controller, NULL },	// actually ps2 - maybe break out later
	{ MAKE_CONTROLLER_ID( 0x0925, 0x0005 ), k_eControllerType_PS3Controller, NULL },	// Sony PS3 Controller
	{ MAKE_CONTROLLER_ID( 0x0925, 0x8866 ), k_eControllerType_PS3Controller, NULL },	// PS2 maybe break out later
	{ MAKE_CONTROLLER_ID( 0x0925, 0x8888 ), k_eControllerType_PS3Controller, NULL },	// Actually ps2 -maybe break out later Lakeview Research WiseGroup Ltd, MP-8866 Dual Joypad
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0109 ), k_eControllerType_PS3Controller, NULL },	// PDP Versus Fighting Pad
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x011e ), k_eControllerType_PS3Controller, NULL },	// Rock Candy PS4
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0128 ), k_eControllerType_PS3Controller, NULL },	// Rock Candy PS3
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0203 ), k_eControllerType_PS3Controller, NULL },	// Victrix Pro FS (PS4 peripheral but no trackpad/lightbar)
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0214 ), k_eControllerType_PS3Controller, NULL },	// afterglow ps3
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x1314 ), k_eControllerType_PS3Controller, NULL },	// PDP Afterglow Wireless PS3 controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x6302 ), k_eControllerType_PS3Controller, NULL },	// From SDL
	{ MAKE_CONTROLLER_ID( 0x0e8f, 0x0008 ), k_eControllerType_PS3Controller, NULL },	// Green Asia
	{ MAKE_CONTROLLER_ID( 0x0e8f, 0x3075 ), k_eControllerType_PS3Controller, NULL },	// SpeedLink Strike FX
	{ MAKE_CONTROLLER_ID( 0x0e8f, 0x310d ), k_eControllerType_PS3Controller, NULL },	// From SDL
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x0009 ), k_eControllerType_PS3Controller, NULL },	// HORI BDA GP1
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x004d ), k_eControllerType_PS3Controller, NULL },	// Horipad 3
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x005e ), k_eControllerType_PS3Controller, NULL },	// HORI Fighting commander ps4
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x005f ), k_eControllerType_PS3Controller, NULL },	// HORI Fighting commander ps3
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x006a ), k_eControllerType_PS3Controller, NULL },	// Real Arcade Pro 4
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x006e ), k_eControllerType_PS3Controller, NULL },	// HORI horipad4 ps3
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x0085 ), k_eControllerType_PS3Controller, NULL },	// HORI Fighting Commander PS3
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x0086 ), k_eControllerType_PS3Controller, NULL },	// HORI Fighting Commander PC (Uses the Xbox 360 protocol, but has PS3 buttons)
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x0087 ), k_eControllerType_PS3Controller, NULL },	// HORI fighting mini stick
	{ MAKE_CONTROLLER_ID( 0x0f30, 0x1100 ), k_eControllerType_PS3Controller, NULL },	// Quanba Q1 fight stick
	{ MAKE_CONTROLLER_ID( 0x11ff, 0x3331 ), k_eControllerType_PS3Controller, NULL },	// SRXJ-PH2400
	{ MAKE_CONTROLLER_ID( 0x1345, 0x1000 ), k_eControllerType_PS3Controller, NULL },	// PS2 ACME GA-D5
	{ MAKE_CONTROLLER_ID( 0x1345, 0x6005 ), k_eControllerType_PS3Controller, NULL },	// ps2 maybe break out later
	{ MAKE_CONTROLLER_ID( 0x146b, 0x0603 ), k_eControllerType_PS3Controller, NULL },	// From SDL
	{ MAKE_CONTROLLER_ID( 0x146b, 0x5500 ), k_eControllerType_PS3Controller, NULL },	// From SDL
	{ MAKE_CONTROLLER_ID( 0x1a34, 0x0836 ), k_eControllerType_PS3Controller, NULL },	// Afterglow PS3
	{ MAKE_CONTROLLER_ID( 0x20bc, 0x5500 ), k_eControllerType_PS3Controller, NULL },	// ShanWan PS3
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x576d ), k_eControllerType_PS3Controller, NULL },	// Power A PS3
	{ MAKE_CONTROLLER_ID( 0x20d6, 0xca6d ), k_eControllerType_PS3Controller, NULL },	// From SDL
	{ MAKE_CONTROLLER_ID( 0x2563, 0x0523 ), k_eControllerType_PS3Controller, NULL },	// Digiflip GP006
	{ MAKE_CONTROLLER_ID( 0x2563, 0x0575 ), k_eControllerType_PS3Controller, NULL },	// From SDL
	{ MAKE_CONTROLLER_ID( 0x25f0, 0x83c3 ), k_eControllerType_PS3Controller, NULL },	// gioteck vx2
	{ MAKE_CONTROLLER_ID( 0x25f0, 0xc121 ), k_eControllerType_PS3Controller, NULL },	//
	{ MAKE_CONTROLLER_ID( 0x2c22, 0x2000 ), k_eControllerType_PS3Controller, NULL },	// Quanba Drone
	{ MAKE_CONTROLLER_ID( 0x2c22, 0x2003 ), k_eControllerType_PS3Controller, NULL },	// From SDL
	{ MAKE_CONTROLLER_ID( 0x8380, 0x0003 ), k_eControllerType_PS3Controller, NULL },	// BTP 2163
	{ MAKE_CONTROLLER_ID( 0x8888, 0x0308 ), k_eControllerType_PS3Controller, NULL },	// Sony PS3 Controller

	{ MAKE_CONTROLLER_ID( 0x0079, 0x181b ), k_eControllerType_PS4Controller, NULL },	// Venom Arcade Stick - XXX:this may not work and may need to be called a ps3 controller
	{ MAKE_CONTROLLER_ID( 0x054c, 0x05c4 ), k_eControllerType_PS4Controller, NULL },	// Sony PS4 Controller
	{ MAKE_CONTROLLER_ID( 0x054c, 0x05c5 ), k_eControllerType_PS4Controller, NULL },	// STRIKEPAD PS4 Grip Add-on
	{ MAKE_CONTROLLER_ID( 0x054c, 0x09cc ), k_eControllerType_PS4Controller, NULL },	// Sony PS4 Slim Controller
	{ MAKE_CONTROLLER_ID( 0x054c, 0x0ba0 ), k_eControllerType_PS4Controller, NULL },	// Sony PS4 Controller (Wireless dongle)
	{ MAKE_CONTROLLER_ID( 0x0738, 0x8250 ), k_eControllerType_PS4Controller, NULL },	// Mad Catz FightPad Pro PS4
	{ MAKE_CONTROLLER_ID( 0x0738, 0x8384 ), k_eControllerType_PS4Controller, NULL },	// Mad Catz FightStick TE S+ PS4
	{ MAKE_CONTROLLER_ID( 0x0738, 0x8480 ), k_eControllerType_PS4Controller, NULL },	// Mad Catz FightStick TE 2 PS4
	{ MAKE_CONTROLLER_ID( 0x0738, 0x8481 ), k_eControllerType_PS4Controller, NULL },	// Mad Catz FightStick TE 2+ PS4
	{ MAKE_CONTROLLER_ID( 0x0C12, 0x0E10 ), k_eControllerType_PS4Controller, NULL },	// Armor Armor 3 Pad PS4
	{ MAKE_CONTROLLER_ID( 0x0C12, 0x1CF6 ), k_eControllerType_PS4Controller, NULL },	// EMIO PS4 Elite Controller
	{ MAKE_CONTROLLER_ID( 0x0c12, 0x0e15 ), k_eControllerType_PS4Controller, NULL },	// Game:Pad 4
	{ MAKE_CONTROLLER_ID( 0x0c12, 0x0ef6 ), k_eControllerType_PS4Controller, NULL },	// Hitbox Arcade Stick
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x0055 ), k_eControllerType_PS4Controller, NULL },	// HORIPAD 4 FPS
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x0066 ), k_eControllerType_PS4Controller, NULL },	// HORIPAD 4 FPS Plus
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x0084 ), k_eControllerType_PS4Controller, NULL },	// HORI Fighting Commander PS4
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x008a ), k_eControllerType_PS4Controller, NULL },	// HORI Real Arcade Pro 4
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x009c ), k_eControllerType_PS4Controller, NULL },	// HORI TAC PRO mousething
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x00a0 ), k_eControllerType_PS4Controller, NULL },	// HORI TAC4 mousething
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x00ee ), k_eControllerType_PS4Controller, NULL },	// Hori mini wired https://www.playstation.com/en-us/explore/accessories/gaming-controllers/mini-wired-gamepad/
	{ MAKE_CONTROLLER_ID( 0x11c0, 0x4001 ), k_eControllerType_PS4Controller, NULL },	// "PS4 Fun Controller" added from user log
	{ MAKE_CONTROLLER_ID( 0x146b, 0x0d01 ), k_eControllerType_PS4Controller, NULL },	// Nacon Revolution Pro Controller - has gyro
	{ MAKE_CONTROLLER_ID( 0x146b, 0x0d02 ), k_eControllerType_PS4Controller, NULL },	// Nacon Revolution Pro Controller v2 - has gyro
	{ MAKE_CONTROLLER_ID( 0x146b, 0x0d10 ), k_eControllerType_PS4Controller, NULL },	// NACON Revolution Infinite - has gyro
	{ MAKE_CONTROLLER_ID( 0x1532, 0X0401 ), k_eControllerType_PS4Controller, NULL },	// Razer Panthera PS4 Controller
	{ MAKE_CONTROLLER_ID( 0x1532, 0x1000 ), k_eControllerType_PS4Controller, NULL },	// Razer Raiju PS4 Controller
	{ MAKE_CONTROLLER_ID( 0x1532, 0x1004 ), k_eControllerType_PS4Controller, NULL },	// Razer Raiju 2 Ultimate USB
	{ MAKE_CONTROLLER_ID( 0x1532, 0x1007 ), k_eControllerType_PS4Controller, NULL },	// Razer Raiju 2 Tournament edition USB
	{ MAKE_CONTROLLER_ID( 0x1532, 0x1008 ), k_eControllerType_PS4Controller, NULL },	// Razer Panthera Evo Fightstick
	{ MAKE_CONTROLLER_ID( 0x1532, 0x1009 ), k_eControllerType_PS4Controller, NULL },	// Razer Raiju 2 Ultimate BT
	{ MAKE_CONTROLLER_ID( 0x1532, 0x100A ), k_eControllerType_PS4Controller, NULL },	// Razer Raiju 2 Tournament edition BT
	{ MAKE_CONTROLLER_ID( 0x1532, 0x1100 ), k_eControllerType_PS4Controller, NULL },	// Razer RAION Fightpad - Trackpad, no gyro, lightbar hardcoded to green
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x792a ), k_eControllerType_PS4Controller, NULL },	// PowerA Fusion Fight Pad
	{ MAKE_CONTROLLER_ID( 0x7545, 0x0104 ), k_eControllerType_PS4Controller, NULL },	// Armor 3 or Level Up Cobra - At least one variant has gyro
	{ MAKE_CONTROLLER_ID( 0x9886, 0x0025 ), k_eControllerType_PS4Controller, NULL },	// Astro C40
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0207 ), k_eControllerType_PS4Controller, NULL },	// Victrix Pro Fightstick w/ Touchpad for PS4
	// Removing the Giotek because there were a bunch of help tickets from users w/ issues including from non-PS4 controller users. This VID/PID is probably used in different FW's
	// { MAKE_CONTROLLER_ID( 0x7545, 0x1122 ), k_eControllerType_PS4Controller, NULL },	// Giotek VX4 - trackpad/gyro don't work. Had to not filter on interface info. Light bar is flaky, but works.
	{ MAKE_CONTROLLER_ID( 0x044f, 0xd00e ), k_eControllerType_PS4Controller, NULL },	// Thrustmast Eswap Pro - No gyro and lightbar doesn't change color. Works otherwise
	{ MAKE_CONTROLLER_ID( 0x0c12, 0x1e10 ), k_eControllerType_PS4Controller, NULL },	// P4 Wired Gamepad generic knock off - lightbar but not trackpad or gyro
	{ MAKE_CONTROLLER_ID( 0x146b, 0x0d09 ), k_eControllerType_PS4Controller, NULL },	// NACON Daija Fight Stick - touchpad but no gyro/rumble
	{ MAKE_CONTROLLER_ID( 0x146b, 0x0d10 ), k_eControllerType_PS4Controller, NULL },	// NACON Revolution Unlimited
	{ MAKE_CONTROLLER_ID( 0x146b, 0x0d08 ), k_eControllerType_PS4Controller, NULL },	// NACON Revolution Unlimited Wireless Dongle
	{ MAKE_CONTROLLER_ID( 0x146b, 0x0d06 ), k_eControllerType_PS4Controller, NULL },	// NACON Asymetrical Controller Wireless Dongle -- show up as ps4 until you connect controller to it then it reboots into Xbox controller with different vvid/pid
	{ MAKE_CONTROLLER_ID( 0x146b, 0x1103 ), k_eControllerType_PS4Controller, NULL },	// NACON Asymetrical Controller -- on windows this doesn't enumerate
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x0123 ), k_eControllerType_PS4Controller, NULL },	// HORI Wireless Controller Light (Japan only) - only over bt- over usb is xbox and pid 0x0124
	{ MAKE_CONTROLLER_ID( 0x146b, 0x0d13 ), k_eControllerType_PS4Controller, NULL },	// NACON Revolution 3
	{ MAKE_CONTROLLER_ID( 0x0c12, 0x0e20 ), k_eControllerType_PS4Controller, NULL },	// Brook Mars Controller - needs FW update to show up as Ps4 controller on PC. Has Gyro but touchpad is a single button.

	{ MAKE_CONTROLLER_ID( 0x0079, 0x18d4 ), k_eControllerType_XBox360Controller, NULL },	// GPD Win 2 X-Box Controller
	{ MAKE_CONTROLLER_ID( 0x03eb, 0xff02 ), k_eControllerType_XBox360Controller, NULL },	// Wooting Two
	{ MAKE_CONTROLLER_ID( 0x044f, 0xb326 ), k_eControllerType_XBox360Controller, NULL },	// Thrustmaster Gamepad GP XID
	{ MAKE_CONTROLLER_ID( 0x045e, 0x028e ), k_eControllerType_XBox360Controller, uL("Xbox 360 Controller") },	// Microsoft X-Box 360 pad
	{ MAKE_CONTROLLER_ID( 0x045e, 0x028f ), k_eControllerType_XBox360Controller, uL("Xbox 360 Controller") },	// Microsoft X-Box 360 pad v2
	{ MAKE_CONTROLLER_ID( 0x045e, 0x0291 ), k_eControllerType_XBox360Controller, uL("Xbox 360 Wireless Controller") },	// Xbox 360 Wireless Receiver (XBOX)
	{ MAKE_CONTROLLER_ID( 0x045e, 0x02a0 ), k_eControllerType_XBox360Controller, NULL },	// Microsoft X-Box 360 Big Button IR
	{ MAKE_CONTROLLER_ID( 0x045e, 0x02a1 ), k_eControllerType_XBox360Controller, NULL },	// Microsoft X-Box 360 Wireless Controller with XUSB driver on Windows
	{ MAKE_CONTROLLER_ID( 0x045e, 0x02a9 ), k_eControllerType_XBox360Controller, uL("Xbox 360 Wireless Controller") },	// Xbox 360 Wireless Receiver (third party knockoff)
	{ MAKE_CONTROLLER_ID( 0x045e, 0x0719 ), k_eControllerType_XBox360Controller, uL("Xbox 360 Wireless Controller") },	// Xbox 360 Wireless Receiver
	{ MAKE_CONTROLLER_ID( 0x046d, 0xc21d ), k_eControllerType_XBox360Controller, NULL },	// Logitech Gamepad F310
	{ MAKE_CONTROLLER_ID( 0x046d, 0xc21e ), k_eControllerType_XBox360Controller, NULL },	// Logitech Gamepad F510
	{ MAKE_CONTROLLER_ID( 0x046d, 0xc21f ), k_eControllerType_XBox360Controller, NULL },	// Logitech Gamepad F710
	{ MAKE_CONTROLLER_ID( 0x046d, 0xc242 ), k_eControllerType_XBox360Controller, NULL },	// Logitech Chillstream Controller
	{ MAKE_CONTROLLER_ID( 0x056e, 0x2004 ), k_eControllerType_XBox360Controller, NULL },	// Elecom JC-U3613M
	{ MAKE_CONTROLLER_ID( 0x06a3, 0xf51a ), k_eControllerType_XBox360Controller, NULL },	// Saitek P3600
	{ MAKE_CONTROLLER_ID( 0x0738, 0x4716 ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz Wired Xbox 360 Controller
	{ MAKE_CONTROLLER_ID( 0x0738, 0x4718 ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz Street Fighter IV FightStick SE
	{ MAKE_CONTROLLER_ID( 0x0738, 0x4726 ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz Xbox 360 Controller
	{ MAKE_CONTROLLER_ID( 0x0738, 0x4728 ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz Street Fighter IV FightPad
	{ MAKE_CONTROLLER_ID( 0x0738, 0x4736 ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz MicroCon Gamepad
	{ MAKE_CONTROLLER_ID( 0x0738, 0x4738 ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz Wired Xbox 360 Controller (SFIV)
	{ MAKE_CONTROLLER_ID( 0x0738, 0x4740 ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz Beat Pad
	{ MAKE_CONTROLLER_ID( 0x0738, 0xb726 ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz Xbox controller - MW2
	{ MAKE_CONTROLLER_ID( 0x0738, 0xbeef ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz JOYTECH NEO SE Advanced GamePad
	{ MAKE_CONTROLLER_ID( 0x0738, 0xcb02 ), k_eControllerType_XBox360Controller, NULL },	// Saitek Cyborg Rumble Pad - PC/Xbox 360
	{ MAKE_CONTROLLER_ID( 0x0738, 0xcb03 ), k_eControllerType_XBox360Controller, NULL },	// Saitek P3200 Rumble Pad - PC/Xbox 360
	{ MAKE_CONTROLLER_ID( 0x0738, 0xf738 ), k_eControllerType_XBox360Controller, NULL },	// Super SFIV FightStick TE S
	{ MAKE_CONTROLLER_ID( 0x0955, 0x7210 ), k_eControllerType_XBox360Controller, NULL },	// Nvidia Shield local controller
	{ MAKE_CONTROLLER_ID( 0x0955, 0xb400 ), k_eControllerType_XBox360Controller, NULL },	// NVIDIA Shield streaming controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0105 ), k_eControllerType_XBox360Controller, NULL },	// HSM3 Xbox360 dancepad
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0113 ), k_eControllerType_XBox360Controller, uL("PDP Xbox 360 Afterglow") },	// PDP Afterglow Gamepad for Xbox 360
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x011f ), k_eControllerType_XBox360Controller, uL("PDP Xbox 360 Rock Candy") },	// PDP Rock Candy Gamepad for Xbox 360
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0125 ), k_eControllerType_XBox360Controller, uL("PDP INJUSTICE FightStick") },	// PDP INJUSTICE FightStick for Xbox 360
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0127 ), k_eControllerType_XBox360Controller, uL("PDP INJUSTICE FightPad") },	// PDP INJUSTICE FightPad for Xbox 360
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0131 ), k_eControllerType_XBox360Controller, uL("PDP EA Soccer Controller") },	// PDP EA Soccer Gamepad
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0133 ), k_eControllerType_XBox360Controller, uL("PDP Battlefield 4 Controller") },	// PDP Battlefield 4 Gamepad
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0143 ), k_eControllerType_XBox360Controller, uL("PDP MK X Fight Stick") },	// PDP MK X Fight Stick for Xbox 360
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0147 ), k_eControllerType_XBox360Controller, uL("PDP Xbox 360 Marvel Controller") },	// PDP Marvel Controller for Xbox 360
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0201 ), k_eControllerType_XBox360Controller, uL("PDP Xbox 360 Controller") },	// PDP Gamepad for Xbox 360
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0213 ), k_eControllerType_XBox360Controller, uL("PDP Xbox 360 Afterglow") },	// PDP Afterglow Gamepad for Xbox 360
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x021f ), k_eControllerType_XBox360Controller, uL("PDP Xbox 360 Rock Candy") },	// PDP Rock Candy Gamepad for Xbox 360
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0301 ), k_eControllerType_XBox360Controller, uL("PDP Xbox 360 Controller") },	// PDP Gamepad for Xbox 360
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0313 ), k_eControllerType_XBox360Controller, uL("PDP Xbox 360 Afterglow") },	// PDP Afterglow Gamepad for Xbox 360
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0314 ), k_eControllerType_XBox360Controller, uL("PDP Xbox 360 Afterglow") },	// PDP Afterglow Gamepad for Xbox 360
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0401 ), k_eControllerType_XBox360Controller, uL("PDP Xbox 360 Controller") },	// PDP Gamepad for Xbox 360
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0413 ), k_eControllerType_XBox360Controller, NULL },	// PDP Afterglow AX.1 (unlisted)
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0501 ), k_eControllerType_XBox360Controller, NULL },	// PDP Xbox 360 Controller (unlisted)
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0xf900 ), k_eControllerType_XBox360Controller, NULL },	// PDP Afterglow AX.1 (unlisted)
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x000a ), k_eControllerType_XBox360Controller, NULL },	// Hori Co. DOA4 FightStick
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x000c ), k_eControllerType_XBox360Controller, NULL },	// Hori PadEX Turbo
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x000d ), k_eControllerType_XBox360Controller, NULL },	// Hori Fighting Stick EX2
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x0016 ), k_eControllerType_XBox360Controller, NULL },	// Hori Real Arcade Pro.EX
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x001b ), k_eControllerType_XBox360Controller, NULL },	// Hori Real Arcade Pro VX
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x008c ), k_eControllerType_XBox360Controller, NULL },	// Hori Real Arcade Pro 4
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x00db ), k_eControllerType_XBox360Controller, uL("HORI Slime Controller") },	// Hori Dragon Quest Slime Controller
	{ MAKE_CONTROLLER_ID( 0x1038, 0x1430 ), k_eControllerType_XBox360Controller, uL("SteelSeries Stratus Duo") },	// SteelSeries Stratus Duo
	{ MAKE_CONTROLLER_ID( 0x1038, 0x1431 ), k_eControllerType_XBox360Controller, uL("SteelSeries Stratus Duo") },	// SteelSeries Stratus Duo
	{ MAKE_CONTROLLER_ID( 0x1038, 0xb360 ), k_eControllerType_XBox360Controller, NULL },	// SteelSeries Nimbus/Stratus XL
	{ MAKE_CONTROLLER_ID( 0x11c9, 0x55f0 ), k_eControllerType_XBox360Controller, NULL },	// Nacon GC-100XF
	{ MAKE_CONTROLLER_ID( 0x12ab, 0x0004 ), k_eControllerType_XBox360Controller, NULL },	// Honey Bee Xbox360 dancepad
	{ MAKE_CONTROLLER_ID( 0x12ab, 0x0301 ), k_eControllerType_XBox360Controller, NULL },	// PDP AFTERGLOW AX.1
	{ MAKE_CONTROLLER_ID( 0x12ab, 0x0303 ), k_eControllerType_XBox360Controller, NULL },	// Mortal Kombat Klassic FightStick
	{ MAKE_CONTROLLER_ID( 0x1430, 0x02a0 ), k_eControllerType_XBox360Controller, NULL },	// RedOctane Controller Adapter
	{ MAKE_CONTROLLER_ID( 0x1430, 0x4748 ), k_eControllerType_XBox360Controller, NULL },	// RedOctane Guitar Hero X-plorer
	{ MAKE_CONTROLLER_ID( 0x1430, 0xf801 ), k_eControllerType_XBox360Controller, NULL },	// RedOctane Controller
	{ MAKE_CONTROLLER_ID( 0x146b, 0x0601 ), k_eControllerType_XBox360Controller, NULL },	// BigBen Interactive XBOX 360 Controller
//	{ MAKE_CONTROLLER_ID( 0x1532, 0x0037 ), k_eControllerType_XBox360Controller, NULL },	// Razer Sabertooth
	{ MAKE_CONTROLLER_ID( 0x15e4, 0x3f00 ), k_eControllerType_XBox360Controller, NULL },	// Power A Mini Pro Elite
	{ MAKE_CONTROLLER_ID( 0x15e4, 0x3f0a ), k_eControllerType_XBox360Controller, NULL },	// Xbox Airflo wired controller
	{ MAKE_CONTROLLER_ID( 0x15e4, 0x3f10 ), k_eControllerType_XBox360Controller, NULL },	// Batarang Xbox 360 controller
	{ MAKE_CONTROLLER_ID( 0x162e, 0xbeef ), k_eControllerType_XBox360Controller, NULL },	// Joytech Neo-Se Take2
	{ MAKE_CONTROLLER_ID( 0x1689, 0xfd00 ), k_eControllerType_XBox360Controller, NULL },	// Razer Onza Tournament Edition
	{ MAKE_CONTROLLER_ID( 0x1689, 0xfd01 ), k_eControllerType_XBox360Controller, NULL },	// Razer Onza Classic Edition
	{ MAKE_CONTROLLER_ID( 0x1689, 0xfe00 ), k_eControllerType_XBox360Controller, NULL },	// Razer Sabertooth
	{ MAKE_CONTROLLER_ID( 0x1949, 0x041a ), k_eControllerType_XBox360Controller, uL("Amazon Luna Controller") },	// Amazon Luna Controller
	{ MAKE_CONTROLLER_ID( 0x1bad, 0x0002 ), k_eControllerType_XBox360Controller, NULL },	// Harmonix Rock Band Guitar
	{ MAKE_CONTROLLER_ID( 0x1bad, 0x0003 ), k_eControllerType_XBox360Controller, NULL },	// Harmonix Rock Band Drumkit
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf016 ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz Xbox 360 Controller
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf018 ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz Street Fighter IV SE Fighting Stick
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf019 ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz Brawlstick for Xbox 360
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf021 ), k_eControllerType_XBox360Controller, NULL },	// Mad Cats Ghost Recon FS GamePad
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf023 ), k_eControllerType_XBox360Controller, NULL },	// MLG Pro Circuit Controller (Xbox)
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf025 ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz Call Of Duty
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf027 ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz FPS Pro
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf028 ), k_eControllerType_XBox360Controller, NULL },	// Street Fighter IV FightPad
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf02e ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz Fightpad
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf036 ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz MicroCon GamePad Pro
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf038 ), k_eControllerType_XBox360Controller, NULL },	// Street Fighter IV FightStick TE
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf039 ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz MvC2 TE
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf03a ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz SFxT Fightstick Pro
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf03d ), k_eControllerType_XBox360Controller, NULL },	// Street Fighter IV Arcade Stick TE - Chun Li
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf03e ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz MLG FightStick TE
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf03f ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz FightStick SoulCaliber
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf042 ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz FightStick TES+
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf080 ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz FightStick TE2
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf501 ), k_eControllerType_XBox360Controller, NULL },	// HoriPad EX2 Turbo
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf502 ), k_eControllerType_XBox360Controller, NULL },	// Hori Real Arcade Pro.VX SA
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf503 ), k_eControllerType_XBox360Controller, NULL },	// Hori Fighting Stick VX
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf504 ), k_eControllerType_XBox360Controller, NULL },	// Hori Real Arcade Pro. EX
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf505 ), k_eControllerType_XBox360Controller, NULL },	// Hori Fighting Stick EX2B
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf506 ), k_eControllerType_XBox360Controller, NULL },	// Hori Real Arcade Pro.EX Premium VLX
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf900 ), k_eControllerType_XBox360Controller, NULL },	// Harmonix Xbox 360 Controller
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf901 ), k_eControllerType_XBox360Controller, NULL },	// Gamestop Xbox 360 Controller
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf902 ), k_eControllerType_XBox360Controller, NULL },	// Mad Catz Gamepad2
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf903 ), k_eControllerType_XBox360Controller, NULL },	// Tron Xbox 360 controller
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf904 ), k_eControllerType_XBox360Controller, NULL },	// PDP Versus Fighting Pad
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xf906 ), k_eControllerType_XBox360Controller, NULL },	// MortalKombat FightStick
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xfa01 ), k_eControllerType_XBox360Controller, NULL },	// MadCatz GamePad
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xfd00 ), k_eControllerType_XBox360Controller, NULL },	// Razer Onza TE
	{ MAKE_CONTROLLER_ID( 0x1bad, 0xfd01 ), k_eControllerType_XBox360Controller, NULL },	// Razer Onza
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x5000 ), k_eControllerType_XBox360Controller, NULL },	// Razer Atrox Arcade Stick
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x5300 ), k_eControllerType_XBox360Controller, NULL },	// PowerA MINI PROEX Controller
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x5303 ), k_eControllerType_XBox360Controller, NULL },	// Xbox Airflo wired controller
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x530a ), k_eControllerType_XBox360Controller, NULL },	// Xbox 360 Pro EX Controller
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x531a ), k_eControllerType_XBox360Controller, NULL },	// PowerA Pro Ex
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x5397 ), k_eControllerType_XBox360Controller, NULL },	// FUS1ON Tournament Controller
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x5500 ), k_eControllerType_XBox360Controller, NULL },	// Hori XBOX 360 EX 2 with Turbo
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x5501 ), k_eControllerType_XBox360Controller, NULL },	// Hori Real Arcade Pro VX-SA
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x5502 ), k_eControllerType_XBox360Controller, NULL },	// Hori Fighting Stick VX Alt
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x5503 ), k_eControllerType_XBox360Controller, NULL },	// Hori Fighting Edge
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x5506 ), k_eControllerType_XBox360Controller, NULL },	// Hori SOULCALIBUR V Stick
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x550d ), k_eControllerType_XBox360Controller, NULL },	// Hori GEM Xbox controller
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x550e ), k_eControllerType_XBox360Controller, NULL },	// Hori Real Arcade Pro V Kai 360
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x5508 ), k_eControllerType_XBox360Controller, NULL },	// Hori PAD A
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x5510 ), k_eControllerType_XBox360Controller, NULL },	// Hori Fighting Commander ONE
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x5b00 ), k_eControllerType_XBox360Controller, NULL },	// ThrustMaster Ferrari Italia 458 Racing Wheel
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x5b02 ), k_eControllerType_XBox360Controller, NULL },	// Thrustmaster, Inc. GPX Controller
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x5b03 ), k_eControllerType_XBox360Controller, NULL },	// Thrustmaster Ferrari 458 Racing Wheel
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x5d04 ), k_eControllerType_XBox360Controller, NULL },	// Razer Sabertooth
	{ MAKE_CONTROLLER_ID( 0x24c6, 0xfafa ), k_eControllerType_XBox360Controller, NULL },	// Aplay Controller
	{ MAKE_CONTROLLER_ID( 0x24c6, 0xfafb ), k_eControllerType_XBox360Controller, NULL },	// Aplay Controller
	{ MAKE_CONTROLLER_ID( 0x24c6, 0xfafc ), k_eControllerType_XBox360Controller, NULL },	// Afterglow Gamepad 1
	{ MAKE_CONTROLLER_ID( 0x24c6, 0xfafd ), k_eControllerType_XBox360Controller, NULL },	// Afterglow Gamepad 3
	{ MAKE_CONTROLLER_ID( 0x24c6, 0xfafe ), k_eControllerType_XBox360Controller, NULL },	// Rock Candy Gamepad for Xbox 360

	{ MAKE_CONTROLLER_ID( 0x045e, 0x02d1 ), k_eControllerType_XBoxOneController, uL("Xbox One Controller") },	// Microsoft X-Box One pad
	{ MAKE_CONTROLLER_ID( 0x045e, 0x02dd ), k_eControllerType_XBoxOneController, uL("Xbox One Controller") },	// Microsoft X-Box One pad (Firmware 2015)
	{ MAKE_CONTROLLER_ID( 0x045e, 0x02e0 ), k_eControllerType_XBoxOneController, uL("Xbox One S Controller") },	// Microsoft X-Box One S pad (Bluetooth)
	{ MAKE_CONTROLLER_ID( 0x045e, 0x02e3 ), k_eControllerType_XBoxOneController, uL("Xbox One Elite Controller") },	// Microsoft X-Box One Elite pad
	{ MAKE_CONTROLLER_ID( 0x045e, 0x02ea ), k_eControllerType_XBoxOneController, uL("Xbox One S Controller") },	// Microsoft X-Box One S pad
	{ MAKE_CONTROLLER_ID( 0x045e, 0x02fd ), k_eControllerType_XBoxOneController, uL("Xbox One S Controller") },	// Microsoft X-Box One S pad (Bluetooth)
	{ MAKE_CONTROLLER_ID( 0x045e, 0x02ff ), k_eControllerType_XBoxOneController, NULL },	// Microsoft X-Box One controller with XBOXGIP driver on Windows
	{ MAKE_CONTROLLER_ID( 0x045e, 0x0b00 ), k_eControllerType_XBoxOneController, uL("Xbox One Elite 2 Controller") },	// Microsoft X-Box One Elite Series 2 pad
	{ MAKE_CONTROLLER_ID( 0x045e, 0x0b02 ), k_eControllerType_XBoxOneController, uL("Xbox One Elite 2 Controller") },	// Microsoft X-Box One Elite Series 2 pad (Bluetooth)
	{ MAKE_CONTROLLER_ID( 0x045e, 0x0b05 ), k_eControllerType_XBoxOneController, uL("Xbox One Elite 2 Controller") },	// Microsoft X-Box One Elite Series 2 pad (Bluetooth)
	{ MAKE_CONTROLLER_ID( 0x045e, 0x0b0a ), k_eControllerType_XBoxOneController, uL("Xbox Adaptive Controller") },	// Microsoft X-Box Adaptive pad
	{ MAKE_CONTROLLER_ID( 0x045e, 0x0b0c ), k_eControllerType_XBoxOneController, uL("Xbox Adaptive Controller") },	// Microsoft X-Box Adaptive pad (Bluetooth)
	{ MAKE_CONTROLLER_ID( 0x045e, 0x0b12 ), k_eControllerType_XBoxOneController, uL("Xbox Series X Controller") },	// Microsoft X-Box Series X pad
	{ MAKE_CONTROLLER_ID( 0x045e, 0x0b13 ), k_eControllerType_XBoxOneController, uL("Xbox Series X Controller") },	// Microsoft X-Box Series X pad (BLE)
	{ MAKE_CONTROLLER_ID( 0x045e, 0x0b20 ), k_eControllerType_XBoxOneController, uL("Xbox One S Controller") },	// Microsoft X-Box One S pad (BLE)
	{ MAKE_CONTROLLER_ID( 0x045e, 0x0b21 ), k_eControllerType_XBoxOneController, uL("Xbox Adaptive Controller") },	// Microsoft X-Box Adaptive pad (BLE)
	{ MAKE_CONTROLLER_ID( 0x045e, 0x0b22 ), k_eControllerType_XBoxOneController, uL("Xbox One Elite 2 Controller") },	// Microsoft X-Box One Elite Series 2 pad (BLE)
	{ MAKE_CONTROLLER_ID( 0x0738, 0x4a01 ), k_eControllerType_XBoxOneController, NULL },	// Mad Catz FightStick TE 2
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0139 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Afterglow") },	// PDP Afterglow Wired Controller for Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x013B ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Face-Off Controller") },	// PDP Face-Off Gamepad for Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x013a ), k_eControllerType_XBoxOneController, NULL },	// PDP Xbox One Controller (unlisted)
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0145 ), k_eControllerType_XBoxOneController, uL("PDP MK X Fight Pad") },	// PDP MK X Fight Pad for Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0146 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Rock Candy") },	// PDP Rock Candy Wired Controller for Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x015b ), k_eControllerType_XBoxOneController, uL("PDP Fallout 4 Vault Boy Controller") },	// PDP Fallout 4 Vault Boy Wired Controller for Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x015c ), k_eControllerType_XBoxOneController, uL("PDP Xbox One @Play Controller") },	// PDP @Play Wired Controller for Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x015d ), k_eControllerType_XBoxOneController, uL("PDP Mirror's Edge Controller") },	// PDP Mirror's Edge Wired Controller for Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x015f ), k_eControllerType_XBoxOneController, uL("PDP Metallic Controller") },	// PDP Metallic Wired Controller for Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0160 ), k_eControllerType_XBoxOneController, uL("PDP NFL Face-Off Controller") },	// PDP NFL Official Face-Off Wired Controller for Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0161 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Camo") },	// PDP Camo Wired Controller for Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0162 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Controller") },	// PDP Wired Controller for Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0163 ), k_eControllerType_XBoxOneController, uL("PDP Deliverer of Truth") },	// PDP Legendary Collection: Deliverer of Truth
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0164 ), k_eControllerType_XBoxOneController, uL("PDP Battlefield 1 Controller") },	// PDP Battlefield 1 Official Wired Controller for Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0165 ), k_eControllerType_XBoxOneController, uL("PDP Titanfall 2 Controller") },	// PDP Titanfall 2 Official Wired Controller for Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0166 ), k_eControllerType_XBoxOneController, uL("PDP Mass Effect: Andromeda Controller") },	// PDP Mass Effect: Andromeda Official Wired Controller for Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0167 ), k_eControllerType_XBoxOneController, uL("PDP Halo Wars 2 Face-Off Controller") },	// PDP Halo Wars 2 Official Face-Off Wired Controller for Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0205 ), k_eControllerType_XBoxOneController, uL("PDP Victrix Pro Fight Stick") },	// PDP Victrix Pro Fight Stick
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0206 ), k_eControllerType_XBoxOneController, uL("PDP Mortal Kombat Controller") },	// PDP Mortal Kombat 25 Anniversary Edition Stick (Xbox One)
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0246 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Rock Candy") },	// PDP Rock Candy Wired Controller for Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0261 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Camo") },	// PDP Camo Wired Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0262 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Controller") },	// PDP Wired Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02a0 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Midnight Blue") },	// PDP Wired Controller for Xbox One - Midnight Blue
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02a1 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Verdant Green") },	// PDP Wired Controller for Xbox One - Verdant Green
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02a2 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Crimson Red") },	// PDP Wired Controller for Xbox One - Crimson Red
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02a3 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Arctic White") },	// PDP Wired Controller for Xbox One - Arctic White
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02a4 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Phantom Black") },	// PDP Wired Controller for Xbox One - Stealth Series | Phantom Black
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02a5 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Ghost White") },	// PDP Wired Controller for Xbox One - Stealth Series | Ghost White
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02a6 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Revenant Blue") },	// PDP Wired Controller for Xbox One - Stealth Series | Revenant Blue
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02a7 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Raven Black") },	// PDP Wired Controller for Xbox One - Raven Black
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02a8 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Arctic White") },	// PDP Wired Controller for Xbox One - Arctic White
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02a9 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Midnight Blue") },	// PDP Wired Controller for Xbox One - Midnight Blue
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02aa ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Verdant Green") },	// PDP Wired Controller for Xbox One - Verdant Green
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02ab ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Crimson Red") },	// PDP Wired Controller for Xbox One - Crimson Red
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02ac ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Ember Orange") },	// PDP Wired Controller for Xbox One - Ember Orange
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02ad ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Phantom Black") },	// PDP Wired Controller for Xbox One - Stealth Series | Phantom Black
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02ae ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Ghost White") },	// PDP Wired Controller for Xbox One - Stealth Series | Ghost White
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02af ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Revenant Blue") },	// PDP Wired Controller for Xbox One - Stealth Series | Revenant Blue
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02b0 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Raven Black") },	// PDP Wired Controller for Xbox One - Raven Black
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02b1 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Arctic White") },	// PDP Wired Controller for Xbox One - Arctic White
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02b3 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Afterglow") },	// PDP Afterglow Prismatic Wired Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02b5 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One GAMEware Controller") },	// PDP GAMEware Wired Controller Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02b6 ), k_eControllerType_XBoxOneController, NULL },	// PDP One-Handed Joystick Adaptive Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02bd ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Royal Purple") },	// PDP Wired Controller for Xbox One - Royal Purple
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02be ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Raven Black") },	// PDP Deluxe Wired Controller for Xbox One - Raven Black
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02bf ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Midnight Blue") },	// PDP Deluxe Wired Controller for Xbox One - Midnight Blue
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02c0 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Phantom Black") },	// PDP Deluxe Wired Controller for Xbox One - Stealth Series | Phantom Black
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02c1 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Ghost White") },	// PDP Deluxe Wired Controller for Xbox One - Stealth Series | Ghost White
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02c2 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Revenant Blue") },	// PDP Deluxe Wired Controller for Xbox One - Stealth Series | Revenant Blue
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02c3 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Verdant Green") },	// PDP Deluxe Wired Controller for Xbox One - Verdant Green
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02c4 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Ember Orange") },	// PDP Deluxe Wired Controller for Xbox One - Ember Orange
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02c5 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Royal Purple") },	// PDP Deluxe Wired Controller for Xbox One - Royal Purple
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02c6 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Crimson Red") },	// PDP Deluxe Wired Controller for Xbox One - Crimson Red
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02c7 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Arctic White") },	// PDP Deluxe Wired Controller for Xbox One - Arctic White
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02c8 ), k_eControllerType_XBoxOneController, uL("PDP Kingdom Hearts Controller") },	// PDP Kingdom Hearts Wired Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02c9 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Phantasm Red") },	// PDP Deluxe Wired Controller for Xbox One - Stealth Series | Phantasm Red
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02ca ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Specter Violet") },	// PDP Deluxe Wired Controller for Xbox One - Stealth Series | Specter Violet
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02cb ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Specter Violet") },	// PDP Wired Controller for Xbox One - Stealth Series | Specter Violet
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02cd ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Blu-merang") },	// PDP Rock Candy Wired Controller for Xbox One - Blu-merang
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02ce ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Cranblast") },	// PDP Rock Candy Wired Controller for Xbox One - Cranblast
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02cf ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Aqualime") },	// PDP Rock Candy Wired Controller for Xbox One - Aqualime
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02d5 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One Red Camo") },	// PDP Wired Controller for Xbox One - Red Camo
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0346 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One RC Gamepad") },	// PDP RC Gamepad for Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0446 ), k_eControllerType_XBoxOneController, uL("PDP Xbox One RC Gamepad") },	// PDP RC Gamepad for Xbox One
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02da ), k_eControllerType_XBoxOneController, uL("PDP Xbox Series X Afterglow") },	// PDP Xbox Series X Afterglow
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02d6 ), k_eControllerType_XBoxOneController, uL("Victrix Gambit Tournament Controller") },	// Victrix Gambit Tournament Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02d9 ), k_eControllerType_XBoxOneController, uL("PDP Xbox Series X Midnight Blue") },	// PDP Xbox Series X Midnight Blue
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x0063 ), k_eControllerType_XBoxOneController, NULL },	// Hori Real Arcade Pro Hayabusa (USA) Xbox One
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x0067 ), k_eControllerType_XBoxOneController, NULL },	// HORIPAD ONE
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x0078 ), k_eControllerType_XBoxOneController, NULL },	// Hori Real Arcade Pro V Kai Xbox One
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x00c5 ), k_eControllerType_XBoxOneController, NULL },	// HORI Fighting Commander
	{ MAKE_CONTROLLER_ID( 0x1532, 0x0a00 ), k_eControllerType_XBoxOneController, NULL },	// Razer Atrox Arcade Stick
	{ MAKE_CONTROLLER_ID( 0x1532, 0x0a03 ), k_eControllerType_XBoxOneController, NULL },	// Razer Wildcat
	{ MAKE_CONTROLLER_ID( 0x1532, 0x0a14 ), k_eControllerType_XBoxOneController, NULL },	// Razer Wolverine Ultimate
	{ MAKE_CONTROLLER_ID( 0x1532, 0x0a15 ), k_eControllerType_XBoxOneController, NULL },	// Razer Wolverine Tournament Edition
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x2001 ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X EnWired Controller - Black Inline
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x2002 ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X EnWired Controller Gray/White Inline
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x2003 ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X EnWired Controller Green Inline
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x2004 ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X EnWired Controller Pink inline
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x2005 ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X Wired Controller Core - Black
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x2006 ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X Wired Controller Core - White
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x2009 ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X EnWired Controller Red inline
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x200a ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X EnWired Controller Blue inline
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x200b ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X EnWired Controller Camo Metallic Red
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x200c ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X EnWired Controller Camo Metallic Blue
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x200d ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X EnWired Controller Seafoam Fade
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x200e ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X EnWired Controller Midnight Blue
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x200f ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X EnWired Soldier Green
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x2011 ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X EnWired - Metallic Ice
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x2012 ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X Cuphead EnWired Controller - Mugman
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x2015 ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X EnWired Controller - Blue Hint
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x2016 ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X EnWired Controller - Green Hint
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x2017 ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X EnWired Cntroller - Arctic Camo
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x2018 ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X EnWired Controller Arc Lightning
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x2019 ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X EnWired Controller Royal Purple
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x201a ), k_eControllerType_XBoxOneController, uL("PowerA Xbox Series X Controller") },       // PowerA Xbox Series X EnWired Controller Nebula
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x4001 ), k_eControllerType_XBoxOneController, uL("PowerA Fusion Pro 2 Controller") },	// PowerA Fusion Pro 2 Wired Controller (Xbox Series X style)
	{ MAKE_CONTROLLER_ID( 0x20d6, 0x4002 ), k_eControllerType_XBoxOneController, uL("PowerA Spectra Infinity Controller") },	// PowerA Spectra Infinity Wired Controller (Xbox Series X style)
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x541a ), k_eControllerType_XBoxOneController, NULL },	// PowerA Xbox One Mini Wired Controller
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x542a ), k_eControllerType_XBoxOneController, NULL },	// Xbox ONE spectra
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x543a ), k_eControllerType_XBoxOneController, uL("PowerA Xbox One Controller") },	// PowerA Xbox ONE liquid metal controller
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x551a ), k_eControllerType_XBoxOneController, NULL },	// PowerA FUSION Pro Controller
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x561a ), k_eControllerType_XBoxOneController, NULL },	// PowerA FUSION Controller
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x581a ), k_eControllerType_XBoxOneController, NULL },	// BDA XB1 Classic Controller
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x591a ), k_eControllerType_XBoxOneController, NULL },	// PowerA FUSION Pro Controller
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x592a ), k_eControllerType_XBoxOneController, NULL },	// BDA XB1 Spectra Pro
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x791a ), k_eControllerType_XBoxOneController, NULL },	// PowerA Fusion Fight Pad
	{ MAKE_CONTROLLER_ID( 0x2e24, 0x0652 ), k_eControllerType_XBoxOneController, NULL },	// Hyperkin Duke
	{ MAKE_CONTROLLER_ID( 0x2e24, 0x1618 ), k_eControllerType_XBoxOneController, NULL },	// Hyperkin Duke
	{ MAKE_CONTROLLER_ID( 0x2e24, 0x1688 ), k_eControllerType_XBoxOneController, NULL },	// Hyperkin X91
	{ MAKE_CONTROLLER_ID( 0x146b, 0x0611 ), k_eControllerType_XBoxOneController, NULL },	// Xbox Controller Mode for NACON Revolution 3

	// These have been added via Minidump for unrecognized Xinput controller assert
	{ MAKE_CONTROLLER_ID( 0x0000, 0x0000 ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x045e, 0x02a2 ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller - Microsoft VID
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x1414 ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0159 ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x24c6, 0xfaff ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x006d ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x00a4 ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0079, 0x1832 ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0079, 0x187f ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0079, 0x1883 ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x03eb, 0xff01 ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x2c22, 0x2303 ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0c12, 0x0ef8 ), k_eControllerType_XBox360Controller, NULL },	// Homemade fightstick based on brook pcb (with XInput driver??)
	{ MAKE_CONTROLLER_ID( 0x046d, 0x1000 ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x1345, 0x6006 ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller

	{ MAKE_CONTROLLER_ID( 0x056e, 0x2012 ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x146b, 0x0602 ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x00ae ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x046d, 0x0401 ), k_eControllerType_XBox360Controller, NULL },	// logitech xinput
	{ MAKE_CONTROLLER_ID( 0x046d, 0x0301 ), k_eControllerType_XBox360Controller, NULL },	// logitech xinput
	{ MAKE_CONTROLLER_ID( 0x046d, 0xcaa3 ), k_eControllerType_XBox360Controller, NULL },	// logitech xinput
	{ MAKE_CONTROLLER_ID( 0x046d, 0xc261 ), k_eControllerType_XBox360Controller, NULL },	// logitech xinput
	{ MAKE_CONTROLLER_ID( 0x046d, 0x0291 ), k_eControllerType_XBox360Controller, NULL },	// logitech xinput
	{ MAKE_CONTROLLER_ID( 0x0079, 0x18d3 ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x00b1 ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0001, 0x0001 ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0079, 0x188e ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0079, 0x187c ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0079, 0x189c ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0079, 0x1874 ), k_eControllerType_XBox360Controller, NULL },	// Unknown Controller

	{ MAKE_CONTROLLER_ID( 0x2f24, 0x0050 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x2f24, 0x002e ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x9886, 0x0024 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x2f24, 0x0091 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x1430, 0x0719 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x00ed ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x00c0 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x0152 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02a7 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x046d, 0x1007 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02b8 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02a8 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x2c22, 0x2503 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0079, 0x18a1 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller

	/* Added from Minidumps 10-9-19 */
	{ MAKE_CONTROLLER_ID( 0x0000, 0x6686 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x11ff, 0x0511 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x12ab, 0x0304 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x1430, 0x0291 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x1430, 0x02a9 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x1430, 0x070b ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x146b, 0x0604 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x146b, 0x0605 ), k_eControllerType_XBoxOneController, NULL },	// NACON PS4 controller in Xbox mode - might also be other bigben brand xbox controllers
	{ MAKE_CONTROLLER_ID( 0x146b, 0x0606 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x146b, 0x0609 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x1bad, 0x028e ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x1bad, 0x02a0 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x1bad, 0x5500 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x20ab, 0x55ef ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x24c6, 0x5509 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x2516, 0x0069 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x25b1, 0x0360 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x2c22, 0x2203 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x2f24, 0x0011 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x2f24, 0x0053 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x2f24, 0x00b7 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x046d, 0x0000 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x046d, 0x1004 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x046d, 0x1008 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x046d, 0xf301 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0738, 0x02a0 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0738, 0x7263 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0738, 0xb738 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0738, 0xcb29 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0738, 0xf401 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0079, 0x18c2 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0079, 0x18c8 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0079, 0x18cf ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0c12, 0x0e17 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0c12, 0x0e1c ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0c12, 0x0e22 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0c12, 0x0e30 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0xd2d2, 0xd2d2 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0d62, 0x9a1a ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0d62, 0x9a1b ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0e00, 0x0e00 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x012a ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02a1 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02a2 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02a5 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02b2 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02bd ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02bf ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02c0 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0x02c6 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x0097 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x00ba ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0f0d, 0x00d8 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0fff, 0x02a1 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x045e, 0x0867 ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	// Added 12-17-2020
	{ MAKE_CONTROLLER_ID( 0x16d0, 0x0f3f ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x2f24, 0x008f ), k_eControllerType_XBoxOneController, NULL },	// Unknown Controller
	{ MAKE_CONTROLLER_ID( 0x0e6f, 0xf501 ), k_eControllerType_XBoxOneController, NULL }	// Unknown Controller
};

#endif /* JSTICK_IDS_H_ */
