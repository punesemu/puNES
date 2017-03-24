/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

#include <QtCore/QStringList>
#include "info.h"
#include "conf.h"
#include "settings.h"
#include "audio_outin_dev.h"

static void outdev_reset_list(void);
static void indev_reset_list(void);

typedef struct _snd_dev_id_list {
	QStringList dev;
	QStringList id;
} _snd_dev_id_list;

_snd_dev_id_list outdev_list;
_snd_dev_id_list indev_list;

// ------------------------------ menu_Output_Device ------------------------------------
void outdev_init(void) {
	outdev_reset_list();
}
void outdev_add(uTCHAR *dev, uTCHAR *id) {
	QString sdev = uQString(dev);
	QString sid = uQString(id);

	outdev_list.dev.append(sdev);
	outdev_list.id.append(sid);
}
const char *outdev_dev_item(int index) {
	return ((const char *) outdev_list.dev.at(index).constData());
}
const char *outdev_id_item(int index) {
	return ((const char *) outdev_list.id.at(index).constData());
}
int outdev_dev_item_size(int index) {
	return (outdev_list.dev.at(index).length());
}
int outdev_id_item_size(int index) {
	return (outdev_list.id.at(index).length());
}

// ------------------------------- menu_Input_Device ------------------------------------
void indev_init(void) {
	indev_reset_list();
}
void indev_add(uTCHAR *dev, uTCHAR *id) {
	QString sdev = uQString(dev);
	QString sid = uQString(id);

	indev_list.dev.append(sdev);
	indev_list.id.append(sid);
}
const char *indev_item(int index) {
	return ((const char *) indev_list.dev.at(index).constData());
}
const char *indev_id_item(int index) {
	return ((const char *) indev_list.id.at(index).constData());
}
int indev_dev_item_size(int index) {
	return (indev_list.dev.at(index).length());
}
int indev_id_item_size(int index) {
	return (indev_list.id.at(index).length());
}

static void outdev_reset_list(void) {
	outdev_list.dev.clear();
	outdev_list.id.clear();
}
static void indev_reset_list(void) {
	indev_list.dev.clear();
	indev_list.id.clear();
}
