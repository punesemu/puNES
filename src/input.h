/*
 * input.h
 *
 *  Created on: 01/nov/2011
 *      Author: fhorse
 */

#ifndef INPUT_H_
#define INPUT_H_

#include "common.h"

#define SET_PORT1(funct) input_port1 = funct
#define SET_PORT2(funct) input_port2 = funct
#define SET_RD_REG1(funct) input_rd_reg1 = funct
#define SET_RD_REG2(funct) input_rd_reg2 = funct

#define TURBO_BUTTON_DELAY_DEFAULT 3
#define TURBO_BUTTON_DELAY_MAX     20

enum controller_types { CTRL_DISABLED, CTRL_STANDARD, CTRL_ZAPPER };
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
	TRB_B
};
enum turbo_buttons { TURBOA, TURBOB };
enum input_types { KEYBOARD, JOYSTICK };
enum button_states { RELEASED = 0x40, PRESSED = 0x41 };

typedef struct {
	BYTE value;
} _r4016;
typedef struct {
	BYTE active;
	BYTE frequency;
	BYTE counter;
} _turbo_button;
typedef struct {
	BYTE type;
	BYTE joy_id;
	BYTE changed;
	/* standard controller */
	BYTE index;
	BYTE data[24];
	DBWORD input[2][24];
	/* turbo buttons */
	_turbo_button turbo[2];
	/* zapper */
	BYTE zapper;
} _port;

_r4016 r4016;
_port port1, port2;

void input_init(void);

BYTE input_rd_reg_disabled(BYTE openbus, WORD **screen_index, _port *port);

BYTE input_port_standard(BYTE mode, DBWORD event, BYTE type, _port *port);
BYTE input_rd_reg_standard(BYTE openbus, WORD **screen_index, _port *port);

BYTE input_rd_reg_zapper(BYTE openbus, WORD **screen_index, _port *port);

BYTE (*input_port1)(BYTE mode, DBWORD event, BYTE type, _port *port);
BYTE (*input_port2)(BYTE mode, DBWORD event, BYTE type, _port *port);
BYTE (*input_rd_reg1)(BYTE openbus, WORD **screen_index, _port *port);
BYTE (*input_rd_reg2)(BYTE openbus, WORD **screen_index, _port *port);

#endif /* INPUT_H_ */

#ifdef _INPUTINLINE_
static void INLINE input_turbo_buttons_control(_port *port);

static void INLINE input_turbo_buttons_control(_port *port) {
	if (port->turbo[TURBOA].active) {
		if (++port->turbo[TURBOA].counter == port->turbo[TURBOA].frequency) {
			port->data[BUT_A] = PRESSED;
		} else if (port->turbo[TURBOA].counter > port->turbo[TURBOA].frequency) {
			port->data[BUT_A] = RELEASED;
			port->turbo[TURBOA].counter = 0;
		}
	}
	if (port->turbo[TURBOB].active) {
		if (++port->turbo[TURBOB].counter == port->turbo[TURBOB].frequency) {
			port->data[BUT_B] = PRESSED;
		} else if (port->turbo[TURBOB].counter > port->turbo[TURBOB].frequency) {
			port->data[BUT_B] = RELEASED;
			port->turbo[TURBOB].counter = 0;
		}
	}
}
#endif
