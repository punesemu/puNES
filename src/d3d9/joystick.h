/*
 * joystick.h
 *
 *  Created on: 03/nov/2011
 *      Author: fhorse
 */

#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include "common.h"
#include "input.h"

#define name_to_jsv(name) js_from_name(name, jsv_list, LENGTH(jsv_list))
#define name_to_jsn(name) js_from_name(name, jsn_list, LENGTH(jsn_list))
#define jsv_to_name(jsvl) js_to_name(jsvl, jsv_list, LENGTH(jsv_list))
#define jsn_to_name(jsvl) js_to_name(jsvl, jsn_list, LENGTH(jsn_list))

typedef struct {
	BYTE p1;
} _js;
typedef struct {
	BYTE p2;
} _js_element;

static const _js_element jsn_list[] = {
};
static const _js_element jsv_list[] = {
};

void js_init(void);
void js_open(_js *joy);
void js_control(_js *joy, _port *port);
void js_close(_js *joy);
void js_quit(void);
char *js_to_name(const DBWORD val, const _js_element *list, const DBWORD length);
DBWORD js_from_name(const char *name, const _js_element *list, const DBWORD lenght);

#endif /* JOYSTICK_H_ */
