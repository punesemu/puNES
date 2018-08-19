/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include "jstick.h"
#include "input.h"

void js_init(BYTE first_time) {
}
void js_quit(BYTE last_time) {
}
void js_update_detected_devices(void) {}
void js_control(_js *joy, _port *port) {
}

BYTE js_is_connected(int dev) {
	return (EXIT_ERROR);
}
BYTE js_is_this(BYTE dev, BYTE *id) {
	return (FALSE);
}
BYTE js_is_null(BYTE *id) {
	return (FALSE);
}
void js_set_id(BYTE *id, int dev) {
}
uTCHAR *js_name_device(int dev) {
	static uTCHAR name[128];

	umemset(name, 0x00, usizeof(name));
	ustrncpy(name, uL("Not connected"), usizeof(name));

	return((uTCHAR *) name);
}
BYTE js_read_event(_js_event *event, _js *joy) {
	return (EXIT_ERROR);
}
uTCHAR *js_to_name(const DBWORD val, const _js_element *list, const DBWORD length) {
	return ((uTCHAR *) list[0].name);
}
DBWORD js_from_name(const uTCHAR *name, const _js_element *list, const DBWORD length) {
	DBWORD js = 0;

	return (js);
}
DBWORD js_read_in_dialog(BYTE *id, int fd) {
	DBWORD value = 0;

	return (value);
}

void js_shcut_init(void) {
}
void js_shcut_stop(void) {
}
BYTE js_shcut_read(_js_sch *js_sch) {
	return (EXIT_ERROR);
}
