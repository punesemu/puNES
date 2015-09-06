/*
 * handler.c
 *
 *  Created on: 06 set 2015
 *      Author: fhorse
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
			snd_frequency(0.3f);
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

