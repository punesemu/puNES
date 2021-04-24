/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#include <signal.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sndio.h>
#include "info.h"
#include "audio/snd.h"
#include "clock.h"
#include "conf.h"
#include "fps.h"
#include "audio/blipbuf.h"
#include "audio/channels.h"
#include "gui.h"
#include "wave.h"
#include "rewind.h"

enum snd_thread_actions {
	ST_UNINITIALIZED,
	ST_RUN,
	ST_STOP,
	ST_PAUSE
};

typedef struct _sndio {
	struct sio_hdl *playback;
	struct pollfd *pfds;
} _sndio;
typedef struct _snd_thread {
	pthread_t thread;
	pthread_mutex_t lock;

	BYTE action;
	BYTE in_run;
	int pause_calls;

#if !defined (RELEASE)
	double tick;
#endif
} _snd_thread;

static void _snd_playback_stop(void);

static void *sndio_thread_loop(void *data);
INLINE static void sndio_wr_buf(void *buffer, uint32_t avail);

static _snd_thread snd_thread;
static _sndio sndio;
static _callback_data cbd;

_snd snd;
_snd_list snd_list;

void (*snd_apu_tick)(void);
void (*snd_end_frame)(void);

BYTE snd_init(void) {
	memset(&snd, 0x00, sizeof(_snd));
	memset(&sndio, 0x00, sizeof(_sndio));
	memset(&cbd, 0x00, sizeof(_callback_data));
	memset(&snd_thread, 0x00, sizeof(_snd_thread));

	snd_apu_tick = NULL;
	snd_end_frame = NULL;

	{
		int rc;

		// creo il lock
		if (pthread_mutex_init(&snd_thread.lock, NULL) != 0) {
			fprintf(stderr, "Unable to allocate the mutex\n");
			return (EXIT_ERROR);
		}

		snd_thread.action = ST_PAUSE;

		if ((rc = pthread_create(&snd_thread.thread, NULL, sndio_thread_loop, NULL))) {
			fprintf(stderr, "Error - pthread_create() return code: %d\n", rc);
			return (EXIT_ERROR);
		}
	}

	// apro e avvio la riproduzione
	if (snd_playback_start()) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
void snd_quit(void) {
	// se e' in corso una registrazione, la concludo
	wave_close();

	if (snd_thread.action != ST_UNINITIALIZED) {
		snd_thread.action = ST_STOP;

		pthread_join(snd_thread.thread, NULL);
		pthread_mutex_destroy(&snd_thread.lock);
		memset(&snd_thread, 0x00, sizeof(_snd_thread));
	}

	snd_playback_stop();

#if !defined (RELEASE)
	fprintf(stderr, "\n");
#endif
}

void snd_reset_buffers(void) {
	snd_thread_pause();

	if (snd.initialized == TRUE) {
		cbd.samples_available = 0;
		cbd.bytes_available = 0;
		cbd.write = cbd.start;
		cbd.read = (SBYTE *)cbd.start;
		memset(cbd.start, 0x00, snd.buffer.size);

		audio_channels_reset();
		audio_reset_blipbuf();

		snd.buffer.start = FALSE;
	}

	snd_thread_continue();
}

void snd_thread_pause(void) {
	if (snd_thread.action == ST_UNINITIALIZED) {
		return;
	}

	snd_thread.pause_calls++;

	if (snd_thread.pause_calls == 1) {
		snd_thread.action = ST_PAUSE;

		while (snd_thread.in_run == TRUE) {
			gui_sleep(1);
		}
	}
}
void snd_thread_continue(void) {
	if (snd_thread.action == ST_UNINITIALIZED) {
		return;
	}

	if (--snd_thread.pause_calls < 0) {
		snd_thread.pause_calls = 0;
	}

	if (snd_thread.pause_calls == 0) {
		snd_thread.action = ST_RUN;

		if (snd.initialized == TRUE) {
			while (snd_thread.in_run == FALSE) {
				gui_sleep(1);
			}
		}
	}
}

void snd_thread_lock(void) {
	pthread_mutex_lock(&snd_thread.lock);
}
void snd_thread_unlock(void) {
	pthread_mutex_unlock(&snd_thread.lock);
}

BYTE snd_playback_start(void) {
	snd_thread_pause();

	// come prima cosa blocco eventuali riproduzioni
	_snd_playback_stop();

	memset(&snd, 0x00, sizeof(_snd));
	memset(&sndio, 0x00, sizeof(_sndio));
	memset(&cbd, 0x00, sizeof(_callback_data));
	snd.cache = &cbd;

	audio_channels(cfg->channels_mode);

	switch (cfg->samplerate) {
		case S48000:
			snd.samplerate = 48000;
			break;
		case S44100:
			snd.samplerate = 44100;
			break;
		case S22050:
			snd.samplerate = 22050;
			break;
		case S11025:
			snd.samplerate = 11025;
			break;
	}

	if ((sndio.playback = sio_open(SIO_DEVANY, SIO_PLAY, TRUE)) == NULL) {
		fprintf(stderr, "sio_open() failed : audio disabled\n");
		cfg->apu.channel[APU_MASTER] = 0;
		return (EXIT_OK);
	}

	if ((sndio.pfds = calloc(sio_nfds(sndio.playback), sizeof(struct pollfd))) == NULL) {
		fprintf(stderr, "sio_nfds() failed\n");
		goto snd_playback_start_error;
	}

	{
		static int factor[10] = { 90, 80, 70, 60, 50, 40, 30, 20, 10, 5 };
		struct sio_par par;

		sio_initpar(&par);

		// snd.samplarate / 50 = 20 ms
		snd.period.samples = (snd.samplerate / factor[cfg->audio_buffer_factor]);

		par.bits = 16;
		par.pchan = snd.channels;
		par.rate = snd.samplerate;
		par.round = snd.period.samples;
		par.xrun = SIO_IGNORE;

		if (!sio_setpar(sndio.playback, &par)) {
			fprintf(stderr, "sio_setpar() failed\n");
			goto snd_playback_start_error;
		}

		if (!sio_getpar(sndio.playback, &par)) {
			fprintf(stderr, "sio_getpar() failed\n");
			goto snd_playback_start_error;
		}

		snd.period.samples = par.round;
		snd.frequency = machine.cpu_hz / (double)snd.samplerate;

		// dimensione in bytes del buffer software
		snd.period.size = snd.period.samples * snd.channels * sizeof(*cbd.write);
		snd.buffer.size = snd.period.size * ((snd.samplerate / snd.period.samples) + 1);

		snd.buffer.limit.low = snd.period.size * 2;
		snd.buffer.limit.high = snd.period.size * 7;

#if !defined (RELEASE)
		//fprintf(stderr, "new psize and bsize : %d %d\n", snd.period.samples, par.appbufsz);
		fprintf(stderr, "softw bsize    : %10d - %10d\n", snd.buffer.size, snd.period.samples);
		fprintf(stderr, "softw limit    : %10d - %10d\n", snd.buffer.limit.high, snd.buffer.limit.low);
#endif

		// alloco il buffer in memoria
		if (!(cbd.start = (SWORD *)malloc(snd.buffer.size))) {
			fprintf(stderr, "Unable to allocate audio buffers\n");
			goto snd_playback_start_error;
		}

		if (!(cbd.silence = (SWORD *)malloc(snd.period.size))) {
			fprintf(stderr, "Unable to allocate silence buffer\n");
			goto snd_playback_start_error;
		}

		// inizializzo il frame di scrittura
		cbd.write = cbd.start;
		// inizializzo il frame di lettura
		cbd.read = (SBYTE *)cbd.start;
		// punto alla fine del buffer
		cbd.end = cbd.read + snd.buffer.size;
		// azzero completamente i buffers
		memset(cbd.start, 0x00, snd.buffer.size);
		// azzero completamente il buffer del silenzio
		memset(cbd.silence, 0x00, snd.period.size);
	}

	audio_channels_init_mode();

	audio_init_blipbuf();

	if (!sio_start(sndio.playback)) {
		fprintf(stderr, "sio_start() failed\n");
		goto snd_playback_start_error;
	}

	snd.initialized = TRUE;

	snd_thread_continue();
	gui_sleep(150);
	return (EXIT_OK);

	snd_playback_start_error:
	_snd_playback_stop();
	snd_thread_continue();
	gui_sleep(150);
	return (EXIT_ERROR);
}
void snd_playback_stop(void) {
	snd_thread_pause();
	_snd_playback_stop();
	snd_thread_continue();
}

void snd_playback_pause(void) {
	snd.pause_calls++;
}
void snd_playback_continue(void) {
	if (--snd.pause_calls < 0) {
		snd.pause_calls = 0;
	}
}

uTCHAR *snd_playback_device_desc(UNUSED(int dev)) {
	return (0);
}
uTCHAR *snd_playback_device_id(UNUSED(int dev)) {
	return (0);
}

uTCHAR *snd_capture_device_desc(UNUSED(int dev)) {
	return (0);
}
uTCHAR *snd_capture_device_id(UNUSED(int dev)) {
	return (0);
}

void snd_list_devices(void) {}

static void _snd_playback_stop(void) {
	snd.initialized = FALSE;

	if (cbd.start) {
		free(cbd.start);
		cbd.start = NULL;
	}

	if (cbd.silence) {
		free(cbd.silence);
		cbd.silence = NULL;
	}

	snd.cache = NULL;

	if (audio_channels_quit) {
		audio_channels_quit();
	}

	audio_quit_blipbuf();

	if (sndio.playback) {
		sio_close(sndio.playback);
		sndio.playback = NULL;
		gui_sleep(150);
	}

	if (sndio.pfds) {
		free(sndio.pfds);
		sndio.pfds = NULL;
	}
}

static void *sndio_thread_loop(UNUSED(void *data)) {
	int32_t avail, len;

#if !defined (RELEASE)
	snd_thread.tick = gui_get_ms();
#endif

	while (TRUE) {
		if (snd_thread.action == ST_STOP) {
			snd_thread.in_run = FALSE;
			break;
		} else if ((snd_thread.action == ST_PAUSE) || (snd.initialized == FALSE)) {
			snd_thread.in_run = FALSE;
			gui_sleep(1);
			continue;
		}

		snd_thread.in_run = TRUE;

		{
			int nfds, event, rc;

			nfds = sio_pollfd(sndio.playback, sndio.pfds, POLLOUT);
			rc = poll(sndio.pfds, nfds, 1);

			if (rc == 0) {
				continue;
			} else if (rc < 0) {
				if (errno == EINTR) {
					continue;
				}
				perror("poll");
				continue;
			}

			event = sio_revents(sndio.playback, sndio.pfds);

			if (event & POLLHUP) {
				continue;
			}

			if ((event & POLLOUT) == 0) {
				continue;
			}
		}

		avail = snd.period.samples;
		len = avail * snd.channels * sizeof(*cbd.write);

		if (info.no_rom | info.turn_off | info.pause | rwnd.active | fps.fast_forward | !snd.buffer.start) {
			sndio_wr_buf((void *)cbd.silence, len);
		} else if (cbd.bytes_available < len) {
			sndio_wr_buf((void *)cbd.silence, len);
			snd.out_of_sync++;
		} else {
			void *read = (void *)cbd.read;

			snd_thread_lock();

			if (snd.pause_calls) {
				read = (void *)cbd.silence;
			}

			wave_write((SWORD *)read, avail);
			sndio_wr_buf(read, len);

			cbd.bytes_available -= len;
			cbd.samples_available -= avail;

#if !defined (RELEASE)
			if (((void*)cbd.write > (void*)cbd.read) && ((void*)cbd.write < (void*)(cbd.read + len))) {
				snd.overlap++;
			}
#endif

			if ((cbd.read += len) >= cbd.end) {
				cbd.read = (SBYTE *)cbd.start;
			}

			snd_thread_unlock();
		}

#if !defined (RELEASE)
		if ((gui_get_ms() - snd_thread.tick) >= 250.0f) {
			snd_thread.tick = gui_get_ms();
			if (info.snd_info == TRUE)
			fprintf(stderr, "snd : %d %d %6d %6d %4d %4d %4d %4d %3d %f %4s\r",
				avail,
				len,
				cbd.samples_available,
				cbd.bytes_available,
				fps.frames_emu_too_long,
				fps.frames_skipped,
				snd.overlap,
				snd.out_of_sync,
				(int)fps.gfx,
				machine.ms_frame,
				" ");
		}
#endif
 	}

	pthread_exit((void *)EXIT_OK);
}

INLINE static void sndio_wr_buf(void *buffer, uint32_t avail) {
	if (sio_write(sndio.playback, buffer, avail) == 0) {
		if (sio_eof(sndio.playback)) {
			fprintf(stderr, "sio_write() failed\n");
		}
	}
}
