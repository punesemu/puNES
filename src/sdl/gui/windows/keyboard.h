/*
 * keyboard.h
 *
 *  Created on: 02/nov/2011
 *      Author: fhorse
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include "common.h"

DBWORD keyvalFromName(const char *keyval_name);
char *keyvalToName(const DBWORD keyval);

#endif /* KEYBOARD_H_ */
