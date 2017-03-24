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

#ifndef AUDIO_OUTIN_DEV_H_
#define AUDIO_OUTIN_DEV_H_

#include "common.h"

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void outdev_init(void);
EXTERNC void outdev_add(uTCHAR *dev, uTCHAR *id);
EXTERNC void outdev_add_default(void);
EXTERNC const char *outdev_dev_item(int index);
EXTERNC const char *outdev_id_item(int index);
EXTERNC int outdev_dev_item_size(int index);
EXTERNC int outdev_id_item_size(int index);

EXTERNC void indev_init(void);
EXTERNC void indev_add(uTCHAR *dev, uTCHAR *id);
EXTERNC void indev_add_default(void);
EXTERNC const char *indev_dev_item(int index);
EXTERNC const char *indev_id_item(int index);
EXTERNC int indev_dev_item_size(int index);
EXTERNC int indev_id_item_size(int index);

#undef EXTERNC

#endif /* AUDIO_OUTIN_DEV_H_ */
