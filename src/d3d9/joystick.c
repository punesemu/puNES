/*
 * joystick.c
 *
 *  Created on: 03/nov/2011
 *      Author: fhorse
 */

#include "win.h"

void js_init(void) {
	return;
}
void js_open(_js *joy) {
	return;
}
void js_control(_js *joy, _port *port) {
	return;
}
void js_close(_js *joy) {
	return;
}
void js_quit(void) {
	return;
}
char *js_to_name(const DBWORD val, const _js_element *list, const DBWORD length) {
	static char str[20];

	return (str);
}
DBWORD js_from_name(const char *name, const _js_element *list, const DBWORD length) {
	return (0);
}
