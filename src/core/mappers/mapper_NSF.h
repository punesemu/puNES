/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#ifndef MAPPER_NSF_H_
#define MAPPER_NSF_H_

#include "common.h"

enum { NSF_MAPPER = 0x1003 };

void map_init_NSF(void);
void extcl_snd_playback_start_NSF(WORD samplarate);
BYTE extcl_save_mapper_NSF(BYTE mode, BYTE slot, FILE *fp);
void extcl_length_clock_NSF(void);
void extcl_envelope_clock_NSF(void);
void extcl_apu_tick_NSF(void);

#endif /* MAPPER_NSF_H_ */
