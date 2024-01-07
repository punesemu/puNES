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

#include "audio/snd.h"
#include "clock.h"
#include "fps.h"

BYTE snd_handler(void) {
	if (snd.cache->bytes_available >= snd.buffer.size) {
		return (EXIT_ERROR);
	} else if (!fps_fast_forward_enabled()) {
		double landmark = snd.buffer.limit.low;
		double percent = ((((double)snd.cache->bytes_available) / landmark) * 100.0f) - 100.0f;
		double factor = 1.0f + ((((1.0f / (double)machine.fps) / 100.0f) * 1.0f) * percent);

		if (snd.factor != factor) {
			fps_machine_ms(factor);
			snd.factor = factor;
		}
	}
	return (EXIT_OK);
}
