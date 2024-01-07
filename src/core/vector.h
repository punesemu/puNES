/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#ifndef VECTOR_H_
#define VECTOR_H_

#include <stdio.h>
#include <stdlib.h>
#include "common.h"

enum _vector_misc {
	VECTOR_INIT_CAPACITY = 6,
};

typedef struct _vector {
	size_t item_size;
	size_t capacity;
	size_t total;
	void *items;
} _vector;

BYTE vector_init(_vector *v, size_t item_size);
size_t vector_total(_vector *v);
BYTE vector_resize(_vector *v, size_t capacity);
BYTE vector_push_back(_vector *v, void *item);
BYTE vector_set(_vector *v, size_t index, void *item);
void *vector_get(_vector *v, size_t index);
BYTE vector_delete(_vector *v, size_t index);
BYTE vector_clear(_vector *v);
BYTE vector_free(_vector *v);

#endif /* VECTOR_H_ */
