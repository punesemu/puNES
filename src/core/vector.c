/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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
#include "vector.h"

BYTE vector_init(_vector *v, size_t item_size) {
	BYTE status = EXIT_ERROR;

	v->item_size = item_size;
	v->capacity = VECTOR_INIT_CAPACITY;
	v->total = 0;
	v->items = malloc(v->item_size * v->capacity);
	if (v->items) {
		status = EXIT_OK;
	}
	return (status);
}
size_t vector_total(_vector *v) {
	size_t total = -1;

	if (v) {
		total = v->total;
	}
	return (total);
}
BYTE vector_resize(_vector *v, size_t capacity) {
	BYTE status = EXIT_ERROR;

	if (v) {
		void *items = realloc(v->items, v->item_size * capacity);

		if (items) {
			v->items = items;
			v->capacity = capacity;
			status = EXIT_OK;
		}
	}
	return (status);
}
BYTE vector_push_back(_vector *v, void *item) {
	BYTE status = EXIT_ERROR;

	if (v) {
		if (v->capacity == v->total) {
			status = vector_resize(v, v->capacity * 2);
			if (status != EXIT_ERROR) {
				memcpy(v->items + (v->total * v->item_size), item, v->item_size);
				v->total++;
			}
		} else {
			memcpy(v->items + (v->total * v->item_size), item, v->item_size);
			v->total++;
			status = EXIT_OK;
		}
	}
	return (status);
}
BYTE vector_set(_vector *v, size_t index, void *item) {
	BYTE status = EXIT_ERROR;

	if (v) {
		if (index < v->total) {
			memcpy(v->items + (index * v->item_size), item, v->item_size);
			status = EXIT_OK;
		}
	}
	return (status);
}
void *vector_get(_vector *v, size_t index) {
	void *item = NULL;

	if (v) {
		if (index < v->total) {
			item = v->items + (index * v->item_size);
		}
	}
	return (item);
}
BYTE vector_delete(_vector *v, size_t index) {
	BYTE status = EXIT_ERROR;
	size_t i;

	if (v) {
		if (index >= v->total) {
			return (status);
		}
		for (i = index; i < (v->total - 1); i++) {
			memcpy(v->items + (i * v->item_size), v->items + ((i + 1) * v->item_size), v->item_size);
		}
		v->total--;
		memset(v->items + (v->total * v->item_size), 0x00, v->item_size);
		if ((v->total > 0) && (v->total == (v->capacity / 4))) {
			vector_resize(v, v->capacity / 2);
		}
		status = EXIT_OK;
	}
	return (status);
}
BYTE vector_clear(_vector *v) {
	BYTE status = EXIT_ERROR;

	if (v) {
		vector_free(v);
		status = vector_init(v, v->item_size);
	}
	return (status);
}
BYTE vector_free(_vector *v) {
	BYTE status = EXIT_ERROR;

	if (v) {
		if (v->items) {
			free(v->items);
			v->items = NULL;
		}
		v->capacity = 0;
		v->total = 0;
		status = EXIT_OK;
	}
	return (status);
}
