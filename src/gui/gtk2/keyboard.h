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

DBWORD keyvalFromName(const gchar *keyval_name);
gchar *keyvalToName(const DBWORD keyval);

#endif /* KEYBOARD_H_ */
