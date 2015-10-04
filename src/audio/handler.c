/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

#include "snd.h"
#include "clock.h"
#include "fps.h"

static void INLINE snd_frequency(double factor);

BYTE snd_handler(void) {
	if (SNDCACHE->bytes_available >= snd.buffer.size) {
		return (EXIT_ERROR);
	} else if (fps.fast_forward == FALSE) {
		if (SNDCACHE->bytes_available < snd.buffer.limit.low) {
			snd_frequency(0.7f);
		} else if ((SNDCACHE->bytes_available > snd.buffer.limit.high)) {
			snd_frequency(2.0f);
		} else {
			snd_frequency(1.0f);
		}
	}

	return (EXIT_OK);
}

static void INLINE snd_frequency(double factor) {
	if (snd.factor != factor) {
		fps_machine_ms(factor)
		snd.factor = factor;
	}
}

