/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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
#include "snd.h"
#include "clock.h"
#include "conf.h"
#include "fps.h"
#include "audio/blipbuf.h"
#include "audio/channels.h"
#include "gui.h"
#include "wave.h"

enum sndio_thread_loop_actions {
	AT_UNINITIALIZED,
	AT_RUN,
	AT_STOP,
	AT_PAUSE
};

typedef struct _sndio {
	struct sio_hdl *playback;
	struct pollfd *pfds;

	size_t bsize;
	size_t psize;
} _sndio;
typedef struct _thread {
	pthread_t thread;
	pthread_mutex_t lock;

	BYTE action;
	BYTE in_run;

	void *cache;
	_sndio *sndio;

#if !defined (RELEASE)
	double tick;
#endif
} _thread;

static void *sndio_playback_loop(void *data);
static void sndio_playback_loop_in_pause(void);
static void INLINE sndio_wr_buf(_sndio *sndio, void *buffer, uint32_t avail);

static _thread loop;
static _sndio sndio;
static _callback_data cbd;

BYTE snd_init(void) {
	memset(&snd, 0x00, sizeof(_snd));
	memset(&sndio, 0x00, sizeof(_sndio));
	memset(&cbd, 0x00, sizeof(_callback_data));
	memset(&loop, 0x00, sizeof(_thread));

	snd_apu_tick = NULL;
	snd_end_frame = NULL;

	// apro e avvio la riproduzione
	if (snd_playback_start()) {
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
void snd_quit(void) {
	// se e' in corso una registrazione, la concludo
	wave_close();

	if (loop.action != AT_UNINITIALIZED) {
		loop.action = AT_STOP;

		pthread_join(loop.thread, NULL);
		pthread_mutex_destroy(&loop.lock);
		memset(&loop, 0x00, sizeof(_thread));
	}

	snd_playback_stop();

#if !defined (RELEASE)
	fprintf(stderr, "\n");
#endif
}

BYTE snd_playback_start(void) {
	if (!cfg->apu.channel[APU_MASTER]) {
		return (EXIT_OK);
	}

	// se il thread del loop e' gia' in funzione lo metto in pausa
	if (loop.action == AT_RUN) {
		sndio_playback_loop_in_pause();
	}

	if (loop.action != AT_UNINITIALIZED) {
		snd_playback_lock(NULL);
	}

	// come prima cosa blocco eventuali riproduzioni
	snd_playback_stop();

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
		sndio.psize = (snd.samplerate / factor[cfg->audio_buffer_factor]);
		sndio.bsize = sndio.psize;

#if !defined (RELEASE)
		fprintf(stderr, "psize and bsize : %ld %ld\n", sndio.psize, sndio.bsize);
#endif

		par.bits = 16;
		par.sig = 1;
		par.pchan = snd.channels;
		par.rate = snd.samplerate;
		par.round = sndio.psize;
		par.appbufsz = sndio.bsize;
		par.xrun = SIO_IGNORE;

		if (!sio_setpar(sndio.playback, &par)) {
			fprintf(stderr, "sio_setpar() failed\n");
			goto snd_playback_start_error;
		}

		if (!sio_getpar(sndio.playback, &par)) {
			fprintf(stderr, "sio_getpar() failed\n");
			goto snd_playback_start_error;
		}

		sndio.psize = par.round;
		sndio.bsize = par.appbufsz;

#if !defined (RELEASE)
		fprintf(stderr, "new psize and bsize : %ld %ld\n", sndio.psize, sndio.bsize);
#endif
	}

	snd.samples = sndio.bsize * 4;
	snd.frequency = machine.cpu_hz / (double) snd.samplerate;

	{
		// dimensione in bytes del buffer software
		snd.buffer.size = (sndio.bsize * snd.channels * sizeof(*cbd.write)) * 10;

		snd.buffer.limit.low = (snd.buffer.size / 100) * 15;
		snd.buffer.limit.high = (snd.buffer.size / 100) * 45;

#if !defined (RELEASE)
		printf("softw bsize    : %10d - %10d\n", snd.buffer.size, snd.samples);
		printf("softw limit    : %10d - %10d\n", snd.buffer.limit.high, snd.buffer.limit.low);
#endif

		// alloco il buffer in memoria
		if (!(cbd.start = (SWORD *) malloc(snd.buffer.size))) {
			fprintf(stderr, "Unable to allocate audio buffers\n");
			goto snd_playback_start_error;
		}

		if (!(cbd.silence = (SWORD *) malloc(snd.buffer.size))) {
			fprintf(stderr, "Unable to allocate silence buffer\n");
			goto snd_playback_start_error;
		}

		// inizializzo il frame di scrittura
		cbd.write = cbd.start;
		// inizializzo il frame di lettura
		cbd.read = (SBYTE *) cbd.start;
		// punto alla fine del buffer
		cbd.end = cbd.read + snd.buffer.size;
		// azzero completamente i buffers
		memset(cbd.start, 0x00, snd.buffer.size);
		// azzero completamente il buffer del silenzio
		memset(cbd.silence, 0x00, snd.buffer.size);

		cbd.lock = (void *) &loop.lock;
	}

	if (extcl_snd_playback_start) {
		extcl_snd_playback_start((WORD) snd.samplerate);
	}

	audio_channels_init_mode();

	audio_init_blipbuf();

	if (loop.action == AT_UNINITIALIZED) {
		int rc;

		loop.sndio = &sndio;
		loop.action = AT_PAUSE;

		// creo il lock
		if (pthread_mutex_init(&loop.lock, NULL) != 0) {
			fprintf(stderr, "Unable to allocate the thread mutex\n");
			goto snd_playback_start_error;
		}

		snd_playback_lock(NULL);

		if ((rc = pthread_create(&loop.thread, NULL, sndio_playback_loop, (void *) &loop))) {
			fprintf(stderr, "Error - pthread_create() return code: %d\n", rc);
			goto snd_playback_start_error;;
		}
	}

	loop.cache = &cbd;

	if (!sio_start(sndio.playback)) {
		fprintf(stderr, "sio_start() failed\n");
		goto snd_playback_start_error;
	}

	snd_playback_unlock(NULL);
	gui_sleep(50);

	loop.action = AT_RUN;

	return (EXIT_OK);

	snd_playback_start_error:
	snd_playback_stop();
	return (EXIT_ERROR);
}
void snd_playback_stop(void) {
	sndio_playback_loop_in_pause();

	if (snd.cache) {
		if (SNDCACHE->start) {
			free(SNDCACHE->start);
		}

		if (SNDCACHE->silence) {
			free(SNDCACHE->silence);
		}

		snd.cache = NULL;
	}

	if (audio_channels_quit) {
		audio_channels_quit();
	}

	audio_quit_blipbuf();

	if (sndio.playback) {
		sio_close(sndio.playback);
		sndio.playback = NULL;
	}

	if (sndio.pfds) {
		free(sndio.pfds);
		sndio.pfds = NULL;
	}
}
void snd_playback_lock(_callback_data *cache) {
	pthread_mutex_lock(&loop.lock);
}
void snd_playback_unlock(_callback_data *cache) {
	pthread_mutex_unlock(&loop.lock);
}
uTCHAR *snd_playback_device_desc(int dev) {
	return (0);
}
uTCHAR *snd_playback_device_id(int dev) {
	return (0);
}

uTCHAR *snd_capture_device_desc(int dev) {
	return (0);
}
uTCHAR *snd_capture_device_id(int dev) {
	return (0);
}

void snd_list_devices(void) {}

static void *sndio_playback_loop(void *data) {
	_thread *th = (_thread *) data;
	uint32_t avail, len;

#if !defined (RELEASE)
	th->tick = gui_get_ms();
#endif

	while (TRUE) {
		_callback_data *cache = (_callback_data *) th->cache;

		if (th->action == AT_STOP) {
			th->in_run = FALSE;
			break;
		} else if (th->action == AT_PAUSE) {
			th->in_run = FALSE;
			gui_sleep(10);
			continue;
		}

		th->in_run = TRUE;

		{
			int nfds, event;

			if ((nfds = sio_pollfd(sndio.playback, sndio.pfds, POLLOUT)) <= 0) {
				fprintf(stderr, "sio_pollfd() failed\n");
				continue;
			}

			if (poll(sndio.pfds, nfds, -1) < 0) {
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

		snd_playback_lock(NULL);

		avail = sndio.psize;
		len = avail * snd.channels * sizeof(*cache->write);

		if ((info.no_rom | info.turn_off | info.pause) || (snd.buffer.start == FALSE) ||
			(fps.fast_forward == TRUE)) {
			sndio_wr_buf(th->sndio, (void *) cache->silence, len);
		} else if (cache->bytes_available < len) {
			sndio_wr_buf(th->sndio, (void *) cache->silence, len);
			snd.out_of_sync++;
		} else {
			wave_write((SWORD *) cache->read, avail);
			sndio_wr_buf(th->sndio, (void *) cache->read, len);

			cache->bytes_available -= len;
			cache->samples_available -= avail;

			if ((cache->read += len) >= cache->end) {
				cache->read = (SBYTE *) cache->start;
			}
		}

#if !defined (RELEASE)
		uint32_t request = avail;

		if ((gui_get_ms() - th->tick) >= 250.0f) {
			th->tick = gui_get_ms();
			if (info.snd_info == TRUE)
			fprintf(stderr, "snd : %6d %6d %d %6d %d %6d %6d %f %3d %f %4s\r",
				request,
				avail,
				len,
				fps.frames_skipped,
				cache->samples_available,
				cache->bytes_available,
				snd.out_of_sync,
				snd.frequency,
				(int) fps.gfx,
				machine.ms_frame,
				" ");
		}
#endif

		snd_playback_unlock(NULL);
 	}

	pthread_exit((void *)EXIT_OK);
}
static void sndio_playback_loop_in_pause(void) {
	if (loop.action != AT_UNINITIALIZED) {
		loop.action = AT_PAUSE;

		while (loop.in_run == TRUE) {
			gui_sleep(10);
		}
	}
}
static void INLINE sndio_wr_buf(_sndio *sndio, void *buffer, uint32_t avail) {
	if (sio_write(sndio->playback, buffer, avail) == 0) {
		if (sio_eof(sndio->playback)) {
			fprintf(stderr, "sio_write() failed\n");
		}
	}
}

