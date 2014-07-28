/*
 * keyboard.c
 *
 *  Created on: 02/nov/2011
 *      Author: fhorse
 */

#include <string.h>
#include "gui.h"

DBWORD keyval_from_name(const gchar *keyval_name) {
	if (strcmp(keyval_name, "NULL") == 0) {
		return (0);
	}
	return (gdk_keyval_to_lower(gdk_keyval_from_name(keyval_name)));
}
const gchar *keyval_to_name(const DBWORD keyval) {
	if (!keyval) {
		return ("NULL");
	}
	return (gdk_keyval_name(gdk_keyval_to_lower(keyval)));
}
