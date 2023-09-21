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

#ifndef SST39SF040_H_
#define SST39SF040_H_

#include "common.h"

void sst39sf040_init(BYTE *data, size_t size, BYTE manufacter_id, BYTE model_id, WORD adr1, WORD adr2, int sector_size);
void sst39sf040_write(BYTE cidx, WORD address, BYTE value);
BYTE sst39sf040_read(BYTE cidx, WORD address);
void sst39sf040_tick(void);
BYTE sst39sf040_save_mapper(BYTE mode, BYTE slot, FILE *fp);

#endif /* SST39SF040_H_ */
