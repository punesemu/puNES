/*
 * keyboard.h
 *
 *  Created on: 02/nov/2011
 *      Author: fhorse
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include "common.h"

DBWORD keyval_from_name(const char *keyval_name);
char *keyval_to_name(const DBWORD keyval);

#endif /* KEYBOARD_H_ */
