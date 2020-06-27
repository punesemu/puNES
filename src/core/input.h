/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#ifndef INPUT_H_
#define INPUT_H_

#if defined (_WIN32)
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#endif
#include "common.h"

#define TURBO_BUTTON_DELAY_DEFAULT 1
#define TURBO_BUTTON_DELAY_MAX     20

enum controller_modes { CTRL_MODE_NES, CTRL_MODE_FAMICOM, CTRL_MODE_FOUR_SCORE };
enum controller_types {
	CTRL_DISABLED,
	CTRL_STANDARD,
	CTRL_ZAPPER,
	CTRL_SNES_MOUSE,
	CTRL_ARKANOID_PADDLE,
	CTRL_OEKA_KIDS_TABLET
};
enum pad_types { CTRL_PAD_AUTO, CTRL_PAD_ORIGINAL, CTRL_PAD_3RD_PARTY };
enum controller_buttons {
	BUT_A,
	BUT_B,
	SELECT,
	START,
	UP,
	DOWN,
	LEFT,
	RIGHT,
	TRB_A,
	TRB_B,
	MAX_STD_PAD_BUTTONS
};
enum turbo_buttons { TURBOA, TURBOB };
enum input_types { KEYBOARD, JOYSTICK };
enum button_states { RELEASED = 0x00, PRESSED = 0x01 };
enum input_max_values { MAX_JOYSTICK = 16 };
enum input_set_cursor { NO_SET_CURSOR = FALSE, SET_CURSOR = TRUE};
enum port_controllers {
	PORT1,
	PORT2,
	PORT_BASE,
	PORT3 = PORT_BASE,
	PORT4,
	PORT_MAX
};

typedef struct _config_input {
	BYTE permit_updown_leftright;
	BYTE hide_zapper_cursor;
	BYTE controller_mode;
	BYTE expansion;
#if defined (_WIN32)
	GUID shcjoy_id;
#else
	BYTE shcjoy_id;
#endif
} _config_input;
typedef struct _r4016 {
	BYTE value;
} _r4016;
typedef struct _turbo_button {
	BYTE frequency;
	BYTE active;
	BYTE counter;
} _turbo_button;
typedef struct _port {
	BYTE type;
#if defined (_WIN32)
	GUID joy_id;
#else
	BYTE joy_id;
#endif

	// decodifica tastiera e joystick
	DBWORD input[2][24];

	// standard controller
	BYTE type_pad;
	BYTE index;
	BYTE data[24];
	// turbo buttons
	_turbo_button turbo[2];
} _port;
typedef struct _arkanoid {
	int x, rdx, button;
} _arkanoid;
typedef struct _array_pointers_port {
	_port *port[PORT_MAX];
} _array_pointers_port;
typedef struct _port_funct {
	void (*input_wr)(BYTE *value, BYTE nport);
	void (*input_rd)(BYTE *value, BYTE nport, BYTE shift);
	void (*input_add_event)(BYTE index);
	BYTE (*input_decode_event)(BYTE mode, BYTE autorepeat, DBWORD event, BYTE type, _port *port);
} _port_funct;

extern _r4016 r4016;
extern _port port[PORT_MAX];
extern _port_funct port_funct[PORT_MAX];
extern _arkanoid arkanoid[PORT_BASE];

extern BYTE (*input_wr_reg)(BYTE value);
extern BYTE (*input_rd_reg[2])(BYTE openbus, BYTE nport);

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void input_init(BYTE set_cursor);

EXTERNC void input_wr_disabled(BYTE *value, BYTE nport);
EXTERNC void input_rd_disabled(BYTE *value, BYTE nport, BYTE shift);

EXTERNC BYTE input_draw_target();

#undef EXTERNC

#endif /* INPUT_H_ */
