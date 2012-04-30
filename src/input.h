/*
 * input.h
 *
 *  Created on: 01/nov/2011
 *      Author: fhorse
 */

#ifndef INPUT_H_
#define INPUT_H_

#include "common.h"

#define SETPORT1(funct) inputPort1 = funct
#define SETPORT2(funct) inputPort2 = funct
#define SETREADREG1(funct) inputReadReg1 = funct
#define SETREADREG2(funct) inputReadReg2 = funct

enum {
	CTRLDISABLED,
	STDCTRL,
	ZAPPER
};
enum {
	BUT_A,
	BUT_B,
	SELECT,
	START,
	UP,
	DOWN,
	LEFT,
	RIGHT,
	TRBA,
	TRBB
};
enum {
	TURBOA,
	TURBOB
};
enum {
	KEYBOARD,
	JOYSTICK,
};
enum {
	RELEASED = 0x40,
	PRESSED = 0x41
};

typedef struct {
	BYTE value;
} _r4016;
typedef struct {
	BYTE active;
	BYTE frequency;
	BYTE counter;
} _turboButton;
typedef struct {
	BYTE type;
	BYTE joyID;
	BYTE changed;
	/* standard controller */
	BYTE index;
	BYTE data[24];
	DBWORD input[2][24];
	/* turbo buttons */
	_turboButton turbo[2];
	/* zapper */
	BYTE zapper;
} _port;

_r4016 r4016;
_port port1, port2;

void inputInit(void);
void inputTurboButtonsFrequency(void);

BYTE inputReadRegDisabled(BYTE openbus, WORD **screenIndex, _port *port);

BYTE inputPortStandard(BYTE mode, DBWORD event, BYTE type, _port *port);
BYTE inputReadRegStandard(BYTE openbus, WORD **screenIndex, _port *port);

BYTE inputReadRegZapper(BYTE openbus, WORD **screenIndex, _port *port);

BYTE (*inputPort1)(BYTE mode, DBWORD event, BYTE type, _port *port);
BYTE (*inputPort2)(BYTE mode, DBWORD event, BYTE type, _port *port);
BYTE (*inputReadReg1)(BYTE openbus, WORD **screenIndex, _port *port);
BYTE (*inputReadReg2)(BYTE openbus, WORD **screenIndex, _port *port);

#endif /* INPUT_H_ */

#ifdef _INPUTINLINE_
static void INLINE inputTurboButtonsControl(_port *port);

static void INLINE inputTurboButtonsControl(_port *port) {
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
