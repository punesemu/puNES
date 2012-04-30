/*
 * keyboard.c
 *
 *  Created on: 02/nov/2011
 *      Author: fhorse
 */

#include <string.h>
#include "gtk2.h"

DBWORD keyvalFromName(const gchar *keyval_name) {
	if (strcmp(keyval_name, "NULL") == 0) {
		return (0);
	}
	return (gdk_keyval_to_lower(gdk_keyval_from_name(keyval_name)));
}
gchar *keyvalToName(const DBWORD keyval) {
	if (!keyval) {
		return ("NULL");
	}
	return (gdk_keyval_name(gdk_keyval_to_lower(keyval)));
}
