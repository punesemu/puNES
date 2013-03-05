/*
 * keyboard.h
 *
 *  Created on: 02/nov/2011
 *      Author: fhorse
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

DBWORD keyval_from_name(const gchar *keyval_name);
gchar *keyval_to_name(const DBWORD keyval);

#endif /* KEYBOARD_H_ */
