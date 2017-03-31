/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

#if defined (__WIN32__)
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#endif
#include "common.h"

#define SET_DECODE_EVENT(id, funct) input_decode_event[id] = funct
#define SET_ADD_EVENT(id, funct) input_add_event[id] = funct
#define SET_WR_REG(funct) input_wr_reg = funct
#define SET_RD_REG(id, funct) input_rd_reg[id] = funct

#define TURBO_BUTTON_DELAY_DEFAULT 1
#define TURBO_BUTTON_DELAY_MAX     20

enum controller_modes { CTRL_MODE_NES, CTRL_MODE_FAMICOM, CTRL_MODE_FOUR_SCORE };
enum controller_types { CTRL_DISABLED, CTRL_STANDARD, CTRL_ZAPPER };
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
	PORT3,
	PORT4,
	PORT_MAX
};

typedef struct _config_input {
	BYTE permit_updown_leftright;
	BYTE controller_mode;
#if defined (__WIN32__)
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
#if defined (__WIN32__)
	GUID joy_id;
#else
	BYTE joy_id;
#endif

	/* standard controller */
	BYTE type_pad;
	BYTE index;
	BYTE data[24];
	DBWORD input[2][24];
	/* turbo buttons */
	_turbo_button turbo[2];
	/* zapper */
	BYTE zapper;
} _port;
typedef struct _four_score {
	BYTE count;
} _four_score;
typedef struct _array_pointers_port {
	_port *port[PORT_MAX];
} _array_pointers_port;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC _r4016 r4016;
EXTERNC _port port[PORT_MAX];
EXTERNC _four_score four_score[PORT2 + 1];

EXTERNC void input_init(BYTE set_cursor);

EXTERNC BYTE input_rd_reg_disabled(BYTE openbus, WORD **screen_index, BYTE nport);

EXTERNC BYTE input_decode_event_standard(BYTE mode, DBWORD event, BYTE type, _port *port);
EXTERNC void input_add_event_standard(BYTE index);
EXTERNC BYTE input_wr_reg_standard(BYTE value);
EXTERNC BYTE input_rd_reg_standard(BYTE openbus, WORD **screen_index, BYTE nport);

EXTERNC BYTE input_rd_reg_famicon_expansion(BYTE openbus, WORD **screen_index, BYTE nport);

EXTERNC BYTE input_wr_reg_four_score(BYTE value);
EXTERNC BYTE input_rd_reg_four_score(BYTE openbus, WORD **screen_index, BYTE nport);

EXTERNC BYTE input_rd_reg_zapper(BYTE openbus, WORD **screen_index, BYTE nport);
EXTERNC BYTE input_zapper_is_connected(_port *array);

EXTERNC BYTE input_wr_reg_vs(BYTE value);
EXTERNC BYTE input_rd_reg_vs_disabled(BYTE openbus, WORD **screen_index, BYTE nport);
EXTERNC BYTE input_rd_reg_vs_standard(BYTE openbus, WORD **screen_index, BYTE nport);
EXTERNC BYTE input_rd_reg_vs_zapper(BYTE openbus, WORD **screen_index, BYTE nport);

EXTERNC BYTE (*input_decode_event[PORT_MAX])(BYTE mode, DBWORD event, BYTE type, _port *port);
EXTERNC void (*input_add_event[PORT_MAX])(BYTE index);
EXTERNC BYTE (*input_wr_reg)(BYTE value);
EXTERNC BYTE (*input_rd_reg[PORT2 + 1])(BYTE openbus, WORD **screen_index, BYTE nport);

#undef EXTERNC

#endif /* INPUT_H_ */
